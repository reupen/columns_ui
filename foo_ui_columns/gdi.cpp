#include "pch.h"

#include "gdi.h"

namespace cui::gdi {

wil::unique_hbitmap create_hbitmap_from_32bpp_data(
    int width, int height, const void* data, size_t size, std::optional<int> stride)
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
        if (!stride) {
            if (size != width * height * 4)
                uBugCheck();

            memcpy(hbitmap_data, data, size);
        } else {
            for (const auto row_index : std::views::iota(0, height))
                memcpy(static_cast<uint8_t*>(hbitmap_data) + row_index * width * 4,
                    static_cast<const uint8_t*>(data) + row_index * *stride, width * 4);
        }
    }

    return hbitmap;
}

} // namespace cui::gdi
