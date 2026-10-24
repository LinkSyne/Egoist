#pragma once
#include <algorithm>
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
using std::min;
using std::max;

namespace Img {
    void Init();
    Gdiplus::Bitmap* FromResource(int resId);
    Gdiplus::Bitmap* FromFile(const wchar_t* path);
    void Draw(HDC dc, Gdiplus::Bitmap* bmp, int x, int y, int w, int h);
    HICON ToIcon(Gdiplus::Bitmap* bmp, int size);
    Gdiplus::Bitmap* Logo();
}
