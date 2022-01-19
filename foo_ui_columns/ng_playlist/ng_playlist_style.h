#pragma once

namespace cui::panels::playlist_view {
class CellStyleData {
public:
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
    bool use_frame_left : 1;
    bool use_frame_top : 1;
    bool use_frame_right : 1;
    bool use_frame_bottom : 1;

    static CellStyleData g_create_default();

    void set(const CellStyleData* in)
    {
        text_colour = in->text_colour;
        selected_text_colour = in->selected_text_colour;
        background_colour = in->background_colour;
        selected_background_colour = in->selected_background_colour;
        selected_text_colour_non_focus = in->selected_text_colour_non_focus;
        selected_background_colour_non_focus = in->selected_background_colour_non_focus;
        frame_left = in->frame_left;
        frame_top = in->frame_top;
        frame_right = in->frame_right;
        frame_bottom = in->frame_bottom;
        use_frame_left = in->use_frame_left;
        use_frame_top = in->use_frame_top;
        use_frame_right = in->use_frame_right;
        use_frame_bottom = in->use_frame_bottom;
    }

    CellStyleData() : use_frame_left(false), use_frame_top(false), use_frame_right(false), use_frame_bottom(false) {}

    CellStyleData(COLORREF text, COLORREF text_sel, COLORREF back, COLORREF back_sel, COLORREF text_no_focus,
        COLORREF sel_no_focus)
        : use_frame_left(false)
        , use_frame_top(false)
        , use_frame_right(false)
        , use_frame_bottom(false)
    {
        text_colour.set(text);
        selected_text_colour.set(text_sel);
        background_colour.set(back);
        selected_background_colour.set(back_sel);
        selected_text_colour_non_focus.set(text_no_focus);
        selected_background_colour_non_focus.set(sel_no_focus);
    }

    bool is_equal(const CellStyleData& c2)
    {
        const CellStyleData& c1 = *this;
        return (c1.text_colour == c2.text_colour && c1.selected_text_colour == c2.selected_text_colour
            && c1.background_colour == c2.background_colour
            && c1.selected_background_colour == c2.selected_background_colour
            && c1.selected_text_colour_non_focus == c2.selected_text_colour_non_focus
            && c1.selected_background_colour_non_focus == c2.selected_background_colour_non_focus
            && c1.use_frame_left == c2.use_frame_left && c1.use_frame_right == c2.use_frame_right
            && c1.use_frame_bottom == c2.use_frame_bottom && c1.use_frame_top == c2.use_frame_top
            && (c1.use_frame_left ? c1.frame_left == c2.frame_left : true)
            && (c1.use_frame_bottom ? c1.frame_bottom == c2.frame_bottom : true)
            && (c1.use_frame_top ? c1.frame_top == c2.frame_top : true)
            && (c1.use_frame_right ? c1.frame_right == c2.frame_right : true));
    }
};
class SharedCellStyleData
    : public pfc::refcounted_object_root
    , public CellStyleData {
public:
    using self_t = SharedCellStyleData;
    using ptr = pfc::refcounted_object_ptr_t<self_t>;

    SharedCellStyleData(const CellStyleData& in) : CellStyleData(in) {}
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
    pfc::array_t<char> text, selected_text, back, selected_back, selected_back_no_focus, selected_text_no_focus,
        m_index_text;
    CellStyleData& p_colours;
    t_size m_index;
    bool m_is_group;

public:
    bool process_field(
        titleformat_text_out* p_out, const char* p_name, unsigned p_name_length, bool& p_found_flag) override;
    bool process_function(titleformat_text_out* p_out, const char* p_name, unsigned p_name_length,
        titleformat_hook_function_params* p_params, bool& p_found_flag) override;

    StyleTitleformatHook(CellStyleData& vars, t_size index, bool b_is_group = false)
        : p_default_colours(vars)
        , p_colours(vars)
        , m_index(index)
        , m_is_group(b_is_group)
    {
    }
};
} // namespace cui::panels::playlist_view
