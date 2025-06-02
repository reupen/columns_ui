#include "pch.h"

#include "main_window.h"
#include "status_pane.h"

#include "dark_mode.h"
#include "font_utils.h"
#include "menu_items.h"
#include "metadb_helpers.h"

namespace cui::status_pane {

namespace {

const auto* default_status_pane_script
    = "// This is the default script for the content of the main status pane section during playback.\r\n"
      "\r\n"
      "%artist% – %title%\r\n"
      "$crlf()\r\n"
      "%codec% | %bitrate% kbps | %samplerate% Hz | $caps(%channels%) | %playback_time%[ / %length%]";

}

fbh::ConfigString status_pane_script(GUID{0x952b7029, 0x8dd5, 0x46e8, {0xbc, 0xbe, 0x67, 0xf, 0xd5, 0x2, 0x6f, 0x3f}},
    default_status_pane_script, [](auto&&) { g_status_pane.refresh_playing_text_section(); });

ConfigMenuItem double_click_action(GUID{0x55c049ba, 0x9f46, 0x4986, {0xa4, 0xf5, 0xf8, 0x3d, 0x14, 0x49, 0x29, 0x9b}},
    cui::main_menu::commands::activate_now_playing_id);

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
    recreate_font();
    main_window.resize_child_windows();
}

void StatusPane::recreate_font()
{
    m_text_format.reset();

    const auto font = fonts::get_font(g_guid_font);
    const auto text_format = font->create_wil_text_format();

    if (!(m_direct_write_context && text_format))
        return;

    try {
        m_text_format = fonts::get_text_format(m_direct_write_context, font);
    }
    CATCH_LOG()

    m_font_height = m_text_format ? m_text_format->get_minimum_height() : 0;
}

void StatusPane::render_background(HDC dc, const RECT& rc)
{
    const auto is_dark = colours::is_dark_mode_active();
    const auto fill_colour = get_colour(dark::ColourID::StatusPaneBackground, is_dark);
    const auto fill_brush = get_colour_brush(dark::ColourID::StatusPaneBackground, is_dark);
    const auto top_line_colour = get_colour(dark::ColourID::StatusPaneTopLine, is_dark);

    FillRect(dc, &rc, fill_brush.get());

    if (top_line_colour != fill_colour) {
        const Gdiplus::Color gradient_start(
            20, GetRValue(top_line_colour), GetGValue(top_line_colour), GetBValue(top_line_colour));
        const Gdiplus::Color gradient_end(
            0, GetRValue(top_line_colour), GetGValue(top_line_colour), GetBValue(top_line_colour));
        const Gdiplus::Rect rect(rc.left, rc.top, wil::rect_width(rc), uih::scale_dpi_value(2));
        Gdiplus::LinearGradientBrush lgb(rect, gradient_start, gradient_end, Gdiplus::LinearGradientModeVertical);
        Gdiplus::Graphics(dc).FillRectangle(&lgb, rect);
    }
}

void StatusPane::update_playback_status_text()
{
    const auto api = playback_control::get();
    if (api->is_playing()) {
        m_track_label = api->is_paused() ? "Paused:" : "Playing:";
    } else {
        m_track_label = "";
    }
}

void StatusPane::update_playing_text()
{
    metadb_handle_ptr track;
    const auto play_api = play_control::get();
    play_api->get_now_playing(track);
    if (track.is_valid()) {
        service_ptr_t<titleformat_object> to_status;
        titleformat_compiler::get()->compile_safe(to_status, status_pane_script.get());
        StatusPaneTitleformatHook tf_hook;
        play_api->playback_format_title_ex(
            track, &tf_hook, playing1, to_status, nullptr, play_control::display_level_all);

        track.release();
    } else {
        playing1.reset();
    }
}

void StatusPane::get_length_data(bool& p_selection, size_t& p_count, pfc::string_base& p_out)
{
    metadb_handle_list_t<pfc::alloc_fast_aggressive> sels;
    double length = 0;

    const auto playlist_api = playlist_manager::get();
    const auto metadb_api = metadb::get();

    auto count = playlist_api->activeplaylist_get_selection_count(pfc_infinite);
    bool b_selection = count > 0;
    if (count == 0)
        count = playlist_api->activeplaylist_get_item_count();

    sels.prealloc(count);

    if (b_selection)
        playlist_api->activeplaylist_get_selected_items(sels);
    else
        playlist_api->activeplaylist_get_all_items(sels);

    length = helpers::calculate_tracks_total_length(sels);

    p_out = pfc::format_time_ex(length, 0);
    p_count = count;
    p_selection = b_selection;
}

StatusPane g_status_pane;

} // namespace cui::status_pane
