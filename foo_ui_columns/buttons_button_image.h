#pragma once

#include "buttons_button.h"

namespace cui::toolbars::buttons {

class ButtonImage {
public:
    ButtonImage() = default;
    ButtonImage(const ButtonImage&) = delete;
    ButtonImage& operator=(const ButtonImage&) = delete;
    ButtonImage(ButtonImage&&) = delete;
    ButtonImage& operator=(ButtonImage&&) = delete;

    bool is_valid() const;
    void preload(const CustomImage& p_image);
    bool load(std::optional<std::reference_wrapper<CustomImage>> custom_image,
        const service_ptr_t<uie::button>& button_ptr, COLORREF colour_btnface, int width, int height);
    unsigned add_to_imagelist(HIMAGELIST iml);
    [[nodiscard]] std::optional<std::tuple<int, int>> get_size() const { return m_bitmap_source_size; }

private:
    bool load_custom_image(const CustomImage& custom_image, int width, int height);
    void load_custom_svg_image(const char* full_path, int width, int height);
    void load_default_image(
        const service_ptr_t<uie::button>& button_ptr, COLORREF colour_btnface, int width, int height);

    wil::com_ptr<IWICBitmapSource> m_bitmap_source;
    std::optional<std::tuple<int, int>> m_bitmap_source_size;

    wil::unique_hbitmap m_bm;
    wil::unique_hicon m_icon;
    ui_extension::t_mask m_mask_type{uie::MASK_NONE};
    wil::unique_hbitmap m_bm_mask;
    COLORREF m_mask_colour{0};
};

} // namespace cui::toolbars::buttons
