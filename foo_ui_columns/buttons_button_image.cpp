#include "pch.h"
#include "buttons.h"
#include "wic.h"

namespace cui::toolbars::buttons {

bool ButtonsToolbar::ButtonImage::is_valid() const
{
    return m_bm || m_icon;
}

void ButtonsToolbar::ButtonImage::preload(const Button::CustomImage& p_image)
{
    TRACK_CALL_TEXT("cui::ButtonsToolbar::ButtonImage::preload");

    m_mask_type = p_image.m_mask_type;
    m_mask_colour = p_image.m_mask_colour;

    if (p_image.is_ico())
        return;

    const pfc::string8 full_path = p_image.get_path();

    try {
        m_bitmap_source = wic::create_bitmap_source_from_path(full_path);
        unsigned width{};
        unsigned height{};
        wic::check_hresult(m_bitmap_source->GetSize(&width, &height));
        m_bitmap_source_size = std::make_tuple(gsl::narrow<int>(width), gsl::narrow<int>(height));
    } catch (const std::exception& ex) {
        fbh::print_to_console(u8"Buttons toolbar – loading image failed: "_pcc, ex.what());
    }
}

bool ButtonsToolbar::ButtonImage::load_custom_image(const Button::CustomImage& custom_image, int width, int height)
{
    const pfc::string8 full_path = custom_image.get_path();

    if (custom_image.is_ico()) {
        m_icon.reset(static_cast<HICON>(
            uLoadImage(wil::GetModuleInstanceHandle(), full_path, IMAGE_ICON, width, height, LR_LOADFROMFILE)));

        if (!m_icon)
            fbh::print_to_console(u8"Buttons toolbar – loading icon failed. Path: "_pcc, full_path.get_ptr());
        return false;
    }

    if (m_bitmap_source) {
        try {
            bool resized{};
            if (*m_bitmap_source_size != std::make_tuple(width, height)) {
                m_bitmap_source = wic::resize_bitmap_source(m_bitmap_source, width, height);
                resized = true;
            }
            m_bm = wic::create_hbitmap_from_bitmap_source(m_bitmap_source);
            return resized;
        } catch (const std::exception& ex) {
            fbh::print_to_console(
                u8"Buttons toolbar – loading image failed. Path: "_pcc, full_path.get_ptr(), " Error: ", ex.what());
        }
        m_bitmap_source.reset();
    }
    return false;
}

void ButtonsToolbar::ButtonImage::load_default_image(
    const service_ptr_t<uie::button>& button_ptr, COLORREF colour_btnface, int width, int height)
{
    uie::button_v2::ptr button_v2_ptr;
    if (!button_ptr->service_query_t(button_v2_ptr)) {
        m_bm.reset(button_ptr->get_item_bitmap(0, colour_btnface, m_mask_type, m_mask_colour, *m_bm_mask.put()));
        return;
    }

    unsigned handle_type = 0;
    const HANDLE image = button_v2_ptr->get_item_bitmap(0, colour_btnface, width, height, handle_type);

    if (handle_type == uie::button_v2::handle_type_icon) {
        m_icon.reset(static_cast<HICON>(image));
        return;
    }

    if (handle_type != uie::button_v2::handle_type_bitmap)
        return;

    wil::unique_hbitmap bitmap(static_cast<HBITMAP>(image));

    BITMAP bitmap_info{};
    if (!GetObject(bitmap.get(), sizeof(bitmap_info), &bitmap_info))
        return;

    if (bitmap_info.bmWidth == width && bitmap_info.bmHeight == height) {
        m_bm = std::move(bitmap);
        return;
    }

    try {
        m_bm = wic::resize_hbitmap(bitmap.get(), width, height);
    } catch (const std::exception& ex) {
        fbh::print_to_console(u8"Buttons toolbar – error resizing default image: "_pcc, ex.what());
    }

    return;
}

bool ButtonsToolbar::ButtonImage::load(std::optional<std::reference_wrapper<Button::CustomImage>> custom_image,
    const service_ptr_t<uie::button>& button_ptr, COLORREF colour_btnface, int width, int height)
{
    TRACK_CALL_TEXT("cui::ButtonsToolbar::ButtonImage::load");

    if (custom_image) {
        return load_custom_image(custom_image->get(), width, height);
    }

    if (button_ptr.is_valid()) {
        load_default_image(button_ptr, colour_btnface, width, height);
        return false;
    }

    return false;
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

} // namespace cui::toolbars::buttons
