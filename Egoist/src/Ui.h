#pragma once
#include <windows.h>

namespace Ui {
    void Init();
    int  S(int v);
    HFONT Font();
    HFONT FontMid();
    HFONT FontBig();
    bool Dark();
    HBRUSH BgBrush();
    LRESULT CtlColor(HDC dc);
    HWND Ctl(HWND parent, const wchar_t* cls, const wchar_t* text, DWORD style,
             int x, int y, int w, int h, int id = 0);
    void Refresh(HWND top);
    void BroadcastRefresh();
    void CenterOn(HWND w, HWND owner);
    void OpenUrl(const wchar_t* url);
}
