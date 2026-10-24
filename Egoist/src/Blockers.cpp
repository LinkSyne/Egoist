#include "Blockers.h"
#include <windows.h>
#include <cwctype>
#include <initializer_list>
#include <string>

namespace {

bool SetDword(HKEY root, const wchar_t* key, const wchar_t* name, DWORD value) {
    HKEY h;
    if (RegCreateKeyExW(root, key, 0, nullptr, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, nullptr, &h, nullptr) != ERROR_SUCCESS)
        return false;
    LONG r = RegSetValueExW(h, name, 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
    RegCloseKey(h);
    return r == ERROR_SUCCESS;
}

bool GetDword(HKEY root, const wchar_t* key, const wchar_t* name, DWORD& out) {
    HKEY h;
    if (RegOpenKeyExW(root, key, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &h) != ERROR_SUCCESS) return false;
    DWORD size = sizeof(out), type = 0;
    LONG r = RegQueryValueExW(h, name, nullptr, &type, (BYTE*)&out, &size);
    RegCloseKey(h);
    return r == ERROR_SUCCESS && type == REG_DWORD;
}

bool DelValue(HKEY root, const wchar_t* key, const wchar_t* name) {
    HKEY h;
    LONG o = RegOpenKeyExW(root, key, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, &h);
    if (o == ERROR_FILE_NOT_FOUND) return true;
    if (o != ERROR_SUCCESS) return false;
    LONG r = RegDeleteValueW(h, name);
    RegCloseKey(h);
    return r == ERROR_SUCCESS || r == ERROR_FILE_NOT_FOUND;
}

void DelKeyIfEmpty(HKEY root, const wchar_t* key) {
    HKEY h;
    if (RegOpenKeyExW(root, key, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &h) != ERROR_SUCCESS) return;
    DWORD subs = 0, vals = 0;
    LONG r = RegQueryInfoKeyW(h, nullptr, nullptr, nullptr, &subs, nullptr, nullptr, &vals,
                              nullptr, nullptr, nullptr, nullptr);
    RegCloseKey(h);
    if (r == ERROR_SUCCESS && subs == 0 && vals == 0)
        RegDeleteKeyExW(root, key, KEY_WOW64_64KEY, 0);
}

const wchar_t* kMarkerKey = L"Software\\Egoist";

bool MarkerGet(HKEY root, const wchar_t* name) {
    DWORD v = 0;
    return GetDword(root, kMarkerKey, name, v) && v == 1;
}

bool MarkerSet(HKEY root, const wchar_t* name, bool on) {
    return on ? SetDword(root, kMarkerKey, name, 1) : DelValue(root, kMarkerKey, name);
}

struct RegVal { const wchar_t* name; DWORD data; };
struct Policy { const wchar_t* browser; const wchar_t* key; std::vector<RegVal> values; };

Policy Chromium(const wchar_t* name, const wchar_t* key) {
    return { name, key, { { L"DefaultNotificationsSetting", 2 } } };
}

Policy Gecko(const wchar_t* name, const wchar_t* key) {
    return { name, key, { { L"BlockNewRequests", 1 }, { L"Locked", 1 } } };
}

const std::vector<Policy>& Policies() {
    static const std::vector<Policy> p = {
        Chromium(L"Google Chrome",         L"SOFTWARE\\Policies\\Google\\Chrome"),
        Chromium(L"Microsoft Edge",        L"SOFTWARE\\Policies\\Microsoft\\Edge"),
        Chromium(L"Brave",                 L"SOFTWARE\\Policies\\BraveSoftware\\Brave"),
        Chromium(L"Vivaldi",               L"SOFTWARE\\Policies\\Vivaldi"),
        Chromium(L"Chromium",              L"SOFTWARE\\Policies\\Chromium"),
        Chromium(L"Naver Whale",           L"SOFTWARE\\Policies\\Naver\\Whale"),
        Chromium(L"Avast Secure Browser",  L"SOFTWARE\\Policies\\AVAST Software\\Browser"),
        Chromium(L"AVG Secure Browser",    L"SOFTWARE\\Policies\\AVG\\Browser"),
        Gecko(L"Mozilla Firefox",          L"SOFTWARE\\Policies\\Mozilla\\Firefox\\Permissions\\Notification"),
    };
    return p;
}

void RemoveLegacy() {
    const wchar_t* key = L"SOFTWARE\\Policies\\YandexBrowser";
    DelValue(HKEY_LOCAL_MACHINE, key, L"DefaultNotificationsSetting");
    DelKeyIfEmpty(HKEY_LOCAL_MACHINE, key);
}

bool BrowserActive() { return MarkerGet(HKEY_LOCAL_MACHINE, L"BrowserBlock"); }

bool BrowserSet(bool on) {
    RemoveLegacy();
    bool ok = true;
    for (const Policy& p : Policies())
        for (const RegVal& v : p.values) {
            bool r = on ? SetDword(HKEY_LOCAL_MACHINE, p.key, v.name, v.data)
                        : DelValue(HKEY_LOCAL_MACHINE, p.key, v.name);
            ok = r && ok;
        }
    if (!on)
        for (const Policy& p : Policies()) DelKeyIfEmpty(HKEY_LOCAL_MACHINE, p.key);
    return MarkerSet(HKEY_LOCAL_MACHINE, L"BrowserBlock", on) && ok;
}

const wchar_t* kPushKey   = L"Software\\Microsoft\\Windows\\CurrentVersion\\PushNotifications";
const wchar_t* kPushPolicy = L"Software\\Policies\\Microsoft\\Windows\\CurrentVersion\\PushNotifications";

bool WindowsActive() { return MarkerGet(HKEY_CURRENT_USER, L"WindowsToasts"); }

bool WindowsSet(bool on) {
    bool ok;
    if (on) {
        ok = SetDword(HKEY_CURRENT_USER, kPushKey, L"ToastEnabled", 0);
        ok = SetDword(HKEY_CURRENT_USER, kPushPolicy, L"NoToastApplicationNotification", 1) && ok;
    } else {
        ok = SetDword(HKEY_CURRENT_USER, kPushKey, L"ToastEnabled", 1);
        ok = DelValue(HKEY_CURRENT_USER, kPushPolicy, L"NoToastApplicationNotification") && ok;
        DelKeyIfEmpty(HKEY_CURRENT_USER, kPushPolicy);
    }
    return MarkerSet(HKEY_CURRENT_USER, L"WindowsToasts", on) && ok;
}

const wchar_t* kNotifKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Notifications\\Settings";

std::wstring Lower(std::wstring s) {
    for (wchar_t& c : s) c = (wchar_t)towlower(c);
    return s;
}

std::vector<std::wstring> ListNotifApps() {
    std::vector<std::wstring> r;
    HKEY h;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kNotifKey, 0, KEY_ENUMERATE_SUB_KEYS | KEY_WOW64_64KEY, &h) != ERROR_SUCCESS)
        return r;
    for (DWORD i = 0;; ++i) {
        wchar_t name[512];
        DWORD len = 512;
        LONG e = RegEnumKeyExW(h, i, name, &len, nullptr, nullptr, nullptr, nullptr);
        if (e == ERROR_NO_MORE_ITEMS) break;
        if (e == ERROR_SUCCESS) r.push_back(name);
    }
    RegCloseKey(h);
    return r;
}

bool SetAppNotifications(bool on, std::initializer_list<const wchar_t*> known,
                         std::initializer_list<const wchar_t*> parts) {
    std::vector<std::wstring> targets, lowered;
    auto add = [&](const std::wstring& n) {
        std::wstring l = Lower(n);
        for (const std::wstring& x : lowered) if (x == l) return;
        lowered.push_back(l);
        targets.push_back(n);
    };
    for (const wchar_t* k : known) add(k);
    for (const std::wstring& app : ListNotifApps()) {
        std::wstring l = Lower(app);
        for (const wchar_t* p : parts)
            if (l.find(p) != std::wstring::npos) { add(app); break; }
    }
    bool ok = true;
    for (const std::wstring& t : targets) {
        std::wstring key = std::wstring(kNotifKey) + L"\\" + t;
        if (on) {
            ok = SetDword(HKEY_CURRENT_USER, key.c_str(), L"Enabled", 0) && ok;
        } else {
            ok = DelValue(HKEY_CURRENT_USER, key.c_str(), L"Enabled") && ok;
            DelKeyIfEmpty(HKEY_CURRENT_USER, key.c_str());
        }
    }
    return ok;
}

bool GamesActive() { return MarkerGet(HKEY_CURRENT_USER, L"GamesToasts"); }
bool GamesSet(bool on) {
    bool ok = SetAppNotifications(on,
        { L"Microsoft.XboxGamingOverlay_8wekyb3d8bbwe!App", L"Microsoft.GamingApp_8wekyb3d8bbwe!Microsoft.Xbox.App" },
        { L"xbox", L"gaming", L"epic", L"battle.net", L"blizzard", L"ubisoft", L"uplay", L"riot", L"galaxyclient" });
    return MarkerSet(HKEY_CURRENT_USER, L"GamesToasts", on) && ok;
}

bool SteamActive() { return MarkerGet(HKEY_CURRENT_USER, L"SteamToasts"); }
bool SteamSet(bool on) {
    bool ok = SetAppNotifications(on, { L"Steam" }, { L"steam" });
    return MarkerSet(HKEY_CURRENT_USER, L"SteamToasts", on) && ok;
}

bool TelegramActive() { return MarkerGet(HKEY_CURRENT_USER, L"TelegramToasts"); }
bool TelegramSet(bool on) {
    bool ok = SetAppNotifications(on, { L"Telegram.TelegramDesktop" }, { L"telegram" });
    return MarkerSet(HKEY_CURRENT_USER, L"TelegramToasts", on) && ok;
}

}

const std::vector<Blocker>& Blockers() {
    static const std::vector<Blocker> list = {
        { Str::BlockBrowser,  Str::NoteBrowser, BrowserActive,  BrowserSet },
        { Str::BlockWindows,  Str::NoteWindows, WindowsActive,  WindowsSet },
        { Str::BlockGames,    Str::NoteApps,    GamesActive,    GamesSet },
        { Str::BlockSteam,    Str::NoteApps,    SteamActive,    SteamSet },
        { Str::BlockTelegram, Str::NoteApps,    TelegramActive, TelegramSet },
    };
    return list;
}

void ReapplyActive() {
    for (const Blocker& b : Blockers())
        if (b.isActive()) b.set(true);
}
