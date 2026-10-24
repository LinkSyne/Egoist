#include "Ui.h"
#include "app.h"
#include "Config.h"
#include <dwmapi.h>
#include <uxtheme.h>
#include <shellapi.h>
#include <cwchar>

namespace Ui {

static int g_dpi = 96;
static HFONT fNormal, fMid, fBig;
static HBRUSH bLight, bDark;

static HFONT MakeFont(int percent, int weight) {
    NONCLIENTMETRICSW m = { sizeof(m) };
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(m), &m, 0);
    LOGFONTW lf = m.lfMessageFont;
    lf.lfHeight = lf.lfHeight * percent / 100;
    if (weight) lf.lfWeight = weight;
    return CreateFontIndirectW(&lf);
}

void Init() {
    HDC dc = GetDC(nullptr);
    g_dpi = GetDeviceCaps(dc, LOGPIXELSX);
    ReleaseDC(nullptr, dc);
    fNormal = MakeFont(100, 0);
    fMid    = MakeFont(140, 0);
    fBig    = MakeFont(230, FW_BOLD);
    bLight  = CreateSolidBrush(RGB(243, 243, 243));
    bDark   = CreateSolidBrush(RGB(32, 32, 32));
}

int S(int v) { return MulDiv(v, g_dpi, 96); }
HFONT Font()    { return fNormal; }
HFONT FontMid() { return fMid; }
HFONT FontBig() { return fBig; }
bool Dark() { return Cfg().theme == Theme::Dark; }
HBRUSH BgBrush() { return Dark() ? bDark : bLight; }

LRESULT CtlColor(HDC dc) {
    SetTextColor(dc, Dark() ? RGB(240, 240, 240) : RGB(0, 0, 0));
    SetBkMode(dc, TRANSPARENT);
    return (LRESULT)BgBrush();
}

HWND Ctl(HWND parent, const wchar_t* cls, const wchar_t* text, DWORD style,
         int x, int y, int w, int h, int id) {
    HWND c = CreateWindowExW(0, cls, text, WS_CHILD | WS_VISIBLE | style, x, y, w, h,
                             parent, (HMENU)(INT_PTR)id, GetModuleHandleW(nullptr), nullptr);
    SendMessageW(c, WM_SETFONT, (WPARAM)fNormal, TRUE);
    return c;
}

static BOOL CALLBACK ThemeChild(HWND h, LPARAM) {
    wchar_t cls[32] = {};
    GetClassNameW(h, cls, 32);
    if (wcscmp(cls, L"Button") == 0)
        SetWindowTheme(h, Dark() ? L"DarkMode_Explorer" : nullptr, nullptr);
    return TRUE;
}

void Refresh(HWND top) {
    BOOL d = Dark();
    DwmSetWindowAttribute(top, 20 , &d, sizeof(d));
    EnumChildWindows(top, ThemeChild, 0);
    RedrawWindow(top, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_FRAME);
}

static BOOL CALLBACK SendRefresh(HWND h, LPARAM) {
    SendMessageW(h, WM_EGOIST_REFRESH, 0, 0);
    return TRUE;
}

void BroadcastRefresh() {
    EnumThreadWindows(GetCurrentThreadId(), SendRefresh, 0);
}

void CenterOn(HWND w, HWND owner) {
    RECT r, o;
    GetWindowRect(w, &r);
    if (owner) GetWindowRect(owner, &o);
    else SystemParametersInfoW(SPI_GETWORKAREA, 0, &o, 0);
    int x = o.left + ((o.right - o.left) - (r.right - r.left)) / 2;
    int y = o.top + ((o.bottom - o.top) - (r.bottom - r.top)) / 2;
    SetWindowPos(w, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void OpenUrl(const wchar_t* url) {
    ShellExecuteW(nullptr, L"open", url, nullptr, nullptr, SW_SHOWNORMAL);
}

}
