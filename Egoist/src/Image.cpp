#include "Image.h"
#include "resource.h"
#include <cstring>

#pragma comment(lib, "gdiplus.lib")

namespace Img {

static ULONG_PTR g_token = 0;

void Init() {
    Gdiplus::GdiplusStartupInput in;
    Gdiplus::GdiplusStartup(&g_token, &in, nullptr);
}

static Gdiplus::Bitmap* Detach(Gdiplus::Bitmap* src) {
    if (!src) return nullptr;
    Gdiplus::Bitmap* copy = src->Clone(0, 0, (INT)src->GetWidth(), (INT)src->GetHeight(), PixelFormat32bppARGB);
    delete src;
    return copy;
}

Gdiplus::Bitmap* FromResource(int resId) {
    HRSRC r = FindResourceW(nullptr, MAKEINTRESOURCEW(resId), RT_RCDATA);
    if (!r) return nullptr;
    HGLOBAL g = LoadResource(nullptr, r);
    DWORD size = SizeofResource(nullptr, r);
    void* data = g ? LockResource(g) : nullptr;
    if (!data || !size) return nullptr;

    HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!mem) return nullptr;
    std::memcpy(GlobalLock(mem), data, size);
    GlobalUnlock(mem);

    IStream* stream = nullptr;
    if (CreateStreamOnHGlobal(mem, TRUE, &stream) != S_OK) { GlobalFree(mem); return nullptr; }
    Gdiplus::Bitmap* bmp = Detach(Gdiplus::Bitmap::FromStream(stream));
    stream->Release();
    return bmp;
}

Gdiplus::Bitmap* FromFile(const wchar_t* path) {
    return Detach(Gdiplus::Bitmap::FromFile(path));
}

void Draw(HDC dc, Gdiplus::Bitmap* bmp, int x, int y, int w, int h) {
    if (!bmp) return;
    Gdiplus::Graphics g(dc);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
    g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    g.DrawImage(bmp, Gdiplus::Rect(x, y, w, h));
}

HICON ToIcon(Gdiplus::Bitmap* bmp, int size) {
    if (!bmp) return nullptr;
    Gdiplus::Bitmap tmp(size, size, PixelFormat32bppARGB);
    {
        Gdiplus::Graphics g(&tmp);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        g.DrawImage(bmp, Gdiplus::Rect(0, 0, size, size));
    }
    HICON icon = nullptr;
    tmp.GetHICON(&icon);
    return icon;
}

Gdiplus::Bitmap* Logo() {
    static Gdiplus::Bitmap* logo = FromResource(IDR_LOGO);
    return logo;
}

}
