#pragma once

#include "dark_mode.h"
#include "font_utils.h"

namespace cui::utils {

template <GUID ColoursClientId, GUID ItemsFontId, GUID HeaderFontId = GUID{}, GUID GroupFontId = GUID{},
    typename t_window = uie::window>
class ListViewPanelBase
    : public uih::ListView
    , public t_window {
public:
    explicit ListViewPanelBase(
        std::unique_ptr<uih::lv::RendererBase> renderer = std::make_unique<uih::lv::DefaultRenderer>())
        : ListView(std::move(renderer))
    {
    }

    HWND create_or_transfer_window(
        HWND parent, const uie::window_host_ptr& host, const ui_helpers::window_position_t& p_position) override
    {
        if (get_wnd()) {
            ShowWindow(get_wnd(), SW_HIDE);
            SetParent(get_wnd(), parent);
            m_window_host->relinquish_ownership(get_wnd());
            m_window_host = host;

            SetWindowPos(get_wnd(), nullptr, p_position.x, p_position.y, p_position.cx, p_position.cy, SWP_NOZORDER);
        } else {
            m_window_host = host;

            set_dark_edit_colours(
                dark::get_dark_system_colour(COLOR_WINDOWTEXT), dark::get_dark_system_colour(COLOR_WINDOW));
            set_use_smooth_scroll(config::use_smooth_scrolling);
            m_use_smooth_scroll_change_token = config::use_smooth_scrolling.on_change(
                [this](bool new_value, auto) { set_use_smooth_scroll(new_value); });

            this->create(
                parent, {p_position.x, p_position.y, static_cast<int>(p_position.cx), static_cast<int>(p_position.cy)});
        }

        return get_wnd();
    }
    void destroy_window() override
    {
        m_use_smooth_scroll_change_token.reset();
        destroy();
        m_window_host.release();
    }

    bool is_available(const uie::window_host_ptr&) const override { return true; }
    HWND get_wnd() const override { return ListView::get_wnd(); }

    const uie::window_host_ptr& get_host() const { return m_window_host; }

protected:
    const char* get_drag_unit_plural() const override { return "tracks"; }
    const char* get_drag_unit_singular() const override { return "track"; }
    bool should_show_drag_text(size_t selection_count) override { return true; }
    void render_get_colour_data(ColourData& p_out) override
    {
        colours::helper p_helper(ColoursClientId);
        p_out.m_themed = p_helper.get_themed();
        p_out.m_use_custom_active_item_frame = p_helper.get_bool(colours::bool_use_custom_active_item_frame);
        p_out.m_text = p_helper.get_colour(colours::colour_text);
        p_out.m_selection_text = p_helper.get_colour(colours::colour_selection_text);
        p_out.m_background = p_helper.get_colour(colours::colour_background);
        p_out.m_selection_background = p_helper.get_colour(colours::colour_selection_background);
        p_out.m_inactive_selection_text = p_helper.get_colour(colours::colour_inactive_selection_text);
        p_out.m_inactive_selection_background = p_helper.get_colour(colours::colour_inactive_selection_background);
        p_out.m_active_item_frame = p_helper.get_colour(colours::colour_active_item_frame);
        if (!p_out.m_themed || !get_group_text_colour_default(p_out.m_group_text))
            p_out.m_group_text = p_out.m_text;
        p_out.m_group_background = p_out.m_background;
    }

    void recreate_items_text_format(size_t layout_cache_size = 32)
    {
        const auto font_api = fb2k::std_api_get<fonts::manager_v3>();
        const auto items_font = font_api->get_font(ItemsFontId);
        const auto items_text_format = fonts::get_text_format(items_font, layout_cache_size);
        const auto items_log_font = items_font->log_font();
        set_font(items_text_format, items_log_font);
    }

    void recreate_header_text_format()
    {
        if (HeaderFontId == GUID{})
            return;

        const auto font_api = fb2k::std_api_get<fonts::manager_v3>();
        const auto header_font = font_api->get_font(HeaderFontId);
        set_header_font(fonts::get_text_format(header_font), header_font->log_font());
    }

    void recreate_group_text_format(size_t layout_cache_size = 32)
    {
        if (GroupFontId == GUID{})
            return;

        const auto font_api = fb2k::std_api_get<fonts::manager_v3>();
        const auto group_font = font_api->get_font(GroupFontId);
        set_group_font(fonts::get_text_format(group_font, layout_cache_size));
    }

private:
    uie::window_host_ptr m_window_host;
    mmh::EventToken::Ptr m_use_smooth_scroll_change_token;
};

} // namespace cui::utils
