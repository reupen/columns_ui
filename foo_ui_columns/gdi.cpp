#include "pch.h"

#include "gdi.h"

namespace cui::gdi {

wil::unique_hbitmap create_hbitmap_from_32bpp_data(int width, int height, const void* data, size_t size)
{
    BITMAPINFOHEADER bmi{};

    bmi.biSize = sizeof(bmi);
    bmi.biWidth = width;
    bmi.biHeight = -height;
    bmi.biPlanes = 1;
    bmi.biBitCount = 32;
    bmi.biCompression = BI_RGB;
    bmi.biClrUsed = 0;
    bmi.biClrImportant = 0;

    std::array<uint8_t, sizeof(BITMAPINFOHEADER)> bm_data{};

    auto* bi = reinterpret_cast<BITMAPINFO*>(bm_data.data());
    bi->bmiHeader = bmi;

    void* hbitmap_data{};
    wil::unique_hbitmap hbitmap(CreateDIBSection(nullptr, bi, DIB_RGB_COLORS, &hbitmap_data, nullptr, 0));

    if (hbitmap_data) {
        GdiFlush();
        memcpy(hbitmap_data, data, size);
    }

    return hbitmap;
}

} // namespace cui::gdi
