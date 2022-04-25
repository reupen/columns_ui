#include "pch.h"
#include "buttons.h"
#include "wic.h"

namespace cui::toolbars::buttons {

bool ButtonsToolbar::ButtonImage::is_valid() const
{
    return m_bm || m_icon;
}

void ButtonsToolbar::ButtonImage::load(const Button::CustomImage& p_image)
{
    TRACK_CALL_TEXT("cui::ButtonsToolbar::ButtonImage::load");

    m_mask_type = p_image.m_mask_type;
    m_mask_colour = p_image.m_mask_colour;

    pfc::string8 full_path;
    p_image.get_path(full_path);

    if (!_stricmp(string_extension(full_path), "bmp")) // Gdiplus vs 32bpp
        m_bm.reset(static_cast<HBITMAP>(uLoadImage(
            wil::GetModuleInstanceHandle(), full_path, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE)));
    else if (!_stricmp(string_extension(full_path), "ico"))
        m_icon.reset(static_cast<HICON>(uLoadImage(wil::GetModuleInstanceHandle(), full_path, IMAGE_ICON,
            GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_LOADFROMFILE)));
    else {
        try {
            m_bm.reset(wic::create_hbitmap_from_path(full_path).release());
        } catch (const std::exception& ex) {
            fbh::print_to_console(u8"Buttons toolbar – loading image failed: "_pcc, ex.what());
        }
    }

    if (!m_bm && !m_icon)
        console::printf("failed loading image \"%s\"", full_path.get_ptr());
}
void ButtonsToolbar::ButtonImage::load(
    const service_ptr_t<uie::button>& p_in, COLORREF colour_btnface, unsigned cx, unsigned cy)
{
    uie::button_v2::ptr inv2;
    if (p_in->service_query_t(inv2)) {
        unsigned handle_type = 0;
        HANDLE image = inv2->get_item_bitmap(0, colour_btnface, cx, cy, handle_type);
        if (handle_type == uie::button_v2::handle_type_bitmap)
            m_bm.reset(static_cast<HBITMAP>(image));
        else if (handle_type == uie::button_v2::handle_type_icon)
            m_icon.reset(static_cast<HICON>(image));
    } else
        m_bm.reset(p_in->get_item_bitmap(0, colour_btnface, m_mask_type, m_mask_colour, *m_bm_mask.put()));
}
unsigned ButtonsToolbar::ButtonImage::add_to_imagelist(HIMAGELIST iml)
{
    unsigned rv = I_IMAGECALLBACK;
    if (m_icon) {
        rv = ImageList_ReplaceIcon(iml, -1, m_icon.get());
    } else if (m_bm) {
        switch (m_mask_type) {
        default:
            rv = ImageList_Add(iml, m_bm.get(), nullptr);
            break;
        case ui_extension::MASK_COLOUR:
            rv = ImageList_AddMasked(iml, m_bm.get(), m_mask_colour);
            break;
        case ui_extension::MASK_BITMAP: {
            rv = ImageList_Add(iml, m_bm.get(), m_bm_mask.get());
        } break;
        }
    }
    return rv;
}

SIZE ButtonsToolbar::ButtonImage::get_size() const
{
    if (m_icon) {
        ICONINFO ii{};
        if (!GetIconInfo(m_icon.get(), &ii))
            return {};

        const wil::unique_hbitmap colour_bitmap(ii.hbmColor);
        const wil::unique_hbitmap mask_bitmap(ii.hbmMask);

        BITMAP bmi{};
        if (!GetObject(ii.hbmColor ? ii.hbmColor : ii.hbmMask, sizeof(BITMAP), &bmi))
            return {};

        return {bmi.bmWidth, ii.hbmColor ? bmi.bmHeight : bmi.bmHeight / 2};
    }

    if (!m_bm)
        return {};

    BITMAP bmi{};
    if (!GetObject(m_bm.get(), sizeof(BITMAP), &bmi))
        return {};

    return {bmi.bmWidth, bmi.bmHeight};
}

} // namespace cui::toolbars::buttons
