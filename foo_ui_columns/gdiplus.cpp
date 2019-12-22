#include "stdafx.h"

namespace cui::gdip {

std::unique_ptr<Gdiplus::Bitmap> create_bitmap_from_32bpp_data(
    unsigned width, unsigned height, unsigned stride, const uint8_t* data, size_t size)
{
    auto bitmap = std::make_unique<Gdiplus::Bitmap>(width, height, PixelFormat32bppARGB);

    if (bitmap->GetLastStatus() != Gdiplus::Ok)
        return {};

    Gdiplus::BitmapData image_data{};
    auto status = bitmap->LockBits(nullptr, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &image_data);

    if (status != Gdiplus::Ok)
        return {};

    assert(image_data.Height == height);
    assert(image_data.Width == width);
    assert(image_data.Stride >= width * 4);

    const uint8_t* src = data;
    uint8_t* dst = static_cast<uint8_t*>(image_data.Scan0);
    for (auto i : ranges::views::iota(0u, image_data.Height)) {
        memcpy(dst, src, image_data.Width * 4);
        dst += image_data.Stride;
        src += stride;
    }

    status = bitmap->UnlockBits(&image_data);
    if (status != Gdiplus::Ok)
        return {};

    return bitmap;
}

void check_status(Gdiplus::Status status)
{
    switch (status) {
    case Gdiplus::Ok:
        break;
    case Gdiplus::GenericError:
        throw pfc::exception("GDI+ Error: Generic Error");
    case Gdiplus::InvalidParameter:
        throw pfc::exception("GDI+ Error: Invalid Parameter");
    case Gdiplus::OutOfMemory:
        throw pfc::exception("GDI+ Error: Out Of Memory");
    case Gdiplus::ObjectBusy:
        throw pfc::exception("GDI+ Error: Object Busy");
    case Gdiplus::InsufficientBuffer:
        throw pfc::exception("GDI+ Error: Insufficient Buffer");
    case Gdiplus::NotImplemented:
        throw pfc::exception("GDI+ Error: Not Implemented");
    case Gdiplus::Win32Error:
        throw pfc::exception("GDI+ Error: Win32 Error");
    case Gdiplus::WrongState:
        throw pfc::exception("GDI+ Error: Wrong State");
    case Gdiplus::Aborted:
        throw pfc::exception("GDI+ Error: Aborted");
    case Gdiplus::FileNotFound:
        throw pfc::exception("GDI+ Error: File Not Found");
    case Gdiplus::ValueOverflow:
        throw pfc::exception("GDI+ Error: Value Overflow");
    case Gdiplus::AccessDenied:
        throw pfc::exception("GDI+ Error: Access Denied");
    case Gdiplus::UnknownImageFormat:
        throw pfc::exception("GDI+ Error: Unknown Image Format");
    case Gdiplus::FontFamilyNotFound:
        throw pfc::exception("GDI+ Error: Font Family Not Found");
    case Gdiplus::FontStyleNotFound:
        throw pfc::exception("GDI+ Error: Font Style Not Found");
    case Gdiplus::NotTrueTypeFont:
        throw pfc::exception("GDI+ Error: Not TrueType Font");
    case Gdiplus::UnsupportedGdiplusVersion:
        throw pfc::exception("GDI+ Error: Unsupported Gdiplus Version");
    case Gdiplus::GdiplusNotInitialized:
        throw pfc::exception("GDI+ Error: Gdiplus Not Initialized");
    case Gdiplus::PropertyNotFound:
        throw pfc::exception("GDI+ Error: Property Not Found");
    case Gdiplus::PropertyNotSupported:
        throw pfc::exception("GDI+ Error: Property Not Supported");
#if (GDIPVER >= 0x0110)
    case Gdiplus::ProfileNotFound:
        throw pfc::exception("GDI+ Error: Profile Not Found");
#endif
    default:
        throw pfc::exception("GDI+ Error: Unknown Error");
    }
}

} // namespace cui::gdip
