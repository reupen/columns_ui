#include "pch.h"

#include "artwork.h"
#include "config.h"
#include "d2d_utils.h"
#include "d3d_utils.h"
#include "imaging.h"
#include "tab_setup.h"

namespace cui::artwork_panel {

namespace {

constexpr unsigned MSG_REFRESH_EFFECTS = WM_USER + 3;
constexpr unsigned MSG_REFRESH_IMAGE = WM_USER + 4;

constexpr bool is_device_reset_error(const HRESULT hr)
{
    return hr == D2DERR_RECREATE_TARGET || hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET;
}

D2D1_COLOR_F srgb_to_linear(D2D1_COLOR_F srgb_colour, float white_level_adjustment)
{
    auto convert_component = [](float value) -> float {
        if (value <= 0.04045f) {
            return value / 12.92f;
        }

        return std::pow((value + 0.055f) / 1.055f, 2.4f);
    };

    return D2D1::ColorF(convert_component(srgb_colour.r) * white_level_adjustment,
        convert_component(srgb_colour.g) * white_level_adjustment,
        convert_component(srgb_colour.b) * white_level_adjustment, srgb_colour.a);
}

std::optional<unsigned> get_sdr_white_level(std::wstring_view device_name)
{
    try {
        std::vector<DISPLAYCONFIG_PATH_INFO> paths;
        std::vector<DISPLAYCONFIG_MODE_INFO> modes;
        constexpr unsigned flags = QDC_ONLY_ACTIVE_PATHS | QDC_VIRTUAL_MODE_AWARE;
        LONG result{};

        do {
            unsigned num_paths{};
            unsigned num_modes{};
            THROW_IF_WIN32_ERROR(GetDisplayConfigBufferSizes(flags, &num_paths, &num_modes));

            paths.resize(num_paths);
            modes.resize(num_modes);

            result = QueryDisplayConfig(flags, &num_paths, paths.data(), &num_modes, modes.data(), nullptr);

            if (result != ERROR_INSUFFICIENT_BUFFER) {
                THROW_IF_WIN32_ERROR(result);
                paths.resize(num_paths);
                modes.resize(num_modes);
            }
        } while (result == ERROR_INSUFFICIENT_BUFFER);

        for (const auto& path : paths) {
            DISPLAYCONFIG_SOURCE_DEVICE_NAME source_name{};
            source_name.header.adapterId = path.sourceInfo.adapterId;
            source_name.header.id = path.sourceInfo.id;
            source_name.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
            source_name.header.size = sizeof(source_name);
            THROW_IF_WIN32_ERROR(DisplayConfigGetDeviceInfo(&source_name.header));

            if (std::wstring_view(source_name.viewGdiDeviceName, std::size(source_name.viewGdiDeviceName))
                != device_name)
                continue;

            DISPLAYCONFIG_SDR_WHITE_LEVEL sdr_white_level{};
            sdr_white_level.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SDR_WHITE_LEVEL;
            sdr_white_level.header.size = sizeof(DISPLAYCONFIG_SDR_WHITE_LEVEL);
            sdr_white_level.header.adapterId = path.targetInfo.adapterId;
            sdr_white_level.header.id = path.targetInfo.id;

            THROW_IF_WIN32_ERROR(DisplayConfigGetDeviceInfo(&sdr_white_level.header));

            return sdr_white_level.SDRWhiteLevel;
        }
    }
    CATCH_LOG()

    return {};
}

int compute_intersection_area(RECT left, RECT right)
{
    return std::max(0l, std::min(left.right, right.right) - std::max(left.left, right.left))
        * std::max(0l, std::min(left.bottom, right.bottom) - std::max(left.top, right.top));
}

bool can_open_path_in_explorer(const char* path)
{
    if (filesystem::g_is_remote_or_unrecognized(path))
        return false;

    if (!archive_impl::g_is_unpack_path(path))
        return true;

    pfc::string8 archive_path;
    pfc::string8 archive_item_path;
    if (!archive_impl::g_parse_unpack_path(path, archive_path, archive_item_path))
        return false;

    return !filesystem::g_is_remote_or_unrecognized(archive_path);
}

} // namespace

// {005C7B29-3915-4b83-A283-C01A4EDC4F3A}
const GUID g_guid_track_mode = {0x5c7b29, 0x3915, 0x4b83, {0xa2, 0x83, 0xc0, 0x1a, 0x4e, 0xdc, 0x4f, 0x3a}};

// {A35E8697-0B8A-4e6f-9DBE-39EC4626524D}
const GUID g_guid_preserve_aspect_ratio = {0xa35e8697, 0xb8a, 0x4e6f, {0x9d, 0xbe, 0x39, 0xec, 0x46, 0x26, 0x52, 0x4d}};

// {F5C8CE6B-5D68-4ce2-8B9F-874D8EDB03B3}
const GUID g_guid_edge_style = {0xf5c8ce6b, 0x5d68, 0x4ce2, {0x8b, 0x9f, 0x87, 0x4d, 0x8e, 0xdb, 0x3, 0xb3}};

enum TrackingMode : uint32_t {
    track_auto_playlist_playing,
    track_playlist,
    track_playing,
    track_auto_selection_playing,
    track_selection,
};

const std::unordered_map<TrackingMode, wil::zstring_view> tracking_mode_labels{
    {track_auto_selection_playing, "Automatic (current selection/playing item)"_zv},
    {track_auto_playlist_playing, "Automatic (playlist selection/playing item)"_zv},
    {track_playing, "Playing item"_zv},
    {track_selection, "Current selection"_zv},
    {track_playlist, "Playlist selection"_zv},
};

const std::unordered_map<GUID, wil::zstring_view> artwork_type_labels{
    {album_art_ids::cover_front, "Front cover"_zv},
    {album_art_ids::cover_back, "Back cover"_zv},
    {album_art_ids::disc, "Disc"_zv},
    {album_art_ids::artist, "Artist"_zv},
};

const auto artwork_type_ids = artwork_type_labels | ranges::views::keys | ranges::to<std::vector>;

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

fbh::ConfigInt32 colour_management_mode({0xb1551e5b, 0x51d7, 0x4702, {0xbe, 0x3f, 0x78, 0x26, 0x13, 0x4d, 0xc3, 0xf2}},
    WI_EnumValue(ColourManagementMode::Legacy));

// {E32DCBA9-A2BF-4901-AB43-228628071410}
static const GUID g_guid_colour_client = {0xe32dcba9, 0xa2bf, 0x4901, {0xab, 0x43, 0x22, 0x86, 0x28, 0x7, 0x14, 0x10}};

void ArtworkPanel::get_config(stream_writer* p_writer, abort_callback& p_abort) const
{
    p_writer->write_lendian_t(m_track_mode, p_abort);
    p_writer->write_lendian_t(static_cast<uint32_t>(current_stream_version), p_abort);
    p_writer->write_lendian_t(m_preserve_aspect_ratio, p_abort);
    p_writer->write_lendian_t(m_artwork_type_locked, p_abort);
    p_writer->write_lendian_t(m_selected_artwork_type_index, p_abort);
}

void ArtworkPanel::get_menu_items(ui_extension::menu_hook_t& p_hook)
{
    if (is_core_image_viewer_available()) {
        p_hook.add_node(uie::menu_node_ptr(new uie::simple_command_menu_node("Open in pop-up viewer",
            "Opens the image in the foobar2000 picture viewer.", 0,
            [this, self = ptr{this}] { open_core_image_viewer(); })));
    }

    if (is_show_in_file_explorer_available()) {
        p_hook.add_node(uie::menu_node_ptr(new uie::simple_command_menu_node("Show in File Explorer",
            "Show and select the file that is the source of the displayed image in File Explorer.", 0,
            [this, self = ptr{this}] { show_in_file_explorer(); })));
    }

    p_hook.add_node(uie::menu_node_ptr(new uie::simple_command_menu_node(
        "Reload artwork", "Reloads the currently displayed artwork.", 0, [this, self = ptr{this}] {
            invalidate_window();
            force_reload_artwork();
        })));

    p_hook.add_node(uie::menu_node_ptr(new uie::menu_node_separator_t()));
    p_hook.add_node(uie::menu_node_ptr(new MenuNodeTypePopup(this)));
    p_hook.add_node(uie::menu_node_ptr(new MenuNodeSourcePopup(this)));

    p_hook.add_node(uie::menu_node_ptr(uie::menu_node_ptr(new uie::simple_command_menu_node("Preserve aspect ratio",
        "Toggle whether the aspect ratio of the displayed image is preserved.",
        m_preserve_aspect_ratio ? uie::menu_node_t::state_checked : 0,
        [this, self = ptr{this}] { toggle_preserve_aspect_ratio(); }))));

    p_hook.add_node(uie::menu_node_ptr(uie::menu_node_ptr(new uie::simple_command_menu_node("Lock artwork type",
        "Toggle whether another the image for another artwork type should not be automatically shown when the selected "
        "artwork type is unavailable.",
        m_artwork_type_locked ? uie::menu_node_t::state_checked : 0,
        [this, self = ptr{this}] { toggle_lock_artwork_type(); }))));

    p_hook.add_node(uie::menu_node_ptr(new uie::simple_command_menu_node("More options",
        "Opens the preferences page for the Artwork view where more options are available.", 0,
        [] { prefs::page_main.get_static_instance().show_tab("Artwork"); })));
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
        m_artwork_reader = std::make_shared<ArtworkReaderManager>();
        now_playing_album_art_notify_manager::get()->add(this);
        m_artwork_reader->set_types(artwork_type_ids);
        play_callback_manager::get()->register_callback(
            this, flag_on_playback_new_track | flag_on_playback_stop | flag_on_playback_edited, false);
        playlist_manager_v3::get()->register_callback(this, playlist_callback_flags);
        g_ui_selection_manager_register_callback_no_now_playing_fallback(this);
        force_reload_artwork();
        g_windows.push_back(this);

        m_display_change_token = system_appearance_manager::add_display_changed_handler(
            [wnd] { PostMessage(wnd, MSG_REFRESH_EFFECTS, 0, 0); });

        m_use_hardware_acceleration_change_token = prefs::add_use_hardware_acceleration_changed_handler([&] {
            reset_d2d_device_resources();
            refresh_image();
        });
        break;
    }
    case WM_DESTROY:
        std::erase(g_windows, this);
        m_use_hardware_acceleration_change_token.reset();
        m_display_change_token.reset();
        ui_selection_manager::get()->unregister_callback(this);
        playlist_manager_v3::get()->unregister_callback(this);
        play_callback_manager::get()->unregister_callback(this);
        now_playing_album_art_notify_manager::get()->remove(this);
        m_selection_handles.remove_all();
        m_show_in_explorer_thread.reset();
        m_artwork_decoder.shut_down();
        if (m_artwork_reader)
            m_artwork_reader->deinitialise();
        m_artwork_reader.reset();
        m_d2d_device_context.reset();
        m_d2d_device.reset();
        m_d2d_factory.reset();
        m_d3d_device.reset();
        m_dxgi_factory.reset();
        m_dxgi_swap_chain.reset();
        m_sdr_white_level.reset();
        m_dxgi_output_desc.reset();
        m_output_effect.reset();
        m_scale_effect.reset();
        m_swap_chain_format.reset();
        m_scale_effect_needs_updating = false;
        break;
    case WM_ERASEBKGND:
        return FALSE;
    case WM_WINDOWPOSCHANGED: {
        const auto lpwp = reinterpret_cast<LPWINDOWPOS>(lp);

        if (lpwp->flags & SWP_NOSIZE)
            break;

        if (m_dxgi_swap_chain) {
            if (m_d2d_device_context) {
                m_d2d_device_context->SetTarget(nullptr);
            }

            HRESULT hr = m_dxgi_swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

            if (is_device_reset_error(hr)) {
                reset_d2d_device_resources();
                refresh_image();
            } else if (FAILED(hr))
                LOG_HR(hr);
        }

        if (m_scale_effect)
            m_scale_effect_needs_updating = true;

        invalidate_window();
        break;
    }
    case WM_LBUTTONDOWN: {
        switch (static_cast<ClickAction>(click_action.get())) {
        case ClickAction::open_image_viewer:
            open_core_image_viewer();
            break;
        case ClickAction::show_in_file_explorer:
            show_in_file_explorer();
            break;
        case ClickAction::show_next_artwork_type:
            show_next_artwork_type();
            break;
        }
        break;
    }
    case WM_CONTEXTMENU:
        handle_wm_contextmenu(wnd, {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)});
        return 0;
    case MSG_REFRESH_EFFECTS:
        reset_effects();
        RedrawWindow(wnd, nullptr, nullptr, RDW_INVALIDATE);
        return 0;
    case MSG_REFRESH_IMAGE:
        refresh_image();
        return 0;
    case WM_PAINT: {
        const auto background_colour = colours::helper(g_guid_colour_client).get_colour(colours::colour_background);

        try {
            create_d2d_device_resources();

            if (!(m_d2d_device_context && m_dxgi_swap_chain)) {
                ValidateRect(wnd, nullptr);
                return 0;
            }

            const auto context = m_d2d_device_context.query<ID2D1DeviceContext>();

            create_effects();

            const auto is_hdr_display
                = m_dxgi_output_desc && m_dxgi_output_desc->ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;

            const auto d2d_background_colour = [&] {
                const auto d2d_colour = uih::d2d::colorref_to_d2d_color(background_colour);
                if (is_advanced_colour_active()) {
                    const auto white_level_adjustment
                        = m_sdr_white_level && is_hdr_display ? *m_sdr_white_level / 1000.f : 1.f;
                    return srgb_to_linear(d2d_colour, white_level_adjustment);
                }

                return d2d_colour;
            }();

            auto image_rect = D2D1::RectF();

            if (m_output_effect)
                THROW_IF_FAILED(context->GetImageLocalBounds(m_output_effect.query<ID2D1Image>().get(), &image_rect));

            m_d2d_device_context->BeginDraw();
            m_d2d_device_context->Clear(d2d_background_colour);

            if (m_output_effect) {
                auto [render_target_width, render_target_height] = m_d2d_device_context->GetSize();

                const auto left = (render_target_width - image_rect.right) * .5f;
                const auto top = (render_target_height - image_rect.bottom) * .5f;

                const auto offset = D2D1::Point2F(left, top);
                context->DrawImage(m_output_effect.get(), &offset, nullptr, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);
            }

            THROW_IF_FAILED(m_d2d_device_context->EndDraw());

            THROW_IF_FAILED(m_dxgi_swap_chain->Present(1, 0));
        } catch (...) {
            if (is_device_reset_error(wil::ResultFromCaughtException())) {
                reset_d2d_device_resources();
                refresh_image();
                return 0;
            }

            LOG_CAUGHT_EXCEPTION();
        }

        ValidateRect(wnd, nullptr);

        return 0;
    }
    }
    return DefWindowProc(wnd, msg, wp, lp);
}

void ArtworkPanel::handle_wm_contextmenu(HWND wnd, POINT pt)
{
    enum : uint32_t {
        ID_OPEN_IMAGE_VIEWER = 1,
        ID_SHOW_IN_FILE_EXPLORER,
        ID_RELOAD_ARTWORK,
        ID_PRESERVE_ASPECT_RATIO,
        ID_LOCK_TYPE,
        ID_MORE_OPTIONS,
        ID_ARTWORK_TYPE_BASE,
    };

    const auto id_tracking_mode_base = ID_ARTWORK_TYPE_BASE + gsl::narrow<uint32_t>(artwork_type_ids.size());

    uih::Menu artwork_type_submenu;
    const auto labels_view = artwork_type_labels | ranges::views::values | ranges::views::enumerate
        | ranges::views::transform(
            [](auto&& item) { return std::make_tuple(gsl::narrow<uint32_t>(item.first), item.second); });

    for (auto&& [index, label] : labels_view) {
        artwork_type_submenu.append_command(ID_ARTWORK_TYPE_BASE + index, mmh::to_utf16(label).c_str(),
            {.is_radio_checked = get_displayed_artwork_type_index() == index});
    }

    uih::Menu tracking_mode_submenu;

    const auto append_tracking_mode_item = [&](TrackingMode mode) {
        tracking_mode_submenu.append_command(id_tracking_mode_base + mode, mmh::to_utf16(tracking_mode_labels.at(mode)),
            {.is_radio_checked = m_track_mode == mode});
    };

    append_tracking_mode_item(track_auto_selection_playing);
    append_tracking_mode_item(track_auto_playlist_playing);
    tracking_mode_submenu.append_separator();
    append_tracking_mode_item(track_playing);
    append_tracking_mode_item(track_selection);
    append_tracking_mode_item(track_playlist);

    uih::Menu menu;

    if (is_core_image_viewer_available())
        menu.append_command(ID_OPEN_IMAGE_VIEWER, L"Open in pop-up viewer");

    if (is_show_in_file_explorer_available())
        menu.append_command(ID_SHOW_IN_FILE_EXPLORER, L"Show in File Explorer");

    menu.append_command(ID_RELOAD_ARTWORK, L"Reload artwork");
    menu.append_separator();
    menu.append_submenu(std::move(artwork_type_submenu), L"Artwork type");
    menu.append_submenu(std::move(tracking_mode_submenu), L"Displayed track");
    menu.append_command(ID_PRESERVE_ASPECT_RATIO, L"Preserve aspect ratio", {.is_checked = m_preserve_aspect_ratio});
    menu.append_command(ID_LOCK_TYPE, L"Lock artwork type", {.is_checked = m_artwork_type_locked});
    menu.append_separator();
    menu.append_command(ID_MORE_OPTIONS, L"More options");

    menu_helpers::win32_auto_mnemonics(menu.get());

    const auto pt_menu = pt.x == -1 && pt.y == -1 ? POINT{} : pt;

    switch (const auto cmd = menu.run(wnd, pt_menu); cmd) {
    case ID_OPEN_IMAGE_VIEWER:
        open_core_image_viewer();
        break;
    case ID_SHOW_IN_FILE_EXPLORER:
        show_in_file_explorer();
        break;
    case ID_RELOAD_ARTWORK:
        invalidate_window();
        force_reload_artwork();
        break;
    case ID_PRESERVE_ASPECT_RATIO:
        toggle_preserve_aspect_ratio();
        break;
    case ID_LOCK_TYPE:
        toggle_lock_artwork_type();
        break;
    case ID_MORE_OPTIONS:
        prefs::page_main.get_static_instance().show_tab("Artwork");
        break;
    default:
        if (std::cmp_greater_equal(cmd, id_tracking_mode_base)) {
            set_tracking_mode(cmd - id_tracking_mode_base);
        } else if (cmd >= ID_ARTWORK_TYPE_BASE) {
            set_artwork_type_index(cmd - ID_ARTWORK_TYPE_BASE);
        }
        break;
    }
}

/**
 * Based on https://learn.microsoft.com/en-gb/windows/win32/direct3darticles/high-dynamic-range#idxgioutput6.
 */
void ArtworkPanel::update_dxgi_output_desc()
{
    m_dxgi_output_desc.reset();

    if (!m_dxgi_factory || !m_dxgi_factory->IsCurrent()) {
        THROW_IF_FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory2), m_dxgi_factory.put_void()));
    }

    if (!m_dxgi_factory.try_query<IDXGIFactory6>())
        return;

    wil::com_ptr<IDXGIAdapter1> dxgi_adapter;
    THROW_IF_FAILED(m_dxgi_factory->EnumAdapters1(0, &dxgi_adapter));

    unsigned index{};
    wil::com_ptr<IDXGIOutput> current_output;
    wil::com_ptr<IDXGIOutput> best_output;
    float best_intersect_area{-1};

    RECT window_rect{};
    THROW_IF_WIN32_BOOL_FALSE(GetWindowRect(get_wnd(), &window_rect));

    while (dxgi_adapter->EnumOutputs(index, &current_output) != DXGI_ERROR_NOT_FOUND) {
        DXGI_OUTPUT_DESC desc{};
        THROW_IF_FAILED(current_output->GetDesc(&desc));
        const RECT desktop_rect = desc.DesktopCoordinates;

        const auto intersection_area = compute_intersection_area(window_rect, desktop_rect);
        if (!best_output || intersection_area > best_intersect_area) {
            best_output = current_output;
            best_intersect_area = static_cast<float>(intersection_area);
        }

        index++;
    }

    if (!best_output)
        return;

    const auto dxgi_output_6 = best_output.try_query<IDXGIOutput6>();

    if (!dxgi_output_6)
        return;

    DXGI_OUTPUT_DESC1 output_desc{};
    THROW_IF_FAILED(dxgi_output_6->GetDesc1(&output_desc));

    m_dxgi_output_desc = output_desc;
    m_sdr_white_level = get_sdr_white_level(
        std::wstring_view(m_dxgi_output_desc->DeviceName, std::size(m_dxgi_output_desc->DeviceName)));
}

void ArtworkPanel::create_d2d_device_resources()
{
    try {
        if (!m_d3d_device) {
            constexpr std::array feature_levels
                = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};

            m_d3d_device = d3d::create_d3d_device(feature_levels, &m_d3d_device_context);
        }

        if (!m_d2d_factory)
            m_d2d_factory = d2d::create_factory(D2D1_FACTORY_TYPE_MULTI_THREADED);

        const auto dxgi_device = m_d3d_device.query<IDXGIDevice1>();

        if (!m_d2d_device) {
            THROW_IF_FAILED(m_d2d_factory->CreateDevice(dxgi_device.get(), &m_d2d_device));
        }

        if (!m_dxgi_swap_chain) {
            wil::com_ptr<IDXGIAdapter> dxgi_adapter;
            THROW_IF_FAILED(dxgi_device->GetAdapter(&dxgi_adapter));

            THROW_IF_FAILED(dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), m_dxgi_factory.put_void()));

            const auto dxgi_factory_6 = m_dxgi_factory.try_query<IDXGIFactory6>();
            const auto use_advanced_colour
                = colour_management_mode == WI_EnumValue(ColourManagementMode::Advanced) && dxgi_factory_6;

            DXGI_SWAP_CHAIN_DESC1 swap_chain_desc{};
            swap_chain_desc.Width = 0;
            swap_chain_desc.Height = 0;
            swap_chain_desc.Format = use_advanced_colour ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM;
            swap_chain_desc.Stereo = false;
            swap_chain_desc.SampleDesc.Count = 1;
            swap_chain_desc.SampleDesc.Quality = 0;
            swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swap_chain_desc.BufferCount = 2;
            swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
            // Once a window has had a flip-model swap chain, replacing it with a non-flip model
            // swap chain doesn't work at all (it just displays the old contents)
            swap_chain_desc.SwapEffect = use_advanced_colour || m_using_flip_model_swap_chain
                ? DXGI_SWAP_EFFECT_FLIP_DISCARD
                : DXGI_SWAP_EFFECT_DISCARD;
            swap_chain_desc.Flags = 0;
            swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

            THROW_IF_FAILED(m_dxgi_factory->CreateSwapChainForHwnd(
                m_d3d_device.get(), get_wnd(), &swap_chain_desc, nullptr, nullptr, &m_dxgi_swap_chain));

            if (use_advanced_colour)
                m_using_flip_model_swap_chain = true;

            m_swap_chain_format = swap_chain_desc.Format;

            if (use_advanced_colour) {
                const auto swap_chain_3 = m_dxgi_swap_chain.query<IDXGISwapChain3>();
                THROW_IF_FAILED(swap_chain_3->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709));
            }

            THROW_IF_FAILED(dxgi_device->SetMaximumFrameLatency(1));

            update_dxgi_output_desc();
        }

        if (!m_d2d_device_context) {
            THROW_IF_FAILED(m_d2d_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_d2d_device_context));
        }

        if (m_d2d_device_context && m_dxgi_swap_chain) {
            wil::com_ptr<ID2D1Image> target;
            m_d2d_device_context->GetTarget(&target);

            if (!target) {
                D2D1_BITMAP_PROPERTIES1 bitmap_properties
                    = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                        D2D1::PixelFormat(*m_swap_chain_format, D2D1_ALPHA_MODE_IGNORE));

                wil::com_ptr<IDXGISurface> dxgi_back_buffer;
                THROW_IF_FAILED(m_dxgi_swap_chain->GetBuffer(0, __uuidof(IDXGISurface), dxgi_back_buffer.put_void()));

                wil::com_ptr<ID2D1Bitmap1> target_bitmap;
                THROW_IF_FAILED(m_d2d_device_context->CreateBitmapFromDxgiSurface(
                    dxgi_back_buffer.get(), &bitmap_properties, &target_bitmap));

                m_d2d_device_context->SetTarget(target_bitmap.get());
            }
        }
    } catch (const wil::ResultException& ex) {
        if (is_device_reset_error(ex.GetErrorCode())) {
            reset_d2d_device_resources();
            PostMessage(get_wnd(), MSG_REFRESH_IMAGE, 0, 0);
            return;
        }

        throw;
    }
}

void ArtworkPanel::reset_d2d_device_resources(bool keep_devices)
{
    m_artwork_decoder.abort();

    reset_effects();

    m_d2d_device_context.reset();
    m_sdr_white_level.reset();
    m_dxgi_output_desc.reset();
    m_dxgi_swap_chain.reset();
    m_swap_chain_format.reset();

    m_artwork_decoder.shut_down();

    if (m_d3d_device_context) {
        m_d3d_device_context->ClearState();
        m_d3d_device_context->Flush();
    }

    m_d3d_device_context.reset();

    if (!keep_devices) {
        m_d2d_device.reset();
        m_d3d_device.reset();
    }
}

void ArtworkPanel::create_effects()
{
    if (m_output_effect) {
        update_scale_effect();
        return;
    }

    if (!m_dxgi_factory || !m_dxgi_factory->IsCurrent()) {
#ifdef _DEBUG
        console::print("Artwork view – DXGI_OUTPUT_DESC1 is out of date, updating…");
#endif
        update_dxgi_output_desc();
    }

    const auto& bitmap = m_artwork_decoder.get_image();
    const auto& image_colour_context = m_artwork_decoder.get_image_colour_context();

    if (!m_d2d_device_context || !bitmap)
        return;

    const auto is_hdr_display
        = m_dxgi_output_desc && m_dxgi_output_desc->ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
    const auto is_advanced_colour = is_advanced_colour_active();

    const auto scale_effect = d2d::create_scale_effect(m_d2d_device_context, calculate_scaling_factor(bitmap));
    scale_effect->SetInput(0, bitmap.get());

    wil::com_ptr<ID2D1Effect> white_level_adjustment_effect;
    wil::com_ptr<ID2D1ColorContext> working_colour_context;

    if (is_advanced_colour) {
        THROW_IF_FAILED(
            m_d2d_device_context->CreateColorContext(D2D1_COLOR_SPACE_SCRGB, nullptr, 0, &working_colour_context));

        const auto working_colour_management_effect
            = d2d::create_colour_management_effect(m_d2d_device_context, image_colour_context, working_colour_context);
        working_colour_management_effect->SetInputEffect(0, scale_effect.get());

        const auto is_hdr_image = m_artwork_decoder.is_float();

        std::optional<float> input_level{};
        std::optional<float> output_level{};

        if (m_sdr_white_level && is_hdr_display && !is_hdr_image) {
            input_level = gsl::narrow_cast<float>(*m_sdr_white_level) / 1000.f * D2D1_SCENE_REFERRED_SDR_WHITE_LEVEL;
            output_level = D2D1_SCENE_REFERRED_SDR_WHITE_LEVEL;
        } else if (is_hdr_image && !is_hdr_display) {
            input_level = D2D1_SCENE_REFERRED_SDR_WHITE_LEVEL;
            output_level = m_dxgi_output_desc ? m_dxgi_output_desc->MaxLuminance : D2D1_SCENE_REFERRED_SDR_WHITE_LEVEL;
        }

        white_level_adjustment_effect
            = d2d::create_white_level_adjustment_effect(m_d2d_device_context, input_level, output_level);
        white_level_adjustment_effect->SetInputEffect(0, working_colour_management_effect.get());
    }

    wil::com_ptr<ID2D1ColorContext> dest_colour_context;

    if (is_advanced_colour) {
        const auto device_context_5 = m_d2d_device_context.query<ID2D1DeviceContext5>();
        wil::com_ptr<ID2D1ColorContext1> dest_colour_context_1;
        THROW_IF_FAILED(device_context_5->CreateColorContextFromDxgiColorSpace(
            DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709, &dest_colour_context_1));
        dest_colour_context = dest_colour_context_1;
    } else {
        dest_colour_context = m_artwork_decoder.get_display_colour_context();
    }

    const auto colour_management_effect = d2d::create_colour_management_effect(m_d2d_device_context,
        working_colour_context ? working_colour_context : image_colour_context, dest_colour_context);

    colour_management_effect->SetInputEffect(
        0, white_level_adjustment_effect ? white_level_adjustment_effect.get() : scale_effect.get());

    m_scale_effect = scale_effect;
    m_output_effect = colour_management_effect;
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
            if (m_selection_handles.get_count())
                request_artwork(m_selection_handles[0]);
            else
                clear_image();
        }
    }
}

void ArtworkPanel::on_playback_stop(play_control::t_stop_reason p_reason) noexcept
{
    m_dynamic_artwork_pending = false;

    if (g_track_mode_includes_now_playing(m_track_mode) && p_reason != play_control::stop_reason_starting_another
        && p_reason != play_control::stop_reason_shutting_down) {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        if (m_track_mode == track_auto_playlist_playing) {
            playlist_manager_v3::get()->activeplaylist_get_selected_items(handles);
        } else if (m_track_mode == track_auto_selection_playing) {
            handles = m_selection_handles;
        }

        if (handles.get_count() > 0)
            request_artwork(handles[0]);
        else
            clear_image();
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
        reset_effects();
        m_artwork_decoder.reset();
        request_artwork(handle, is_from_playback);
    } else {
        clear_image();
    }
}

bool ArtworkPanel::is_core_image_viewer_available() const
{
    if (fb2k::imageViewer::ptr api; !fb2k::imageViewer::tryGet(api)) {
        return false;
    }

    const auto artwork_type_id = artwork_type_ids[get_displayed_artwork_type_index()];
    return m_artwork_reader->has_image(artwork_type_id);
}

void ArtworkPanel::open_core_image_viewer() const
{
    const auto artwork_type_id = artwork_type_ids[get_displayed_artwork_type_index()];
    const album_art_data_ptr data = m_artwork_reader->get_image(artwork_type_id);

    if (!data.is_valid())
        return;

    if (fb2k::imageViewer::ptr api; fb2k::imageViewer::tryGet(api)) {
        api->show(GetParent(get_wnd()), data);
    }
}
bool ArtworkPanel::is_show_in_file_explorer_available() const
{
    const auto artwork_type_id = artwork_type_ids[get_displayed_artwork_type_index()];
    const auto paths = m_artwork_reader->get_paths(artwork_type_id);

    return !m_show_in_explorer_thread && paths.is_valid() && paths->get_count() > 0
        && can_open_path_in_explorer(paths->get_path(0));
}

void ArtworkPanel::show_in_file_explorer()
{
    if (m_show_in_explorer_thread)
        return;

    const auto artwork_type_id = artwork_type_ids[get_displayed_artwork_type_index()];
    const auto paths = m_artwork_reader->get_paths(artwork_type_id);

    if (!paths.is_valid() || paths->get_count() == 0)
        return;

    const auto path = paths->get_path(0);

    if (!can_open_path_in_explorer(path))
        return;

    m_show_in_explorer_thread = std::jthread([this, self{ptr{this}}, path{std::string{path}}] {
        (void)mmh::set_thread_description(GetCurrentThread(), L"[Columns UI] Show in Explorer");

        auto scope_exit
            = wil::scope_exit([&] { fb2k::inMainThread([this, self] { m_show_in_explorer_thread.reset(); }); });

        try {
            pfc::string8 native_path_utf8;

            if (!filesystem::g_get_native_path(path.c_str(), native_path_utf8)) {
                if (!archive_impl::g_is_unpack_path(path.c_str()))
                    return;

                pfc::string8 archive_path;
                pfc::string8 archive_item_path;
                if (!archive_impl::g_parse_unpack_path(path.c_str(), archive_path, archive_item_path))
                    return;

                native_path_utf8.reset();
                if (!filesystem::g_get_native_path(archive_path.c_str(), native_path_utf8))
                    return;
            }

            const auto native_path = mmh::to_utf16(native_path_utf8.c_str());

            auto _ = wil::CoInitializeEx(COINIT_MULTITHREADED);
            wil::unique_any<PIDLIST_ABSOLUTE, decltype(&CoTaskMemFree), CoTaskMemFree> shell_path;
            THROW_IF_FAILED(SHParseDisplayName(native_path.c_str(), nullptr, &shell_path, 0, nullptr));
            THROW_IF_FAILED(SHOpenFolderAndSelectItems(shell_path.get(), 0, nullptr, 0));
        }
        CATCH_LOG()
    });
}

void ArtworkPanel::show_next_artwork_type()
{
    if (!m_artwork_reader || !m_artwork_reader->is_ready())
        return;

    const auto start_artwork_type_index = get_displayed_artwork_type_index();
    auto artwork_type_index = start_artwork_type_index;
    const size_t count = artwork_type_ids.size();

    for (size_t i = 0; i + 1 < count; i++) {
        artwork_type_index = (artwork_type_index + 1) % count;
        const auto artwork_type_id = artwork_type_ids[artwork_type_index];

        if (m_artwork_reader->has_image(artwork_type_id)) {
            m_artwork_type_override_index.reset();
            m_selected_artwork_type_index = artwork_type_index;
            refresh_image();
            break;
        }
    }
}

void ArtworkPanel::set_artwork_type_index(uint32_t index)
{
    m_selected_artwork_type_index = index;
    m_artwork_type_override_index.reset();

    refresh_image();
}

void ArtworkPanel::set_tracking_mode(uint32_t new_tracking_mode)
{
    m_track_mode = new_tracking_mode;
    cfg_track_mode = new_tracking_mode;
    force_reload_artwork();
}

void ArtworkPanel::toggle_preserve_aspect_ratio()
{
    m_preserve_aspect_ratio = !m_preserve_aspect_ratio;
    cfg_preserve_aspect_ratio = m_preserve_aspect_ratio;
    m_scale_effect_needs_updating = true;
    invalidate_window();
}

void ArtworkPanel::toggle_lock_artwork_type()
{
    m_artwork_type_locked = !m_artwork_type_locked;
    if (m_artwork_type_locked) {
        m_selected_artwork_type_index = get_displayed_artwork_type_index();
        m_artwork_type_override_index.reset();
    }
    refresh_image();
}

void ArtworkPanel::on_playlist_switch() noexcept
{
    if (g_track_mode_includes_playlist(m_track_mode)
        && (!g_track_mode_includes_auto(m_track_mode) || !play_control::get()->is_playing())) {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        playlist_manager_v3::get()->activeplaylist_get_selected_items(handles);

        if (handles.get_count())
            request_artwork(handles[0]);
        else
            clear_image();
    }
}

void ArtworkPanel::on_items_selection_change(const bit_array& p_affected, const bit_array& p_state) noexcept
{
    if (g_track_mode_includes_playlist(m_track_mode)
        && (!g_track_mode_includes_auto(m_track_mode) || !play_control::get()->is_playing())) {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        playlist_manager_v3::get()->activeplaylist_get_selected_items(handles);

        if (handles.get_count())
            request_artwork(handles[0]);
        else
            clear_image();
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
    const auto artwork_type_count = gsl::narrow<uint32_t>(artwork_type_ids.size());
    auto iter_limit = artwork_type_count;

    if (m_artwork_type_locked)
        iter_limit = std::min(1u, iter_limit);

    for (auto _ : ranges::views::iota(0u, iter_limit)) {
        const auto artwork_type_index = get_displayed_artwork_type_index();
        const auto artwork_type_id = artwork_type_ids[artwork_type_index];

        if (m_artwork_reader->has_image(artwork_type_id)) {
            b_found = true;
            break;
        }

        m_artwork_type_override_index = (*m_artwork_type_override_index + 1) % artwork_type_count;
    }

    if (!b_found)
        m_artwork_type_override_index.reset();

    refresh_image();
}

void ArtworkPanel::refresh_image()
{
    TRACK_CALL_TEXT("cui::ArtworkPanel::refresh_image");

    if (!m_artwork_reader || !m_artwork_reader->is_ready())
        return;

    const auto artwork_type_index = get_displayed_artwork_type_index();
    const auto artwork_type_id = artwork_type_ids[artwork_type_index];
    auto data = m_artwork_reader->get_image(artwork_type_id);

    if (data.is_empty() && m_artwork_reader->status() != ArtworkReaderStatus::Failed)
        data = m_artwork_reader->get_stub_image(artwork_type_id);

    reset_effects();

    if (!data.is_valid()) {
        m_artwork_decoder.reset();
        invalidate_window();
        return;
    }

    try {
        create_d2d_device_resources();
    }
    CATCH_LOG()

    queue_decode(data);
}

void ArtworkPanel::clear_image()
{
    reset_effects();
    m_artwork_decoder.reset();

    if (m_artwork_reader)
        m_artwork_reader->reset();

    invalidate_window();
}

void ArtworkPanel::reset_effects()
{
    m_scale_effect.reset();
    m_output_effect.reset();
    m_scale_effect_needs_updating = false;
}

D2D1_VECTOR_2F ArtworkPanel::calculate_scaling_factor(const wil::com_ptr<ID2D1Image>& image) const
{
    const auto bitmap = image.query<ID2D1Bitmap>();

    auto [bitmap_width, bitmap_height] = bitmap->GetPixelSize();
    auto [render_target_width, render_target_height] = m_d2d_device_context->GetPixelSize();

    const auto [scaled_width, scaled_height] = cui::utils::calculate_scaled_image_size(gsl::narrow<int>(bitmap_width),
        gsl::narrow<int>(bitmap_height), gsl::narrow<int>(render_target_width), gsl::narrow<int>(render_target_height),
        m_preserve_aspect_ratio, true);

    return D2D1::Vector2F(
        scaled_width / gsl::narrow_cast<float>(bitmap_width), scaled_height / gsl::narrow_cast<float>(bitmap_height));
}

void ArtworkPanel::update_scale_effect()
{
    if (!(m_scale_effect && m_scale_effect_needs_updating))
        return;

    wil::com_ptr<ID2D1Image> image;
    m_scale_effect->GetInput(0, &image);

    LOG_IF_FAILED(m_scale_effect->SetValue(D2D1_SCALE_PROP_SCALE, calculate_scaling_factor(image)));

    m_scale_effect_needs_updating = false;
}

void ArtworkPanel::queue_decode(const album_art_data::ptr& data)
{
    if (!m_d2d_device_context)
        return;

    const auto monitor = is_advanced_colour_active() ? nullptr : MonitorFromWindow(get_wnd(), MONITOR_DEFAULTTONEAREST);

    m_artwork_decoder.decode(m_d2d_device_context, is_advanced_colour_active(), monitor, data, [this, self{ptr{this}}] {
        if (is_device_reset_error(m_artwork_decoder.get_error_result())) {
            reset_d2d_device_resources();
            PostMessage(get_wnd(), MSG_REFRESH_IMAGE, 0, 0);
            return;
        }

        invalidate_window();
    });
}

void ArtworkPanel::invalidate_window() const
{
    RedrawWindow(get_wnd(), nullptr, nullptr, RDW_INVALIDATE);
}

uint32_t ArtworkPanel::get_displayed_artwork_type_index() const
{
    return m_artwork_type_override_index.value_or(m_selected_artwork_type_index);
}

bool ArtworkPanel::is_advanced_colour_active() const
{
    return m_swap_chain_format && *m_swap_chain_format == DXGI_FORMAT_R16G16B16A16_FLOAT;
}

void ArtworkPanel::g_on_colours_change()
{
    for (auto& window : g_windows) {
        if (!window->get_wnd())
            continue;

        window->invalidate_window();
    }
}

void ArtworkPanel::s_on_dark_mode_status_change()
{
    for (auto&& window : g_windows) {
        if (!window->get_wnd())
            continue;

        window->invalidate_window();
    }
}

void ArtworkPanel::s_on_use_advanced_colour_change()
{
    for (auto&& window : g_windows) {
        if (!window->get_wnd())
            continue;

        window->reset_d2d_device_resources(true);
        window->refresh_image();
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
                    if (m_selected_artwork_type_index >= artwork_type_ids.size()) {
                        m_selected_artwork_type_index = 0;
                    }
                    m_artwork_type_override_index.reset();
                }
            }
        }
    }
}

ArtworkPanel::MenuNodeSourcePopup::MenuNodeSourcePopup(service_ptr_t<ArtworkPanel> p_wnd)
{
    const auto make_node = [p_wnd](TrackingMode source) {
        return uie::menu_node_ptr(new uie::simple_command_menu_node(tracking_mode_labels.at(source).c_str(), "",
            p_wnd->m_track_mode == source ? state_radiochecked : 0,
            [p_wnd, source] { p_wnd->set_tracking_mode(source); }));
    };

    m_items.emplace_back(make_node(track_auto_selection_playing));
    m_items.emplace_back(make_node(track_auto_playlist_playing));
    m_items.emplace_back(new uie::menu_node_separator_t());
    m_items.emplace_back(make_node(track_playing));
    m_items.emplace_back(make_node(track_selection));
    m_items.emplace_back(make_node(track_playlist));
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

ArtworkPanel::MenuNodeTypePopup::MenuNodeTypePopup(service_ptr_t<ArtworkPanel> p_wnd)
{
    const auto labels_view = artwork_type_labels | ranges::views::values | ranges::views::enumerate
        | ranges::views::transform(
            [](auto&& item) { return std::make_tuple(gsl::narrow<uint32_t>(item.first), item.second); });

    for (auto&& [index, label] : labels_view) {
        m_items.emplace_back(new uie::simple_command_menu_node(label.c_str(), "",
            p_wnd->get_displayed_artwork_type_index() == index ? state_radiochecked : 0,
            [p_wnd, index] { p_wnd->set_artwork_type_index(index); }));
    }
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

} // namespace cui::artwork_panel
