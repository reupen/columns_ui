#include "pch.h"

#include "artwork.h"
#include "config.h"
#include "wic.h"

namespace cui::artwork_panel {

namespace {

wil::unique_hbitmap scale_image(Gdiplus::Bitmap& bitmap, const int client_width, const int client_height,
    const bool preserve_aspect_ratio, const COLORREF background_colour)
{
    if (bitmap.GetWidth() < 2 || bitmap.GetHeight() < 2)
        return {};

    const double image_aspect_ratio
        = gsl::narrow_cast<double>(bitmap.GetWidth()) / gsl::narrow_cast<double>(bitmap.GetHeight());
    const double client_aspect_ratio = gsl::narrow_cast<double>(client_width) / gsl::narrow_cast<double>(client_height);

    int scaled_width = client_width;
    int scaled_height = client_height;

    if (preserve_aspect_ratio) {
        if (client_aspect_ratio < image_aspect_ratio)
            scaled_height
                = gsl::narrow_cast<unsigned>(floor(gsl::narrow_cast<double>(client_width) / image_aspect_ratio));
        else if (client_aspect_ratio > image_aspect_ratio)
            scaled_width
                = gsl::narrow_cast<unsigned>(floor(gsl::narrow_cast<double>(client_height) * image_aspect_ratio));
    }

    if (((client_height - scaled_height) % 2) != 0)
        ++scaled_height;

    if (((client_width - scaled_width) % 2) != 0)
        ++scaled_width;

    const auto dc = wil::GetDC(nullptr);
    const wil::unique_hdc memory_dc(CreateCompatibleDC(dc.get()));

    wil::unique_hbitmap scaled_bitmap(CreateCompatibleBitmap(dc.get(), client_width, client_height));

    auto _ = wil::SelectObject(memory_dc.get(), scaled_bitmap.get());

    Gdiplus::Graphics graphics(memory_dc.get());
    graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

    Gdiplus::SolidBrush brush(
        Gdiplus::Color(GetRValue(background_colour), GetGValue(background_colour), GetBValue(background_colour)));
    graphics.FillRectangle(&brush, 0, 0, client_width, client_height);

    Gdiplus::Rect dest_rect(
        (client_width - scaled_width) / 2, (client_height - scaled_height) / 2, scaled_width, scaled_height);
    graphics.SetClip(dest_rect);
    Gdiplus::ImageAttributes image_attributes;
    image_attributes.SetWrapMode(Gdiplus::WrapModeTileFlipXY);

    graphics.DrawImage(
        &bitmap, dest_rect, 0, 0, bitmap.GetWidth(), bitmap.GetHeight(), Gdiplus::UnitPixel, &image_attributes);

    return scaled_bitmap;
}

} // namespace

// {005C7B29-3915-4b83-A283-C01A4EDC4F3A}
const GUID g_guid_track_mode = {0x5c7b29, 0x3915, 0x4b83, {0xa2, 0x83, 0xc0, 0x1a, 0x4e, 0xdc, 0x4f, 0x3a}};

// {A35E8697-0B8A-4e6f-9DBE-39EC4626524D}
const GUID g_guid_preserve_aspect_ratio = {0xa35e8697, 0xb8a, 0x4e6f, {0x9d, 0xbe, 0x39, 0xec, 0x46, 0x26, 0x52, 0x4d}};

// {F5C8CE6B-5D68-4ce2-8B9F-874D8EDB03B3}
const GUID g_guid_edge_style = {0xf5c8ce6b, 0x5d68, 0x4ce2, {0x8b, 0x9f, 0x87, 0x4d, 0x8e, 0xdb, 0x3, 0xb3}};

enum TrackingMode {
    track_auto_playlist_playing,
    track_playlist,
    track_playing,
    track_auto_selection_playing,
    track_selection,
};

bool g_track_mode_includes_now_playing(size_t mode)
{
    return mode == track_auto_playlist_playing || mode == track_auto_selection_playing || mode == track_playing;
}

bool g_track_mode_includes_playlist(size_t mode)
{
    return mode == track_auto_playlist_playing || mode == track_playlist;
}

bool g_track_mode_includes_auto(size_t mode)
{
    return mode == track_auto_playlist_playing || mode == track_auto_selection_playing;
}

bool g_track_mode_includes_selection(size_t mode)
{
    return mode == track_auto_selection_playing || mode == track_selection;
}

cfg_uint cfg_track_mode(g_guid_track_mode, track_auto_playlist_playing);
cfg_bool cfg_preserve_aspect_ratio(g_guid_preserve_aspect_ratio, true);
cfg_uint cfg_edge_style(g_guid_edge_style, 0);

fbh::ConfigInt32 click_action({0x01b0f35f, 0xcdca, 0x49ba, {0xac, 0x39, 0x0a, 0x91, 0x81, 0xa1, 0x6f, 0xc1}},
    WI_EnumValue(ClickAction::show_next_artwork_type));

// {E32DCBA9-A2BF-4901-AB43-228628071410}
static const GUID g_guid_colour_client = {0xe32dcba9, 0xa2bf, 0x4901, {0xab, 0x43, 0x22, 0x86, 0x28, 0x7, 0x14, 0x10}};

const std::vector<GUID> g_artwork_types{
    album_art_ids::cover_front, album_art_ids::cover_back, album_art_ids::disc, album_art_ids::artist};

void ArtworkPanel::get_config(stream_writer* p_writer, abort_callback& p_abort) const
{
    p_writer->write_lendian_t(m_track_mode, p_abort);
    p_writer->write_lendian_t(static_cast<uint32_t>(current_stream_version), p_abort);
    p_writer->write_lendian_t(m_preserve_aspect_ratio, p_abort);
    p_writer->write_lendian_t(m_artwork_type_locked, p_abort);
    p_writer->write_lendian_t(gsl::narrow<uint32_t>(m_selected_artwork_type_index), p_abort);
}

void ArtworkPanel::get_menu_items(ui_extension::menu_hook_t& p_hook)
{
    if (is_core_image_viewer_available()) {
        p_hook.add_node(uie::menu_node_ptr(new uie::simple_command_menu_node("Open in pop-up viewer",
            "Opens the image in the foobar2000 picture viewer.", 0,
            [this, self = ptr{this}] { open_core_image_viewer(); })));
    }

    p_hook.add_node(uie::menu_node_ptr(new uie::simple_command_menu_node(
        "Reload artwork", "Reloads the currently displayed artwork.", 0, [this, self = ptr{this}] {
            flush_image();
            force_reload_artwork();
        })));
    p_hook.add_node(uie::menu_node_ptr(new uie::menu_node_separator_t()));
    p_hook.add_node(uie::menu_node_ptr(new MenuNodeTypePopup(this)));
    p_hook.add_node(uie::menu_node_ptr(new MenuNodeSourcePopup(this)));
    p_hook.add_node(uie::menu_node_ptr(new MenuNodePreserveAspectRatio(this)));
    p_hook.add_node(uie::menu_node_ptr(new MenuNodeLockType(this)));
    p_hook.add_node(uie::menu_node_ptr(new MenuNodeOptions()));
}

void ArtworkPanel::request_artwork(const metadb_handle_ptr& track, bool is_from_playback)
{
    const auto handle_artwork_read
        = [self{service_ptr_t{this}}](bool artwork_changed) { self->on_artwork_loaded(artwork_changed); };

    m_artwork_reader->request(track, std::move(handle_artwork_read), is_from_playback);
}

ArtworkPanel::ArtworkPanel() : m_track_mode(cfg_track_mode), m_preserve_aspect_ratio(cfg_preserve_aspect_ratio) {}

uie::container_window_v3_config ArtworkPanel::get_window_config()
{
    uie::container_window_v3_config config(L"columns_ui_artwork_view_VaODDnIsit0", false);

    if (cfg_edge_style == 1)
        config.extended_window_styles |= WS_EX_CLIENTEDGE;
    else if (cfg_edge_style == 2)
        config.extended_window_styles |= WS_EX_STATICEDGE;

    config.class_cursor = IDC_HAND;

    return config;
}

void ArtworkPanel::g_on_edge_style_change()
{
    for (auto& window : g_windows) {
        HWND wnd = window->get_wnd();
        if (wnd) {
            long flags = 0;
            if (cfg_edge_style == 1)
                flags |= WS_EX_CLIENTEDGE;
            if (cfg_edge_style == 2)
                flags |= WS_EX_STATICEDGE;
            SetWindowLongPtr(wnd, GWL_EXSTYLE, flags);
            SetWindowPos(wnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
        }
    }
}

/**
 * Note: This is not called when there is no artwork to display.
 *
 * Additionally, it isn't called when foobar2000 thinks the track belongs to
 * the same album as the previous track (determined using metadata, not
 * artwork equivalence).
 */
void ArtworkPanel::on_album_art(album_art_data::ptr data) noexcept
{
    if (!m_artwork_reader || !m_artwork_reader->is_ready()) {
        m_dynamic_artwork_pending = true;
        return;
    }

    if (m_selected_artwork_type_index == 0) {
        m_artwork_type_override_index.reset();
        refresh_image();
    }
}

const GUID& ArtworkPanel::get_extension_guid() const
{
    // {DEEAD6EC-F0B9-4919-B16D-280AEDDE7343}
    static const GUID guid = {0xdeead6ec, 0xf0b9, 0x4919, {0xb1, 0x6d, 0x28, 0xa, 0xed, 0xde, 0x73, 0x43}};
    return guid;
}

void ArtworkPanel::get_name(pfc::string_base& out) const
{
    out = "Artwork view";
}

void ArtworkPanel::get_category(pfc::string_base& out) const
{
    out = "Panels";
}

unsigned ArtworkPanel::get_type() const
{
    return uie::type_panel;
}

LRESULT ArtworkPanel::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        m_gdiplus_initialised = (Gdiplus::Ok == GdiplusStartup(&m_gdiplus_instance, &gdiplusStartupInput, nullptr));
        m_artwork_reader = std::make_shared<ArtworkReaderManager>();
        now_playing_album_art_notify_manager::get()->add(this);
        m_artwork_reader->set_types(g_artwork_types);
        play_callback_manager::get()->register_callback(
            this, flag_on_playback_new_track | flag_on_playback_stop | flag_on_playback_edited, false);
        playlist_manager_v3::get()->register_callback(this, playlist_callback_flags);
        g_ui_selection_manager_register_callback_no_now_playing_fallback(this);
        force_reload_artwork();
        g_windows.push_back(this);
    } break;
    case WM_DESTROY:
        std::erase(g_windows, this);
        ui_selection_manager::get()->unregister_callback(this);
        playlist_manager_v3::get()->unregister_callback(this);
        play_callback_manager::get()->unregister_callback(this);
        now_playing_album_art_notify_manager::get()->remove(this);
        m_selection_handles.remove_all();
        m_artwork_decoder.shut_down();
        m_bitmap.reset();
        if (m_gdiplus_initialised)
            Gdiplus::GdiplusShutdown(m_gdiplus_instance);
        m_gdiplus_initialised = false;
        if (m_artwork_reader)
            m_artwork_reader->deinitialise();
        m_artwork_reader.reset();
        break;
    case WM_ERASEBKGND:
        return FALSE;
    case WM_WINDOWPOSCHANGED: {
        auto lpwp = (LPWINDOWPOS)lp;
        if (!(lpwp->flags & SWP_NOSIZE)) {
            flush_cached_bitmap();
            invalidate_window();
        }
    } break;
    case WM_LBUTTONDOWN: {
        switch (static_cast<ClickAction>(click_action.get())) {
        case ClickAction::open_image_viewer:
            open_core_image_viewer();
            break;
        case ClickAction::show_next_artwork_type:
            show_next_artwork_type();
            break;
        }
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(wnd, &ps);
        RECT rc;
        GetClientRect(wnd, &rc);
        if (m_gdiplus_initialised) {
            if (!m_bitmap)
                refresh_cached_bitmap();

            if (m_bitmap) {
                HDC dcc = CreateCompatibleDC(dc);

                HBITMAP bm_old = SelectBitmap(dcc, m_bitmap.get());
                BitBlt(dc, 0, 0, wil::rect_width(rc), wil::rect_height(rc), dcc, 0, 0, SRCCOPY);
                SelectBitmap(dcc, bm_old);
                DeleteDC(dcc);
            } else {
                auto background_colour = colours::helper(g_guid_colour_client).get_colour(colours::colour_background);
                const wil::unique_hbrush background_brush(CreateSolidBrush(background_colour));
                FillRect(ps.hdc, &rc, background_brush.get());
            }
        }
        EndPaint(wnd, &ps);
        return 0;
    }
    }
    return DefWindowProc(wnd, msg, wp, lp);
}

bool g_check_process_on_selection_changed()
{
    HWND wnd_focus = GetFocus();
    if (wnd_focus == nullptr)
        return false;

    DWORD processid = NULL;
    GetWindowThreadProcessId(wnd_focus, &processid);
    return processid == GetCurrentProcessId();
}

void ArtworkPanel::on_selection_changed(const pfc::list_base_const_t<metadb_handle_ptr>& p_selection) noexcept
{
    if (g_check_process_on_selection_changed()) {
        if (g_ui_selection_manager_is_now_playing_fallback())
            m_selection_handles.remove_all();
        else
            m_selection_handles = p_selection;

        if (g_track_mode_includes_selection(m_track_mode)
            && (!g_track_mode_includes_auto(m_track_mode) || !play_control::get()->is_playing())) {
            if (m_selection_handles.get_count()) {
                request_artwork(m_selection_handles[0]);
            } else {
                flush_image();
                if (m_artwork_reader)
                    m_artwork_reader->reset();
            }
        }
    }
}

void ArtworkPanel::on_playback_stop(play_control::t_stop_reason p_reason) noexcept
{
    m_dynamic_artwork_pending = false;

    if (g_track_mode_includes_now_playing(m_track_mode) && p_reason != play_control::stop_reason_starting_another
        && p_reason != play_control::stop_reason_shutting_down) {
        bool b_set = false;
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        if (m_track_mode == track_auto_playlist_playing) {
            playlist_manager_v3::get()->activeplaylist_get_selected_items(handles);
        } else if (m_track_mode == track_auto_selection_playing) {
            handles = m_selection_handles;
        }

        if (handles.get_count()) {
            request_artwork(handles[0]);
            b_set = true;
        }

        if (!b_set) {
            flush_image();
            if (m_artwork_reader)
                m_artwork_reader->reset();
        }
    }
}

void ArtworkPanel::on_playback_new_track(metadb_handle_ptr p_track) noexcept
{
    m_dynamic_artwork_pending = false;
    if (g_track_mode_includes_now_playing(m_track_mode) && m_artwork_reader) {
        const auto data = now_playing_album_art_notify_manager::get()->current();

        request_artwork(p_track, true);
    }
}

void ArtworkPanel::force_reload_artwork()
{
    auto is_from_playback = false;
    metadb_handle_ptr handle;
    if (g_track_mode_includes_now_playing(m_track_mode) && play_control::get()->is_playing()) {
        play_control::get()->get_now_playing(handle);
        is_from_playback = true;
        m_dynamic_artwork_pending = now_playing_album_art_notify_manager::get()->current().is_valid();
    } else if (g_track_mode_includes_playlist(m_track_mode)) {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        playlist_manager_v3::get()->activeplaylist_get_selected_items(handles);
        if (handles.get_count())
            handle = handles[0];
    } else if (g_track_mode_includes_selection(m_track_mode)) {
        if (m_selection_handles.get_count())
            handle = m_selection_handles[0];
    }

    if (handle.is_valid()) {
        request_artwork(handle, is_from_playback);
    } else {
        flush_image();
        if (m_artwork_reader)
            m_artwork_reader->reset();
    }
}

bool ArtworkPanel::is_core_image_viewer_available() const
{
    if (fb2k::imageViewer::ptr api; !fb2k::imageViewer::tryGet(api)) {
        return false;
    }

    const auto artwork_type_id = g_artwork_types[get_displayed_artwork_type_index()];
    const album_art_data_ptr data = m_artwork_reader->get_image(artwork_type_id);

    return data.is_valid();
}

void ArtworkPanel::open_core_image_viewer() const
{
    const auto artwork_type_id = g_artwork_types[get_displayed_artwork_type_index()];
    const album_art_data_ptr data = m_artwork_reader->get_image(artwork_type_id);

    if (!data.is_valid())
        return;

    if (fb2k::imageViewer::ptr api; fb2k::imageViewer::tryGet(api)) {
        api->show(GetParent(get_wnd()), data);
    }
}

void ArtworkPanel::show_next_artwork_type()
{
    if (!m_artwork_reader || !m_artwork_reader->is_ready())
        return;

    const auto start_artwork_type_index = get_displayed_artwork_type_index();
    auto artwork_type_index = start_artwork_type_index;
    const size_t count = g_artwork_types.size();

    for (size_t i = 0; i + 1 < count; i++) {
        artwork_type_index = (artwork_type_index + 1) % count;
        const auto artwork_type_id = g_artwork_types[artwork_type_index];
        const auto data = m_artwork_reader->get_image(artwork_type_id);

        if (data.is_valid()) {
            m_artwork_type_override_index.reset();
            m_selected_artwork_type_index = artwork_type_index;
            refresh_image();
            break;
        }
    }
}

void ArtworkPanel::set_artwork_type_index(size_t index)
{
    m_selected_artwork_type_index = index;
    m_artwork_type_override_index.reset();

    if (!m_artwork_reader || !m_artwork_reader->is_ready())
        return;

    const auto artwork_type_id = g_artwork_types[m_selected_artwork_type_index];
    const auto data = m_artwork_reader->get_image(artwork_type_id);

    if (!data.is_valid()) {
        show_stub_image();
        return;
    }

    refresh_image();
}

void ArtworkPanel::on_playlist_switch() noexcept
{
    if (g_track_mode_includes_playlist(m_track_mode)
        && (!g_track_mode_includes_auto(m_track_mode) || !play_control::get()->is_playing())) {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        playlist_manager_v3::get()->activeplaylist_get_selected_items(handles);
        if (handles.get_count()) {
            request_artwork(handles[0]);
        } else {
            flush_image();
            if (m_artwork_reader)
                m_artwork_reader->reset();
        }
    }
}

void ArtworkPanel::on_items_selection_change(const bit_array& p_affected, const bit_array& p_state) noexcept
{
    if (g_track_mode_includes_playlist(m_track_mode)
        && (!g_track_mode_includes_auto(m_track_mode) || !play_control::get()->is_playing())) {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        playlist_manager_v3::get()->activeplaylist_get_selected_items(handles);
        if (handles.get_count()) {
            request_artwork(handles[0]);
        } else {
            flush_image();
            if (m_artwork_reader)
                m_artwork_reader->reset();
        }
    }
}

void ArtworkPanel::on_artwork_loaded(bool artwork_changed)
{
    if (!get_wnd())
        return;

    const auto is_dynamic_artwork_pending = m_dynamic_artwork_pending && m_selected_artwork_type_index == 0;

    if (m_artwork_decoder.has_image() && !is_dynamic_artwork_pending && !artwork_changed)
        return;

    m_dynamic_artwork_pending = false;
    m_artwork_type_override_index = m_selected_artwork_type_index;

    bool b_found = false;
    size_t count = g_artwork_types.size();

    if (m_artwork_type_locked)
        count = std::min(size_t{1}, count);

    for (size_t i = 0; i < count; i++) {
        const auto artwork_type_index = get_displayed_artwork_type_index();
        const auto artwork_type_id = g_artwork_types[artwork_type_index];
        const auto data = m_artwork_reader->get_image(artwork_type_id);

        if (data.is_valid()) {
            refresh_image();
            b_found = true;
            break;
        }

        m_artwork_type_override_index = (*m_artwork_type_override_index + 1) % g_artwork_types.size();
    }

    if (!b_found) {
        m_artwork_type_override_index.reset();
        show_stub_image();
    }
}

void ArtworkPanel::show_stub_image()
{
    flush_image(false);

    if (!m_artwork_reader || !m_artwork_reader->is_ready())
        return;

    const auto artwork_type_id = g_artwork_types[get_displayed_artwork_type_index()];
    const album_art_data_ptr data = m_artwork_reader->get_stub_image(artwork_type_id);

    m_artwork_decoder.decode(data, [this, self{ptr{this}}] { invalidate_window(); });
}

void ArtworkPanel::refresh_image()
{
    TRACK_CALL_TEXT("cui::ArtworkPanel::refresh_image");

    flush_image(false);

    if (!m_artwork_reader || !m_artwork_reader->is_ready())
        return;

    const auto artwork_type_index = get_displayed_artwork_type_index();
    const auto artwork_type_id = g_artwork_types[artwork_type_index];
    const auto data = m_artwork_reader->get_image(artwork_type_id);

    if (data.is_empty())
        return;

    m_artwork_decoder.decode(data, [this, self{ptr{this}}] { invalidate_window(); });
}

void ArtworkPanel::flush_cached_bitmap()
{
    m_bitmap.reset();
}

void ArtworkPanel::flush_image(bool invalidate)
{
    m_artwork_decoder.reset();
    flush_cached_bitmap();
    if (invalidate)
        invalidate_window();
}

void ArtworkPanel::invalidate_window() const
{
    RedrawWindow(get_wnd(), nullptr, nullptr, RDW_INVALIDATE);
}

size_t ArtworkPanel::get_displayed_artwork_type_index() const
{
    return m_artwork_type_override_index.value_or(m_selected_artwork_type_index);
}

void ArtworkPanel::refresh_cached_bitmap()
{
    RECT rc{};
    GetClientRect(get_wnd(), &rc);
    const auto width = static_cast<int>(wil::rect_width(rc));
    const auto height = static_cast<int>(wil::rect_height(rc));

    if (width <= 0 || height <= 0 || !m_artwork_decoder.has_image()) {
        m_bitmap.reset();
        return;
    }

    const auto background_colour = colours::helper(g_guid_colour_client).get_colour(colours::colour_background);
    auto image = m_artwork_decoder.get_image();

    m_bitmap = scale_image(*image, width, height, background_colour, m_preserve_aspect_ratio);
}

void ArtworkPanel::g_on_colours_change()
{
    for (auto& window : g_windows) {
        if (!window->get_wnd())
            continue;

        window->flush_cached_bitmap();
        window->invalidate_window();
    }
}

void ArtworkPanel::s_on_dark_mode_status_change()
{
    for (auto&& window : g_windows) {
        if (!window->get_wnd())
            continue;

        if (window->m_artwork_reader)
            window->m_artwork_reader->reset();

        window->flush_image();
        window->force_reload_artwork();
    }
}

std::vector<ArtworkPanel*> ArtworkPanel::g_windows;

uie::window_factory<ArtworkPanel> g_artwork_panel;

class ArtworkColoursClient : public colours::client {
public:
    static const GUID g_guid;

    const GUID& get_client_guid() const override { return g_guid_colour_client; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Artwork view"; }

    uint32_t get_supported_colours() const override { return colours::colour_flag_background; } // bit-mask
    uint32_t get_supported_bools() const override { return colours::bool_flag_dark_mode_enabled; } // bit-mask
    bool get_themes_supported() const override { return false; }

    void on_colour_changed(uint32_t mask) const override { ArtworkPanel::g_on_colours_change(); }
    void on_bool_changed(uint32_t mask) const override
    {
        if ((mask & colours::bool_flag_dark_mode_enabled))
            ArtworkPanel::s_on_dark_mode_status_change();
    }
};

namespace {
colours::client::factory<ArtworkColoursClient> g_appearance_client_impl;
}

void ArtworkPanel::set_config(stream_reader* p_reader, size_t size, abort_callback& p_abort)
{
    if (size) {
        p_reader->read_lendian_t(m_track_mode, p_abort);
        uint32_t version = pfc_infinite;
        try {
            p_reader->read_lendian_t(version, p_abort);
        } catch (exception_io_data_truncation const&) {
        }

        if (version <= 3) {
            p_reader->read_lendian_t(m_preserve_aspect_ratio, p_abort);
            if (version >= 2) {
                p_reader->read_lendian_t(m_artwork_type_locked, p_abort);
                if (version >= 3) {
                    m_selected_artwork_type_index = p_reader->read_lendian_t<uint32_t>(p_abort);
                    if (m_selected_artwork_type_index >= g_artwork_types.size()) {
                        m_selected_artwork_type_index = 0;
                    }
                    m_artwork_type_override_index.reset();
                }
            }
        }
    }
}

ArtworkPanel::MenuNodeTrackMode::MenuNodeTrackMode(ArtworkPanel* p_wnd, uint32_t p_value)
    : p_this(p_wnd)
    , m_source(p_value)
{
}

void ArtworkPanel::MenuNodeTrackMode::execute()
{
    p_this->m_track_mode = m_source;
    cfg_track_mode = m_source;
    p_this->force_reload_artwork();
}

bool ArtworkPanel::MenuNodeTrackMode::get_description(pfc::string_base& p_out) const
{
    return false;
}

bool ArtworkPanel::MenuNodeTrackMode::get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = get_name(m_source);
    p_displayflags = (m_source == p_this->m_track_mode) ? state_radiochecked : 0;
    return true;
}

const char* ArtworkPanel::MenuNodeTrackMode::get_name(uint32_t source)
{
    if (source == track_playing)
        return "Playing item";
    if (source == track_playlist)
        return "Playlist selection";
    if (source == track_auto_selection_playing)
        return "Automatic (current selection/playing item)";
    if (source == track_selection)
        return "Current selection";
    return "Automatic (playlist selection/playing item)";
}

ArtworkPanel::MenuNodeArtworkType::MenuNodeArtworkType(ArtworkPanel* p_wnd, uint32_t p_value)
    : p_this(p_wnd)
    , m_type(p_value)
{
}

void ArtworkPanel::MenuNodeArtworkType::execute()
{
    p_this->set_artwork_type_index(m_type);
}

bool ArtworkPanel::MenuNodeArtworkType::get_description(pfc::string_base& p_out) const
{
    return false;
}

bool ArtworkPanel::MenuNodeArtworkType::get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = get_name(m_type);
    p_displayflags = m_type == p_this->get_displayed_artwork_type_index() ? state_radiochecked : 0;
    return true;
}

const char* ArtworkPanel::MenuNodeArtworkType::get_name(uint32_t source)
{
    if (source == 0)
        return "Front cover";
    if (source == 1)
        return "Back cover";
    if (source == 2)
        return "Disc cover";
    return "Artist picture";
}

ArtworkPanel::MenuNodeSourcePopup::MenuNodeSourcePopup(ArtworkPanel* p_wnd)
{
    m_items.emplace_back(new MenuNodeTrackMode(p_wnd, 3));
    m_items.emplace_back(new MenuNodeTrackMode(p_wnd, 0));
    m_items.emplace_back(new uie::menu_node_separator_t());
    m_items.emplace_back(new MenuNodeTrackMode(p_wnd, 2));
    m_items.emplace_back(new MenuNodeTrackMode(p_wnd, 4));
    m_items.emplace_back(new MenuNodeTrackMode(p_wnd, 1));
}

void ArtworkPanel::MenuNodeSourcePopup::get_child(size_t p_index, uie::menu_node_ptr& p_out) const
{
    p_out = m_items[p_index].get_ptr();
}

size_t ArtworkPanel::MenuNodeSourcePopup::get_children_count() const
{
    return m_items.size();
}

bool ArtworkPanel::MenuNodeSourcePopup::get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = "Displayed track";
    p_displayflags = 0;
    return true;
}

ArtworkPanel::MenuNodeTypePopup::MenuNodeTypePopup(ArtworkPanel* p_wnd)
{
    m_items.emplace_back(new MenuNodeArtworkType(p_wnd, 0));
    // m_items.add_item(new uie::menu_node_separator_t());
    m_items.emplace_back(new MenuNodeArtworkType(p_wnd, 1));
    m_items.emplace_back(new MenuNodeArtworkType(p_wnd, 2));
    m_items.emplace_back(new MenuNodeArtworkType(p_wnd, 3));
}

void ArtworkPanel::MenuNodeTypePopup::get_child(size_t p_index, uie::menu_node_ptr& p_out) const
{
    p_out = m_items[p_index].get_ptr();
}

size_t ArtworkPanel::MenuNodeTypePopup::get_children_count() const
{
    return m_items.size();
}

bool ArtworkPanel::MenuNodeTypePopup::get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = "Artwork type";
    p_displayflags = 0;
    return true;
}

ArtworkPanel::MenuNodePreserveAspectRatio::MenuNodePreserveAspectRatio(ArtworkPanel* p_wnd) : p_this(p_wnd) {}

void ArtworkPanel::MenuNodePreserveAspectRatio::execute()
{
    p_this->m_preserve_aspect_ratio = !p_this->m_preserve_aspect_ratio;
    cfg_preserve_aspect_ratio = p_this->m_preserve_aspect_ratio;
    p_this->flush_cached_bitmap();
    p_this->invalidate_window();
    p_this->refresh_cached_bitmap();
}

bool ArtworkPanel::MenuNodePreserveAspectRatio::get_description(pfc::string_base& p_out) const
{
    return false;
}

bool ArtworkPanel::MenuNodePreserveAspectRatio::get_display_data(
    pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = "Preserve aspect ratio";
    p_displayflags = (p_this->m_preserve_aspect_ratio) ? state_checked : 0;
    return true;
}

void ArtworkPanel::MenuNodeOptions::execute()
{
    prefs::page_main.get_static_instance().show_tab("Artwork");
}

bool ArtworkPanel::MenuNodeOptions::get_description(pfc::string_base& p_out) const
{
    return false;
}

bool ArtworkPanel::MenuNodeOptions::get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = "Options";
    p_displayflags = 0;
    return true;
}

ArtworkPanel::MenuNodeLockType::MenuNodeLockType(ArtworkPanel* p_wnd) : p_this(p_wnd) {}

void ArtworkPanel::MenuNodeLockType::execute()
{
    p_this->m_artwork_type_locked = !p_this->m_artwork_type_locked;
    if (p_this->m_artwork_type_locked) {
        p_this->m_selected_artwork_type_index = p_this->get_displayed_artwork_type_index();
        p_this->m_artwork_type_override_index.reset();
    }
}

bool ArtworkPanel::MenuNodeLockType::get_description(pfc::string_base& p_out) const
{
    return false;
}

bool ArtworkPanel::MenuNodeLockType::get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = "Lock artwork type";
    p_displayflags = (p_this->m_artwork_type_locked) ? state_checked : 0;
    return true;
}

} // namespace cui::artwork_panel
