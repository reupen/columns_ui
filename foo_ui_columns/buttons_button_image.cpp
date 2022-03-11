#include "pch.h"
#include "buttons.h"
#include "wic.h"

namespace cui::toolbars::buttons {

ButtonsToolbar::ButtonImage::~ButtonImage()
{
    if (m_bm)
        DeleteBitmap(m_bm);
    if (m_bm_mask)
        DeleteBitmap(m_bm_mask);
    if (m_icon)
        DestroyIcon(m_icon);
}
bool ButtonsToolbar::ButtonImage::is_valid()
{
    return m_bm != nullptr;
}

void ButtonsToolbar::ButtonImage::load(const Button::CustomImage& p_image)
{
    TRACK_CALL_TEXT("cui::ButtonsToolbar::ButtonImage::load");

    m_mask_type = p_image.m_mask_type;
    m_mask_colour = p_image.m_mask_colour;

    pfc::string8 fullPath;
    p_image.get_path(fullPath);

    if (!_stricmp(string_extension(fullPath), "bmp")) // Gdiplus vs 32bpp
        m_bm = (HBITMAP)uLoadImage(
            core_api::get_my_instance(), fullPath, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
    else if (!_stricmp(string_extension(fullPath), "ico"))
        m_icon = (HICON)uLoadImage(core_api::get_my_instance(), fullPath, IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),
            GetSystemMetrics(SM_CYSMICON), LR_LOADFROMFILE);
    else {
        try {
            m_bm = wic::create_hbitmap_from_path(fullPath).release();
        } catch (const std::exception& ex) {
            fbh::print_to_console(u8"Buttons toolbar – loading image failed: "_pcc, ex.what());
            m_bm = nullptr;
        }
    }
    if (m_bm) {
        switch (p_image.m_mask_type) {
        default:
            break;
        case ui_extension::MASK_COLOUR:
            break;
#if 0
        case ui_extension::MASK_BITMAP:
        {
            if (!_stricmp(pfc::string_extension(p_image.m_mask_path), "png"))
            {
                m_bm_mask = read_png(0, p_image.m_mask_path);
            }
            else
                m_bm_mask = (HBITMAP)LoadImage(core_api::get_my_instance(), pfc::stringcvt::string_os_from_utf8(p_image.m_mask_path), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE | LR_MONOCHROME);
            if (!m_bm_mask)
                console::printf("failed loading image \"%s\"", p_image.m_mask_path.get_ptr());
        }
        break;
#endif
        }
    } else if (!m_icon)
        console::printf("failed loading image \"%s\"", fullPath.get_ptr());
}
void ButtonsToolbar::ButtonImage::load(
    const service_ptr_t<uie::button>& p_in, COLORREF colour_btnface, unsigned cx, unsigned cy)
{
    uie::button_v2::ptr inv2;
    if (p_in->service_query_t(inv2)) {
        unsigned handle_type = 0;
        HANDLE image = inv2->get_item_bitmap(0, colour_btnface, cx, cy, handle_type);
        if (handle_type == uie::button_v2::handle_type_bitmap)
            m_bm = (HBITMAP)image;
        else if (handle_type == uie::button_v2::handle_type_icon)
            m_icon = (HICON)image;
    } else
        m_bm = p_in->get_item_bitmap(0, colour_btnface, m_mask_type, m_mask_colour, m_bm_mask);
}
unsigned ButtonsToolbar::ButtonImage::add_to_imagelist(HIMAGELIST iml)
{
    unsigned rv = I_IMAGECALLBACK;
    if (m_icon) {
        rv = ImageList_ReplaceIcon(iml, -1, m_icon);
    } else if (m_bm) {
        switch (m_mask_type) {
        default:
            rv = ImageList_Add(iml, m_bm, nullptr);
            break;
        case ui_extension::MASK_COLOUR:
            rv = ImageList_AddMasked(iml, m_bm, m_mask_colour);
            break;
        case ui_extension::MASK_BITMAP: {
            rv = ImageList_Add(iml, m_bm, m_bm_mask);
        } break;
        }
    }
    return rv;
}
void ButtonsToolbar::ButtonImage::get_size(SIZE& p_out)
{
    p_out.cx = 0;
    p_out.cy = 0;
    BITMAP bmi{};
    if (m_icon) {
        ICONINFO ii{};
        if (GetIconInfo(m_icon, &ii)) {
            GetObject(ii.hbmColor ? ii.hbmColor : ii.hbmMask, sizeof(BITMAP), &bmi);
        }
    } else if (m_bm) {
        GetObject(m_bm, sizeof(BITMAP), &bmi);
    }
    p_out.cx = bmi.bmWidth;
    p_out.cy = bmi.bmHeight;
}

} // namespace cui::toolbars::buttons
