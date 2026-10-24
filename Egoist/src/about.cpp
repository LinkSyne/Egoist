#include "app.h"
#include "Image.h"
#include "Lang.h"
#include "Ui.h"

namespace {

enum { ID_AUTHOR = 301, ID_SITE = 302 };
const wchar_t* kAuthorUrl = L"https://t.me/vscode2026";
const wchar_t* kSiteUrl   = L"https://lnsyne.site.je/?i=1";

HWND g_about, hName, hVer, hAuthor, hSite;

void SetTexts() {
    SetWindowTextW(g_about, T(Str::AboutTitle));
    SetWindowTextW(hName, T(Str::AppName));
    SetWindowTextW(hVer, T(Str::Version));
    SetWindowTextW(hAuthor, T(Str::BtnAuthor));
    SetWindowTextW(hSite, T(Str::BtnSite));
}

LRESULT CALLBACK AboutProc(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    using Ui::S;
    switch (msg) {
    case WM_CREATE:
        g_about = h;
        hName = Ui::Ctl(h, L"STATIC", L"", SS_LEFT, S(100), S(16), S(260), S(50));
        SendMessageW(hName, WM_SETFONT, (WPARAM)Ui::FontBig(), TRUE);
        hVer = Ui::Ctl(h, L"STATIC", L"", SS_LEFT, S(102), S(70), S(260), S(24));
        SendMessageW(hVer, WM_SETFONT, (WPARAM)Ui::FontMid(), TRUE);
        hAuthor = Ui::Ctl(h, L"BUTTON", L"", BS_PUSHBUTTON | WS_TABSTOP, S(20), S(130), S(160), S(34), ID_AUTHOR);
        hSite   = Ui::Ctl(h, L"BUTTON", L"", BS_PUSHBUTTON | WS_TABSTOP, S(200), S(130), S(160), S(34), ID_SITE);
        SetTexts();
        Ui::Refresh(h);
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case ID_AUTHOR: Ui::OpenUrl(kAuthorUrl); break;
        case ID_SITE:   Ui::OpenUrl(kSiteUrl);   break;
        case IDCANCEL:  DestroyWindow(h);        break;
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
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(h, &ps);
        Img::Draw(dc, Img::Logo(), S(20), S(16), S(64), S(64));
        EndPaint(h, &ps);
        return 0;
    }
    case WM_CLOSE:
        DestroyWindow(h);
        return 0;
    case WM_DESTROY:
        g_about = nullptr;
        return 0;
    }
    return DefWindowProcW(h, msg, wp, lp);
}

}

void ShowAbout(HWND owner) {
    if (g_about) { SetForegroundWindow(g_about); return; }

    static bool registered = false;
    if (!registered) {
        WNDCLASSEXW wc = { sizeof(wc) };
        wc.lpfnWndProc   = AboutProc;
        wc.hInstance     = GetModuleHandleW(nullptr);
        wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
        wc.lpszClassName = L"EgoistAbout";
        RegisterClassExW(&wc);
        registered = true;
    }

    DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU;
    RECT r = { 0, 0, Ui::S(380), Ui::S(184) };
    AdjustWindowRect(&r, style, FALSE);
    HWND h = CreateWindowExW(0, L"EgoistAbout", L"", style, 0, 0, r.right - r.left, r.bottom - r.top,
                             owner, nullptr, GetModuleHandleW(nullptr), nullptr);
    Ui::CenterOn(h, owner);
    ShowWindow(h, SW_SHOW);
}
