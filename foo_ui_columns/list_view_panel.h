#pragma once

#include "dark_mode.h"

template <typename t_appearance_client, typename t_window = uie::window>
class ListViewPanelBase
    : public uih::ListView
    , public t_window {
public:
    explicit ListViewPanelBase(
        std::unique_ptr<uih::lv::RendererBase> renderer = std::make_unique<uih::lv::DefaultRenderer>())
        : ListView(std::move(renderer))
    {
        set_dark_edit_colours(
            cui::dark::get_dark_system_colour(COLOR_WINDOWTEXT), cui::dark::get_dark_system_colour(COLOR_WINDOW));
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
            this->create(
                parent, {p_position.x, p_position.y, static_cast<int>(p_position.cx), static_cast<int>(p_position.cy)});
        }

        return get_wnd();
    }
    void destroy_window() override
    {
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
        cui::colours::helper p_helper(t_appearance_client::g_guid);
        p_out.m_themed = p_helper.get_themed();
        p_out.m_use_custom_active_item_frame = p_helper.get_bool(cui::colours::bool_use_custom_active_item_frame);
        p_out.m_text = p_helper.get_colour(cui::colours::colour_text);
        p_out.m_selection_text = p_helper.get_colour(cui::colours::colour_selection_text);
        p_out.m_background = p_helper.get_colour(cui::colours::colour_background);
        p_out.m_selection_background = p_helper.get_colour(cui::colours::colour_selection_background);
        p_out.m_inactive_selection_text = p_helper.get_colour(cui::colours::colour_inactive_selection_text);
        p_out.m_inactive_selection_background = p_helper.get_colour(cui::colours::colour_inactive_selection_background);
        p_out.m_active_item_frame = p_helper.get_colour(cui::colours::colour_active_item_frame);
        if (!p_out.m_themed || !get_group_text_colour_default(p_out.m_group_text))
            p_out.m_group_text = p_out.m_text;
        p_out.m_group_background = p_out.m_background;
    }

private:
    uie::window_host_ptr m_window_host;
};
