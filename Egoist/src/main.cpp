#include "app.h"
#include "Blockers.h"
#include "Config.h"
#include "Image.h"
#include "Lang.h"
#include "Ui.h"
#include <commctrl.h>
#include <vector>

namespace {

enum { ID_DISABLE = 1001, ID_ABOUT, ID_SETTINGS, ID_NEXT, ID_BACK, ID_CHECK0 = 2000 };
enum Page { PAGE_MAIN = 0, PAGE_INFO = 1, PAGE_LIST = 2 };

HWND g_wnd, hTitle, hDesc, hDisable, hAbout, hSettings, hPushTitle, hPushInfo, hNext, hBack;
std::vector<HWND> hChecks;
int g_page = PAGE_MAIN;

int ListBackY() { return Ui::S(90) + (int)Blockers().size() * Ui::S(28) + Ui::S(16); }

int ClientH() {
    if (g_page == PAGE_LIST) return ListBackY() + Ui::S(40) + Ui::S(24);
    return Ui::S(240);
}

void FitWindow(HWND h) {
    RECT r = { 0, 0, Ui::S(560), ClientH() };
    AdjustWindowRect(&r, (DWORD)GetWindowLongW(h, GWL_STYLE), FALSE);
    SetWindowPos(h, nullptr, 0, 0, r.right - r.left, r.bottom - r.top,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void Vis(HWND c, bool v) { ShowWindow(c, v ? SW_SHOW : SW_HIDE); }

void ShowPage(HWND h, int page) {
    using Ui::S;
    g_page = page;
    bool main = page == PAGE_MAIN, info = page == PAGE_INFO, list = page == PAGE_LIST;

    Vis(hTitle, main);
    Vis(hDesc, main);
    Vis(hDisable, main);
    Vis(hAbout, main);
    Vis(hSettings, main);

    Vis(hPushTitle, info || list);
    Vis(hPushInfo, info);
    Vis(hNext, info);
    Vis(hBack, info || list);
    for (HWND c : hChecks) Vis(c, list);

    if (info) {
        MoveWindow(hBack, S(20), S(150), S(200), S(44), TRUE);
        MoveWindow(hNext, S(340), S(150), S(200), S(44), TRUE);
    } else if (list) {
        MoveWindow(hBack, S(20), ListBackY(), S(200), S(44), TRUE);
    }

    FitWindow(h);
    InvalidateRect(h, nullptr, TRUE);
}

void SetTexts() {
    SetWindowTextW(g_wnd, T(Str::WindowTitle));
    SetWindowTextW(hTitle, T(Str::AppName));
    SetWindowTextW(hDesc, T(Str::Description));
    SetWindowTextW(hDisable, T(Str::BtnDisable));
    SetWindowTextW(hAbout, T(Str::BtnAbout));
    SetWindowTextW(hSettings, T(Str::BtnSettings));
    SetWindowTextW(hPushTitle, T(Str::PushTitle));
    SetWindowTextW(hPushInfo, T(Str::PushInfo));
    SetWindowTextW(hNext, T(Str::BtnNext));
    SetWindowTextW(hBack, T(Str::BtnBack));
    for (size_t i = 0; i < hChecks.size(); ++i)
        SetWindowTextW(hChecks[i], T(Blockers()[i].label));
}

void OnCheck(HWND parent, size_t i) {
    const Blocker& b = Blockers()[i];
    bool on = SendMessageW(hChecks[i], BM_GETCHECK, 0, 0) == BST_CHECKED;
    if (b.set(on)) {
        MessageBoxW(parent, T(on ? b.note : Str::Reverted), T(Str::AppName), MB_OK | MB_ICONINFORMATION);
    } else {
        SendMessageW(hChecks[i], BM_SETCHECK, on ? BST_UNCHECKED : BST_CHECKED, 0);
        MessageBoxW(parent, T(Str::NeedAdmin), T(Str::AppName), MB_OK | MB_ICONWARNING);
    }
}

LRESULT CALLBACK MainProc(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    using Ui::S;
    switch (msg) {
    case WM_CREATE: {
        g_wnd = h;
        hTitle = Ui::Ctl(h, L"STATIC", L"", SS_LEFT, S(84), S(18), S(440), S(50));
        SendMessageW(hTitle, WM_SETFONT, (WPARAM)Ui::FontBig(), TRUE);
        hDesc = Ui::Ctl(h, L"STATIC", L"", SS_LEFT, S(20), S(84), S(520), S(60));
        hDisable = Ui::Ctl(h, L"BUTTON", L"", BS_PUSHBUTTON | WS_TABSTOP, S(20), S(150), S(260), S(72), ID_DISABLE);
        SendMessageW(hDisable, WM_SETFONT, (WPARAM)Ui::FontMid(), TRUE);
        hAbout    = Ui::Ctl(h, L"BUTTON", L"", BS_PUSHBUTTON | WS_TABSTOP, S(290), S(150), S(250), S(34), ID_ABOUT);
        hSettings = Ui::Ctl(h, L"BUTTON", L"", BS_PUSHBUTTON | WS_TABSTOP, S(290), S(188), S(250), S(34), ID_SETTINGS);

        hPushTitle = Ui::Ctl(h, L"STATIC", L"", SS_LEFT, S(20), S(24), S(520), S(48));
        SendMessageW(hPushTitle, WM_SETFONT, (WPARAM)Ui::FontBig(), TRUE);
        hPushInfo = Ui::Ctl(h, L"STATIC", L"", SS_LEFT, S(20), S(86), S(520), S(40));
        SendMessageW(hPushInfo, WM_SETFONT, (WPARAM)Ui::FontMid(), TRUE);
        hNext = Ui::Ctl(h, L"BUTTON", L"", BS_PUSHBUTTON | WS_TABSTOP, S(340), S(150), S(200), S(44), ID_NEXT);
        SendMessageW(hNext, WM_SETFONT, (WPARAM)Ui::FontMid(), TRUE);
        hBack = Ui::Ctl(h, L"BUTTON", L"", BS_PUSHBUTTON | WS_TABSTOP, S(20), S(150), S(200), S(44), ID_BACK);
        SendMessageW(hBack, WM_SETFONT, (WPARAM)Ui::FontMid(), TRUE);

        const auto& list = Blockers();
        for (size_t i = 0; i < list.size(); ++i) {
            HWND c = Ui::Ctl(h, L"BUTTON", L"", BS_AUTOCHECKBOX | WS_TABSTOP,
                             S(24), S(90) + (int)i * S(28), S(500), S(24), ID_CHECK0 + (int)i);
            if (list[i].isActive()) SendMessageW(c, BM_SETCHECK, BST_CHECKED, 0);
            hChecks.push_back(c);
        }
        SetTexts();
        Ui::Refresh(h);
        ShowPage(h, PAGE_MAIN);
        return 0;
    }
    case WM_COMMAND: {
        int id = LOWORD(wp);
        if (id == ID_DISABLE) {
            ShowPage(h, PAGE_INFO);
        } else if (id == ID_NEXT) {
            ShowPage(h, PAGE_LIST);
        } else if (id == ID_BACK) {
            ShowPage(h, g_page == PAGE_LIST ? PAGE_INFO : PAGE_MAIN);
        } else if (id == ID_ABOUT) {
            ShowAbout(h);
        } else if (id == ID_SETTINGS) {
            ShowSettings(h);
        } else if (id >= ID_CHECK0 && id < ID_CHECK0 + (int)hChecks.size() && HIWORD(wp) == BN_CLICKED) {
            OnCheck(h, (size_t)(id - ID_CHECK0));
        }
        return 0;
    }
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
        if (g_page == PAGE_MAIN) Img::Draw(dc, Img::Logo(), S(20), S(16), S(52), S(52));
        EndPaint(h, &ps);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(h, msg, wp, lp);
}

}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE, LPSTR, int show) {
    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icc);

    LoadConfig();
    Ui::Init();
    Img::Init();
    ReapplyActive();

    WNDCLASSEXW wc = { sizeof(wc) };
    wc.lpfnWndProc   = MainProc;
    wc.hInstance     = inst;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hIcon         = Img::ToIcon(Img::Logo(), 32);
    wc.hIconSm       = Img::ToIcon(Img::Logo(), 16);
    wc.lpszClassName = L"EgoistMain";
    RegisterClassExW(&wc);

    HWND h = CreateWindowExW(0, L"EgoistMain", L"Egoist Beta",
                             WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                             CW_USEDEFAULT, CW_USEDEFAULT, 100, 100,
                             nullptr, nullptr, inst, nullptr);
    FitWindow(h);
    Ui::CenterOn(h, nullptr);
    ShowWindow(h, show);
    UpdateWindow(h);

    MSG m;
    while (GetMessageW(&m, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(GetAncestor(m.hwnd, GA_ROOT), &m)) {
            TranslateMessage(&m);
            DispatchMessageW(&m);
        }
    }
    return (int)m.wParam;
}
