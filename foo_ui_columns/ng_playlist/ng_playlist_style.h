#pragma once

#include "common.h"

namespace cui::panels::playlist_view {

struct CellStyleData {
    Colour text_colour;
    Colour selected_text_colour;
    Colour background_colour;
    Colour selected_background_colour;
    Colour selected_text_colour_non_focus;
    Colour selected_background_colour_non_focus;
    Colour frame_left;
    Colour frame_top;
    Colour frame_right;
    Colour frame_bottom;
    bool use_frame_left : 1 {};
    bool use_frame_top : 1 {};
    bool use_frame_right : 1 {};
    bool use_frame_bottom : 1 {};
    uih::text_style::FormatProperties format_properties;

    static CellStyleData create_default();

    auto operator<=>(const CellStyleData&) const = default;

    COLORREF get_text_colour(bool is_selected, bool is_focused) const
    {
        if (is_selected)
            return is_focused ? selected_text_colour : selected_text_colour_non_focus;

        return text_colour;
    }

    COLORREF get_background_colour(bool is_selected, bool is_focused) const
    {
        if (is_selected)
            return is_focused ? selected_background_colour : selected_background_colour_non_focus;

        return background_colour;
    }
};

class SharedCellStyleData
    : public pfc::refcounted_object_root
    , public CellStyleData {
public:
    using self_t = SharedCellStyleData;
    using ptr = pfc::refcounted_object_ptr_t<self_t>;

    explicit SharedCellStyleData(const CellStyleData& in) : CellStyleData(in) {}
    SharedCellStyleData(const SharedCellStyleData&) = delete;
    SharedCellStyleData& operator=(const SharedCellStyleData&) = delete;
    SharedCellStyleData(SharedCellStyleData&&) = delete;
    SharedCellStyleData& operator=(SharedCellStyleData&&) = delete;
    ~SharedCellStyleData() override;
};

namespace style_cache_manager {

void g_add_object(const CellStyleData& p_data, SharedCellStyleData::ptr& p_out);
void g_remove_object(SharedCellStyleData* p_object);

} // namespace style_cache_manager
using style_data_t = pfc::array_t<SharedCellStyleData::ptr>;

class StyleTitleformatHook : public titleformat_hook {
    CellStyleData p_default_colours;
    pfc::array_t<char> text, selected_text, back, selected_back, selected_back_no_focus, selected_text_no_focus;
    std::optional<std::string> m_index_text;
    CellStyleData& p_colours;
    size_t m_index;
    bool m_is_group;

public:
    bool process_field(
        titleformat_text_out* p_out, const char* p_name, size_t p_name_length, bool& p_found_flag) override;
    bool process_function(titleformat_text_out* p_out, const char* p_name, size_t p_name_length,
        titleformat_hook_function_params* p_params, bool& p_found_flag) override;

    StyleTitleformatHook(CellStyleData& vars, size_t index, bool b_is_group = false)
        : p_default_colours(vars)
        , p_colours(vars)
        , m_index(index)
        , m_is_group(b_is_group)
    {
    }
};
} // namespace cui::panels::playlist_view
