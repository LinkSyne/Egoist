#include "Config.h"
#include <windows.h>

static Config g_cfg;
static const wchar_t* kKey = L"Software\\Egoist";

Config& Cfg() { return g_cfg; }

static bool ReadDword(const wchar_t* name, DWORD& out) {
    DWORD size = sizeof(DWORD);
    return RegGetValueW(HKEY_CURRENT_USER, kKey, name, RRF_RT_REG_DWORD, nullptr, &out, &size) == ERROR_SUCCESS;
}

void LoadConfig() {
    DWORD v;
    if (ReadDword(L"Language", v) && v <= 1) g_cfg.lang = (Lang)v;
    else g_cfg.lang = (PRIMARYLANGID(GetUserDefaultUILanguage()) == LANG_RUSSIAN) ? Lang::RU : Lang::EN;
    if (ReadDword(L"Theme", v) && v <= 1) g_cfg.theme = (Theme)v;
}

void SaveConfig() {
    DWORD l = (DWORD)g_cfg.lang, t = (DWORD)g_cfg.theme;
    RegSetKeyValueW(HKEY_CURRENT_USER, kKey, L"Language", REG_DWORD, &l, sizeof(l));
    RegSetKeyValueW(HKEY_CURRENT_USER, kKey, L"Theme",    REG_DWORD, &t, sizeof(t));
}
