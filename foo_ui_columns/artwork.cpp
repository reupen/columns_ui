#include "stdafx.h"

#include "artwork.h"
#include "config.h"
#include "wic.h"

namespace cui::artwork_panel {

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

bool g_track_mode_includes_now_playing(t_size mode)
{
    return mode == track_auto_playlist_playing || mode == track_auto_selection_playing || mode == track_playing;
}

bool g_track_mode_includes_playlist(t_size mode)
{
    return mode == track_auto_playlist_playing || mode == track_playlist;
}

bool g_track_mode_includes_auto(t_size mode)
{
    return mode == track_auto_playlist_playing || mode == track_auto_selection_playing;
}

bool g_track_mode_includes_selection(t_size mode)
{
    return mode == track_auto_selection_playing || mode == track_selection;
}

cfg_uint cfg_track_mode(g_guid_track_mode, track_auto_playlist_playing);
cfg_bool cfg_preserve_aspect_ratio(g_guid_preserve_aspect_ratio, true);
cfg_uint cfg_edge_style(g_guid_edge_style, 0);

// {E32DCBA9-A2BF-4901-AB43-228628071410}
static const GUID g_guid_colour_client = {0xe32dcba9, 0xa2bf, 0x4901, {0xab, 0x43, 0x22, 0x86, 0x28, 0x7, 0x14, 0x10}};

const std::vector<GUID> g_artwork_types{
    album_art_ids::cover_front, album_art_ids::cover_back, album_art_ids::disc, album_art_ids::artist};

void ArtworkPanel::get_config(stream_writer* p_writer, abort_callback& p_abort) const
{
    p_writer->write_lendian_t(m_track_mode, p_abort);
    p_writer->write_lendian_t((t_uint32)current_stream_version, p_abort);
    p_writer->write_lendian_t(m_preserve_aspect_ratio, p_abort);
    p_writer->write_lendian_t(m_lock_type, p_abort);
    p_writer->write_lendian_t(m_selected_artwork_type_index, p_abort);
}

void ArtworkPanel::get_menu_items(ui_extension::menu_hook_t& p_hook)
{
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

ArtworkPanel::ArtworkPanel() : m_track_mode(cfg_track_mode), m_preserve_aspect_ratio(cfg_preserve_aspect_ratio){};

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
void ArtworkPanel::on_album_art(album_art_data::ptr data)
{
    if (!m_artwork_loader || !m_artwork_loader->is_ready()) {
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
        m_gdiplus_initialised
            = (Gdiplus::Ok == Gdiplus::GdiplusStartup(&m_gdiplus_instance, &gdiplusStartupInput, nullptr));
        m_artwork_loader = std::make_shared<ArtworkReaderManager>();
        now_playing_album_art_notify_manager::get()->add(this);
        m_artwork_loader->set_types(g_artwork_types);
        static_api_ptr_t<play_callback_manager>()->register_callback(this,
            play_callback::flag_on_playback_new_track | play_callback::flag_on_playback_stop
                | play_callback::flag_on_playback_edited,
            false);
        static_api_ptr_t<playlist_manager_v3>()->register_callback(this, playlist_callback_flags);
        g_ui_selection_manager_register_callback_no_now_playing_fallback(this);
        force_reload_artwork();
        g_windows.push_back(this);
    } break;
    case WM_DESTROY:
        g_windows.erase(std::remove(g_windows.begin(), g_windows.end(), this), g_windows.end());
        static_api_ptr_t<ui_selection_manager>()->unregister_callback(this);
        static_api_ptr_t<playlist_manager_v3>()->unregister_callback(this);
        static_api_ptr_t<play_callback_manager>()->unregister_callback(this);
        now_playing_album_art_notify_manager::get()->remove(this);
        m_selection_handles.remove_all();
        m_image.reset();
        m_bitmap.reset();
        if (m_gdiplus_initialised)
            Gdiplus::GdiplusShutdown(m_gdiplus_instance);
        m_gdiplus_initialised = false;
        if (m_artwork_loader)
            m_artwork_loader->deinitialise();
        m_artwork_loader.reset();
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
        const auto start_artwork_type_index = get_displayed_artwork_type_index();
        auto artwork_type_index = start_artwork_type_index;
        const t_size count = g_artwork_types.size();
        bool artwork_found = false;

        for (t_size i = 0; i < count; i++) {
            artwork_type_index = (artwork_type_index + 1) % count;

            if (!refresh_image(artwork_type_index))
                continue;

            artwork_found = true;

            if (artwork_type_index != start_artwork_type_index) {
                m_artwork_type_override_index.reset();
                m_selected_artwork_type_index = artwork_type_index;
            }
            break;
        }

        if (!artwork_found) {
            m_artwork_type_override_index.reset();
            show_stub_image();
        }
    } break;
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
                BitBlt(dc, 0, 0, RECT_CX(rc), RECT_CY(rc), dcc, 0, 0, SRCCOPY);
                SelectBitmap(dcc, bm_old);
                DeleteDC(dcc);
            } else {
                auto background_colour
                    = cui::colours::helper(g_guid_colour_client).get_colour(cui::colours::colour_background);
                const wil::unique_hbrush background_brush(CreateSolidBrush(background_colour));
                FillRect(ps.hdc, &rc, background_brush.get());
            }
        }
        EndPaint(wnd, &ps);
    }
        return 0;
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

void ArtworkPanel::on_selection_changed(const pfc::list_base_const_t<metadb_handle_ptr>& p_selection)
{
    if (g_check_process_on_selection_changed()) {
        if (g_ui_selection_manager_is_now_playing_fallback())
            m_selection_handles.remove_all();
        else
            m_selection_handles = p_selection;

        if (g_track_mode_includes_selection(m_track_mode)
            && (!g_track_mode_includes_auto(m_track_mode) || !static_api_ptr_t<play_control>()->is_playing())) {
            if (m_selection_handles.get_count()) {
                m_artwork_loader->request(m_selection_handles[0], new service_impl_t<CompletionNotifyForwarder>(this));
            } else {
                flush_image();
                if (m_artwork_loader)
                    m_artwork_loader->reset();
            }
        }
    }
}

void ArtworkPanel::on_playback_stop(play_control::t_stop_reason p_reason)
{
    m_dynamic_artwork_pending = false;

    if (g_track_mode_includes_now_playing(m_track_mode) && p_reason != play_control::stop_reason_starting_another
        && p_reason != play_control::stop_reason_shutting_down) {
        bool b_set = false;
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        if (m_track_mode == track_auto_playlist_playing) {
            static_api_ptr_t<playlist_manager_v3>()->activeplaylist_get_selected_items(handles);
        } else if (m_track_mode == track_auto_selection_playing) {
            handles = m_selection_handles;
        }

        if (handles.get_count()) {
            m_artwork_loader->request(handles[0], new service_impl_t<CompletionNotifyForwarder>(this));
            b_set = true;
        }

        if (!b_set) {
            flush_image();
            if (m_artwork_loader)
                m_artwork_loader->reset();
        }
    }
}

void ArtworkPanel::on_playback_new_track(metadb_handle_ptr p_track)
{
    m_dynamic_artwork_pending = false;
    if (g_track_mode_includes_now_playing(m_track_mode) && m_artwork_loader) {
        const auto data = now_playing_album_art_notify_manager::get()->current();

        m_artwork_loader->request(p_track, new service_impl_t<CompletionNotifyForwarder>(this), true);
    }
}

void ArtworkPanel::force_reload_artwork()
{
    auto is_from_playback = false;
    metadb_handle_ptr handle;
    if (g_track_mode_includes_now_playing(m_track_mode) && static_api_ptr_t<play_control>()->is_playing()) {
        static_api_ptr_t<play_control>()->get_now_playing(handle);
        is_from_playback = true;
    } else if (g_track_mode_includes_playlist(m_track_mode)) {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        static_api_ptr_t<playlist_manager_v3>()->activeplaylist_get_selected_items(handles);
        if (handles.get_count())
            handle = handles[0];
    } else if (g_track_mode_includes_selection(m_track_mode)) {
        if (m_selection_handles.get_count())
            handle = m_selection_handles[0];
    }

    if (handle.is_valid()) {
        m_artwork_loader->request(handle, new service_impl_t<CompletionNotifyForwarder>(this), is_from_playback);
    } else {
        flush_image();
        if (m_artwork_loader)
            m_artwork_loader->reset();
    }
}

void ArtworkPanel::on_playlist_switch()
{
    if (g_track_mode_includes_playlist(m_track_mode)
        && (!g_track_mode_includes_auto(m_track_mode) || !static_api_ptr_t<play_control>()->is_playing())) {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        static_api_ptr_t<playlist_manager_v3>()->activeplaylist_get_selected_items(handles);
        if (handles.get_count()) {
            m_artwork_loader->request(handles[0], new service_impl_t<CompletionNotifyForwarder>(this));
        } else {
            flush_image();
            if (m_artwork_loader)
                m_artwork_loader->reset();
        }
    }
}

void ArtworkPanel::on_items_selection_change(const pfc::bit_array& p_affected, const pfc::bit_array& p_state)
{
    if (g_track_mode_includes_playlist(m_track_mode)
        && (!g_track_mode_includes_auto(m_track_mode) || !static_api_ptr_t<play_control>()->is_playing())) {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        static_api_ptr_t<playlist_manager_v3>()->activeplaylist_get_selected_items(handles);
        if (handles.get_count()) {
            m_artwork_loader->request(handles[0], new service_impl_t<CompletionNotifyForwarder>(this));
        } else {
            flush_image();
            if (m_artwork_loader)
                m_artwork_loader->reset();
        }
    }
}

void ArtworkPanel::on_completion(unsigned p_code)
{
    if (!get_wnd())
        return;

    const auto is_dynamic_artwork_pending = m_dynamic_artwork_pending && m_selected_artwork_type_index == 0;
    const auto artwork_changed = p_code == 1;

    if (m_image && !is_dynamic_artwork_pending && !artwork_changed)
        return;

    m_dynamic_artwork_pending = false;
    m_artwork_type_override_index = m_selected_artwork_type_index;

    bool b_found = false;
    t_size count = g_artwork_types.size();
    if (m_lock_type)
        count = min(1, count);
    for (t_size i = 0; i < count; i++) {
        if (refresh_image()) {
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
    // Needs to be delayed until after WIC calls are made (otherwise painting
    // may happen prematurely).
    auto _ = gsl::finally([this] { invalidate_window(); });

    if (!m_artwork_loader || !m_artwork_loader->is_ready())
        return;

    const auto artwork_type_id = g_artwork_types[get_displayed_artwork_type_index()];
    const album_art_data_ptr data = m_artwork_loader->get_stub_image(artwork_type_id);

    if (data.is_empty()) {
        return;
    }

    try {
        const auto bitmap_data = cui::wic::decode_image_data(data->get_ptr(), data->get_size());
        m_image = cui::gdip::create_bitmap_from_wic_data(bitmap_data);
    } catch (const std::exception& ex) {
        fbh::print_to_console(u8"Artwork panel – loading stub image failed: ", ex.what());
    }
}

bool ArtworkPanel::refresh_image(std::optional<size_t> artwork_type_index_override)
{
    TRACK_CALL_TEXT("cui::ArtworkPanel::refresh_image");

    flush_image(false);
    // Needs to be delayed until after WIC calls are made (otherwise painting
    // may happen prematurely).
    auto _ = gsl::finally([this] { invalidate_window(); });

    if (!m_artwork_loader || !m_artwork_loader->is_ready())
        return false;

    const auto artwork_type_index = artwork_type_index_override.value_or(get_displayed_artwork_type_index());
    const auto artwork_type_id = g_artwork_types[artwork_type_index];
    const auto data = m_artwork_loader->get_image(artwork_type_id);

    if (data.is_empty())
        return false;

    try {
        const auto bitmap_data = cui::wic::decode_image_data(data->get_ptr(), data->get_size());
        m_image = cui::gdip::create_bitmap_from_wic_data(bitmap_data);
    } catch (const std::exception& ex) {
        fbh::print_to_console(u8"Artwork panel – loading image failed: ", ex.what());
        return false;
    }

    return static_cast<bool>(m_image);
}

void ArtworkPanel::flush_cached_bitmap()
{
    m_bitmap.reset();
}

void ArtworkPanel::flush_image(bool invalidate)
{
    m_image.reset();
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
    RECT rc;
    GetClientRect(get_wnd(), &rc);
    if (RECT_CX(rc) && RECT_CY(rc) && m_image) {
        HDC dc = nullptr;
        HDC dcc = nullptr;
        dc = GetDC(get_wnd());
        dcc = CreateCompatibleDC(dc);

        m_bitmap.reset(CreateCompatibleBitmap(dc, RECT_CX(rc), RECT_CY(rc)));

        HBITMAP bm_old = SelectBitmap(dcc, m_bitmap.get());

        unsigned err = 0;
        Gdiplus::Graphics graphics(dcc);
        err = graphics.GetLastStatus();

        double ar_source = (double)m_image->GetWidth() / (double)m_image->GetHeight();
        double ar_dest = (double)RECT_CX(rc) / (double)RECT_CY(rc);
        unsigned cx = RECT_CX(rc);
        unsigned cy = RECT_CY(rc);

        graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
        graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        COLORREF cr = cui::colours::helper(g_guid_colour_client).get_colour(cui::colours::colour_background);
        Gdiplus::SolidBrush br(Gdiplus::Color(LOBYTE(LOWORD(cr)), HIBYTE(LOWORD(cr)), LOBYTE(HIWORD(cr))));
        err = graphics.FillRectangle(&br, 0, 0, cx, cy);

        if (m_preserve_aspect_ratio) {
            if (ar_dest < ar_source)
                cy = (unsigned)floor((double)RECT_CX(rc) / ar_source);
            else if (ar_dest > ar_source)
                cx = (unsigned)floor((double)RECT_CY(rc) * ar_source);
        }
        if ((RECT_CY(rc) - cy) % 2)
            cy++;
        if ((RECT_CX(rc) - cx) % 2)
            cx++;

        if (m_image->GetWidth() >= 2 && m_image->GetHeight() >= 2) {
            Gdiplus::Rect destRect(INT((RECT_CX(rc) - cx) / 2), INT((RECT_CY(rc) - cy) / 2), cx, cy);
            graphics.SetClip(destRect);
            Gdiplus::ImageAttributes imageAttributes;
            imageAttributes.SetWrapMode(Gdiplus::WrapModeTileFlipXY);

            graphics.DrawImage(&*m_image, destRect, 0, 0, m_image->GetWidth(), m_image->GetHeight(), Gdiplus::UnitPixel,
                &imageAttributes);
            err = graphics.GetLastStatus();
        }

        SelectBitmap(dcc, bm_old);

        DeleteDC(dcc);
        ReleaseDC(get_wnd(), dc);
    } else
        m_bitmap.reset();
}

void ArtworkPanel::g_on_colours_change()
{
    for (auto& window : g_windows) {
        window->flush_cached_bitmap();
        window->invalidate_window();
    }
}

std::vector<ArtworkPanel*> ArtworkPanel::g_windows;

uie::window_factory<ArtworkPanel> g_artwork_panel;

class ArtworkColoursClient : public cui::colours::client {
public:
    static const GUID g_guid;

    const GUID& get_client_guid() const override { return g_guid_colour_client; };
    void get_name(pfc::string_base& p_out) const override { p_out = "Artwork view"; };

    t_size get_supported_colours() const override { return cui::colours::colour_flag_background; }; // bit-mask
    t_size get_supported_bools() const override { return 0; }; // bit-mask
    bool get_themes_supported() const override { return false; };

    void on_colour_changed(t_size mask) const override { ArtworkPanel::g_on_colours_change(); };
    void on_bool_changed(t_size mask) const override{};
};

namespace {
cui::colours::client::factory<ArtworkColoursClient> g_appearance_client_impl;
};

ArtworkPanel::CompletionNotifyForwarder::CompletionNotifyForwarder(ArtworkPanel* p_this) : m_this(p_this) {}

void ArtworkPanel::CompletionNotifyForwarder::on_completion(unsigned p_code)
{
    m_this->on_completion(p_code);
}

ArtworkPanel::class_data& ArtworkPanel::get_class_data() const
{
    DWORD flags = 0;
    if (cfg_edge_style == 1)
        flags |= WS_EX_CLIENTEDGE;
    if (cfg_edge_style == 2)
        flags |= WS_EX_STATICEDGE;
    __implement_get_class_data_ex2(_T("CUI Artwork View"), _T(""), false, true, 0,
        WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_CONTROLPARENT | flags, 0, IDC_HAND);
}

void ArtworkPanel::set_config(stream_reader* p_reader, t_size size, abort_callback& p_abort)
{
    if (size) {
        p_reader->read_lendian_t(m_track_mode, p_abort);
        t_uint32 version = pfc_infinite;
        try {
            p_reader->read_lendian_t(version, p_abort);
        } catch (exception_io_data_truncation const&) {
        }

        if (version <= 3) {
            p_reader->read_lendian_t(m_preserve_aspect_ratio, p_abort);
            if (version >= 2) {
                p_reader->read_lendian_t(m_lock_type, p_abort);
                if (version >= 3) {
                    p_reader->read_lendian_t(m_selected_artwork_type_index, p_abort);
                    if (m_selected_artwork_type_index >= g_artwork_types.size()) {
                        m_selected_artwork_type_index = 0;
                    }
                    m_artwork_type_override_index.reset();
                }
            }
        }
    }
}

ArtworkPanel::MenuNodeTrackMode::MenuNodeTrackMode(ArtworkPanel* p_wnd, t_size p_value)
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
    p_displayflags = (m_source == p_this->m_track_mode) ? ui_extension::menu_node_t::state_radiochecked : 0;
    return true;
}

const char* ArtworkPanel::MenuNodeTrackMode::get_name(t_size source)
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

ArtworkPanel::MenuNodeArtworkType::MenuNodeArtworkType(ArtworkPanel* p_wnd, t_size p_value)
    : p_this(p_wnd)
    , m_type(p_value)
{
}

void ArtworkPanel::MenuNodeArtworkType::execute()
{
    p_this->m_selected_artwork_type_index = m_type;
    p_this->m_artwork_type_override_index.reset();

    if (!p_this->refresh_image()) {
        p_this->show_stub_image();
    }
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

const char* ArtworkPanel::MenuNodeArtworkType::get_name(t_size source)
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

void ArtworkPanel::MenuNodeSourcePopup::get_child(unsigned p_index, uie::menu_node_ptr& p_out) const
{
    p_out = m_items[p_index].get_ptr();
}

unsigned ArtworkPanel::MenuNodeSourcePopup::get_children_count() const
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

void ArtworkPanel::MenuNodeTypePopup::get_child(unsigned p_index, uie::menu_node_ptr& p_out) const
{
    p_out = m_items[p_index].get_ptr();
}

unsigned ArtworkPanel::MenuNodeTypePopup::get_children_count() const
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
}

bool ArtworkPanel::MenuNodePreserveAspectRatio::get_description(pfc::string_base& p_out) const
{
    return false;
}

bool ArtworkPanel::MenuNodePreserveAspectRatio::get_display_data(
    pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = "Preserve aspect ratio";
    p_displayflags = (p_this->m_preserve_aspect_ratio) ? ui_extension::menu_node_t::state_checked : 0;
    return true;
}

void ArtworkPanel::MenuNodeOptions::execute()
{
    cui::prefs::page_main.get_static_instance().show_tab("Artwork");
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
    p_this->m_lock_type = !p_this->m_lock_type;
    if (p_this->m_lock_type) {
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
    p_displayflags = (p_this->m_lock_type) ? ui_extension::menu_node_t::state_checked : 0;
    return true;
}

}; // namespace cui::artwork_panel
