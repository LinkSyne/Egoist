#include "app.h"
#include "Config.h"
#include "Lang.h"
#include "Ui.h"

namespace {

enum { ID_RU = 401, ID_EN, ID_LIGHT, ID_DARK };

HWND g_set, hLangL, hRU, hEN, hThemeL, hLight, hDark;

void SetTexts() {
    SetWindowTextW(g_set, T(Str::SettingsTitle));
    SetWindowTextW(hLangL, T(Str::LangLabel));
    SetWindowTextW(hThemeL, T(Str::ThemeLabel));
    SetWindowTextW(hLight, T(Str::Light));
    SetWindowTextW(hDark, T(Str::Dark));
}

LRESULT CALLBACK SettingsProc(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    using Ui::S;
    switch (msg) {
    case WM_CREATE:
        g_set = h;
        hLangL = Ui::Ctl(h, L"STATIC", L"", SS_LEFT, S(20), S(16), S(280), S(22));
        SendMessageW(hLangL, WM_SETFONT, (WPARAM)Ui::FontMid(), TRUE);
        hRU = Ui::Ctl(h, L"BUTTON", L"RU", BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP, S(30), S(48), S(120), S(24), ID_RU);
        hEN = Ui::Ctl(h, L"BUTTON", L"EN", BS_AUTORADIOBUTTON, S(160), S(48), S(120), S(24), ID_EN);

        hThemeL = Ui::Ctl(h, L"STATIC", L"", SS_LEFT, S(20), S(92), S(280), S(22));
        SendMessageW(hThemeL, WM_SETFONT, (WPARAM)Ui::FontMid(), TRUE);
        hLight = Ui::Ctl(h, L"BUTTON", L"", BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP, S(30), S(124), S(120), S(24), ID_LIGHT);
        hDark  = Ui::Ctl(h, L"BUTTON", L"", BS_AUTORADIOBUTTON, S(160), S(124), S(120), S(24), ID_DARK);

        SendMessageW(Cfg().lang == Lang::RU ? hRU : hEN, BM_SETCHECK, BST_CHECKED, 0);
        SendMessageW(Cfg().theme == Theme::Light ? hLight : hDark, BM_SETCHECK, BST_CHECKED, 0);
        SetTexts();
        Ui::Refresh(h);
        return 0;
    case WM_COMMAND:
        if (LOWORD(wp) == IDCANCEL) { DestroyWindow(h); return 0; }
        if (HIWORD(wp) == BN_CLICKED) {
            switch (LOWORD(wp)) {
            case ID_RU:    Cfg().lang  = Lang::RU;     break;
            case ID_EN:    Cfg().lang  = Lang::EN;     break;
            case ID_LIGHT: Cfg().theme = Theme::Light; break;
            case ID_DARK:  Cfg().theme = Theme::Dark;  break;
            default: return 0;
            }
            SaveConfig();
            Ui::BroadcastRefresh();
        }
        return 0;
    case WM_EGOIST_REFRESH:
        SetTexts();
        Ui::Refresh(h);
        return 0;
    case WM_ERASEBKGND: {
        RECT rc;
        GetClientRect(h, &rc);
        FillRect((HDC)wp, &rc, Ui::BgBrush());
        return 1;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
        return Ui::CtlColor((HDC)wp);
    case WM_CLOSE:
        DestroyWindow(h);
        return 0;
    case WM_DESTROY:
        g_set = nullptr;
        return 0;
    }
    return DefWindowProcW(h, msg, wp, lp);
}

}

void ShowSettings(HWND owner) {
    if (g_set) { SetForegroundWindow(g_set); return; }

    static bool registered = false;
    if (!registered) {
        WNDCLASSEXW wc = { sizeof(wc) };
        wc.lpfnWndProc   = SettingsProc;
        wc.hInstance     = GetModuleHandleW(nullptr);
        wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
        wc.lpszClassName = L"EgoistSettings";
        RegisterClassExW(&wc);
        registered = true;
    }

    DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU;
    RECT r = { 0, 0, Ui::S(320), Ui::S(168) };
    AdjustWindowRect(&r, style, FALSE);
    HWND h = CreateWindowExW(0, L"EgoistSettings", L"", style, 0, 0, r.right - r.left, r.bottom - r.top,
                             owner, nullptr, GetModuleHandleW(nullptr), nullptr);
    Ui::CenterOn(h, owner);
    ShowWindow(h, SW_SHOW);
}
