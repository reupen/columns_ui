#include "stdafx.h"

void CheckGdiplusStatus::g_CheckGdiplusStatus(Gdiplus::Status pStatus)
{
    switch (pStatus)
    {
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
    };
}

HBITMAP g_CreateHbitmapFromGdiplusBitmapData32bpp(const Gdiplus::BitmapData & pBitmapData)
{
    pfc::array_t<t_uint8> bm_data;
    bm_data.set_size(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 0);
    bm_data.fill(0);

    BITMAPINFOHEADER bmi;
    memset(&bmi, 0, sizeof(bmi));

    bmi.biSize = sizeof(bmi);
    bmi.biWidth = pBitmapData.Width;
    bmi.biHeight = -gsl::narrow<long>(pBitmapData.Height);
    bmi.biPlanes = 1;
    bmi.biBitCount = 32;
    bmi.biCompression = BI_RGB;
    bmi.biClrUsed = 0;
    bmi.biClrImportant = 0;

    BITMAPINFO & bi = *(BITMAPINFO*)bm_data.get_ptr();

    bi.bmiHeader = bmi;

    void * data = nullptr;
    HBITMAP bm = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, &data, nullptr, 0);

    if (data)
    {
        auto * ptr = (char*)data;

        GdiFlush();

        memcpy(ptr, pBitmapData.Scan0, pBitmapData.Stride*pBitmapData.Height);
    }
    return bm;
}

HBITMAP g_load_png_gdiplus_throw(HDC dc, const char * fn, unsigned target_cx, unsigned target_cy)
{
    //FIXME m_gdiplus_initialised = (Gdiplus::Ok == Gdiplus::GdiplusStartup(&m_gdiplus_instance, &Gdiplus::GdiplusStartupInput(), NULL));
    pfc::string8 canPath;
    HBITMAP bm = nullptr;

    abort_callback_dummy p_abort;
    file::ptr p_file;
    filesystem::g_get_canonical_path(fn, canPath);
    filesystem::g_open_read(p_file, canPath, p_abort);
    auto fsize = gsl::narrow<t_size>(p_file->get_size_ex(p_abort));
    pfc::array_staticsize_t<t_uint8> buffer(fsize);
    p_file->read(buffer.get_ptr(), fsize, p_abort);
    p_file.release();

    IStream *pStream = nullptr;
    HGLOBAL gd = GlobalAlloc(GMEM_FIXED | GMEM_MOVEABLE, buffer.get_size());
    if (gd == nullptr)
        throw exception_win32(GetLastError());
    void * p_data = GlobalLock(gd);
    if (p_data == nullptr)
    {
        GlobalFree(gd);
        throw exception_win32(GetLastError());
    }

    memcpy(p_data, buffer.get_ptr(), buffer.get_size());
    GlobalUnlock(gd);

    HRESULT hr = CreateStreamOnHGlobal(gd, TRUE, &pStream);
    if (FAILED(hr))
    {
        GlobalFree(gd);
        throw exception_win32(hr);
    }

    Gdiplus::Bitmap pImage(pStream);

    CheckGdiplusStatus() << pImage.GetLastStatus();
    {
        Gdiplus::BitmapData bitmapData;
        //Gdiplus::Bitmap * ppImage = &pImage;
        if (target_cx != pfc_infinite || target_cy != pfc_infinite)
        {
            Gdiplus::Bitmap pBitmapResized(target_cx == pfc_infinite ? pImage.GetWidth() : target_cx, target_cy == pfc_infinite ? pImage.GetHeight() : target_cy, PixelFormat32bppARGB);
            CheckGdiplusStatus() << pBitmapResized.GetLastStatus();
            Gdiplus::Graphics pGraphics(&pBitmapResized);
            CheckGdiplusStatus() << pGraphics.GetLastStatus();
            CheckGdiplusStatus() << pGraphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
            CheckGdiplusStatus() << pGraphics.SetInterpolationMode(Gdiplus::InterpolationModeBicubic);
            Gdiplus::ImageAttributes imageAttributes;
            CheckGdiplusStatus() << imageAttributes.SetWrapMode(Gdiplus::WrapModeTileFlipXY);
            CheckGdiplusStatus() << pGraphics.DrawImage(&pImage, Gdiplus::Rect(0, 0, pBitmapResized.GetWidth(), pBitmapResized.GetHeight()), 0, 0, pImage.GetWidth(), pImage.GetHeight(), Gdiplus::UnitPixel, &imageAttributes);

            Gdiplus::Rect rect(0, 0, pBitmapResized.GetWidth(), pBitmapResized.GetHeight());
            CheckGdiplusStatus() << pBitmapResized.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bitmapData);
            bm = g_CreateHbitmapFromGdiplusBitmapData32bpp(bitmapData);
            CheckGdiplusStatus() << pBitmapResized.UnlockBits(&bitmapData);
        }
        else
        {
            Gdiplus::Rect rect(0, 0, pImage.GetWidth(), pImage.GetHeight());
            CheckGdiplusStatus() << pImage.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bitmapData);
            //assert bitmapData.Stride == bitmapData.Width
            bm = g_CreateHbitmapFromGdiplusBitmapData32bpp(bitmapData);
            CheckGdiplusStatus() << pImage.UnlockBits(&bitmapData);
        }
    }
    return bm;

}

HBITMAP g_load_png_gdiplus(HDC dc, const char * fn, unsigned target_cx, unsigned target_cy)
{
    try
    {
        return g_load_png_gdiplus_throw(dc, fn, target_cx, target_cy);
    }
    catch (pfc::exception const & ex)
    {
        console::formatter formatter;
        formatter << "Columns UI: Error loading image \"" << fn << "\" - " << ex.what();
        return nullptr;
    }
}
