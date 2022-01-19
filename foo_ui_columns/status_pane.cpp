#include "stdafx.h"

#include "main_window.h"
#include "status_pane.h"

#include "dark_mode.h"

namespace cui::status_pane {

// {522E01C6-EA7C-49f2-AE5E-702B8C6B4B24}
const GUID StatusPane::g_guid_font = {0x522e01c6, 0xea7c, 0x49f2, {0xae, 0x5e, 0x70, 0x2b, 0x8c, 0x6b, 0x4b, 0x24}};

class StatusPaneFontClient : public fonts::client {
public:
    const GUID& get_client_guid() const override { return StatusPane::g_guid_font; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Status pane"; }

    fonts::font_type_t get_default_font_type() const override { return fonts::font_type_labels; }

    void on_font_changed() const override { g_status_pane.on_font_changed(); }
};

StatusPaneFontClient::factory<StatusPaneFontClient> g_font_client_status_pane;

void StatusPane::on_font_changed()
{
    m_font.reset(fonts::helper(g_guid_font).get_font());
    main_window.resize_child_windows();
}

void StatusPane::render_background(HDC dc, const RECT& rc)
{
    const auto is_dark = dark::is_dark_mode_enabled();
    const auto fill_brush = dark::get_system_colour_brush(COLOR_BTNFACE, is_dark);
    COLORREF cr2 = dark::get_system_colour(COLOR_3DDKSHADOW, is_dark);

    FillRect(dc, &rc, fill_brush.get());

    if (m_theme) {
        COLORREF cr_back = cr2;
        Gdiplus::Color cr_end(0, LOBYTE(LOWORD(cr_back)), HIBYTE(LOWORD(cr_back)), LOBYTE(HIWORD(cr_back)));
        Gdiplus::Color cr_start(33, LOBYTE(LOWORD(cr_back)), HIBYTE(LOWORD(cr_back)), LOBYTE(HIWORD(cr_back)));
        Gdiplus::Rect rect(rc.left, rc.top, RECT_CX(rc), uih::scale_dpi_value(2));
        Gdiplus::LinearGradientBrush lgb(rect, cr_start, cr_end, Gdiplus::LinearGradientModeVertical);
        Gdiplus::Graphics(dc).FillRectangle(&lgb, rect);
    } else {
        RECT rcl = {0, 0, rc.right, uih::scale_dpi_value(1)};
        FillRect(dc, &rcl, dark::get_system_colour_brush(COLOR_3DLIGHT, is_dark).get());
    }
}

void StatusPane::update_playback_status_text()
{
    static_api_ptr_t<playback_control> api;
    if (api->is_playing()) {
        m_track_label = api->is_paused() ? "Paused:" : "Playing:";
    } else {
        m_track_label = "";
    }
}

void StatusPane::get_length_data(bool& p_selection, t_size& p_count, pfc::string_base& p_out)
{
    metadb_handle_list_t<pfc::alloc_fast_aggressive> sels;
    double length = 0;

    static_api_ptr_t<playlist_manager> playlist_api;
    static_api_ptr_t<metadb> metadb_api;

    unsigned count = playlist_api->activeplaylist_get_selection_count(pfc_infinite);
    bool b_selection = count > 0;
    if (count == 0)
        count = playlist_api->activeplaylist_get_item_count();

    sels.prealloc(count);

    if (b_selection)
        playlist_api->activeplaylist_get_selected_items(sels);
    else
        playlist_api->activeplaylist_get_all_items(sels);

    length = sels.calc_total_duration();

    p_out = pfc::format_time_ex(length, 0);
    p_count = count;
    p_selection = b_selection;
}

StatusPane g_status_pane;

} // namespace cui::status_pane
