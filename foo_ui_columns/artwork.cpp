#include "pch.h"

#include "artwork.h"
#include "config.h"
#include "wic.h"

namespace cui::artwork_panel {

namespace {

std::tuple<int, int> calculate_scaled_size(
    int bitmap_width, int bitmap_height, int client_width, int client_height, bool preserve_aspect_ratio)
{
    int scaled_width = client_width;
    int scaled_height = client_height;

    const double source_aspect_ratio = gsl::narrow_cast<double>(bitmap_width) / gsl::narrow_cast<double>(bitmap_height);
    const double client_aspect_ratio = gsl::narrow_cast<double>(client_width) / gsl::narrow_cast<double>(client_height);

    if (preserve_aspect_ratio) {
        if (client_aspect_ratio < source_aspect_ratio)
            scaled_height
                = gsl::narrow_cast<unsigned>(floor(gsl::narrow_cast<double>(client_width) / source_aspect_ratio));
        else if (client_aspect_ratio > source_aspect_ratio)
            scaled_width
                = gsl::narrow_cast<unsigned>(floor(gsl::narrow_cast<double>(client_height) * source_aspect_ratio));
    }

    if (((client_height - scaled_height) % 2) != 0)
        ++scaled_height;

    if (((client_width - scaled_width) % 2) != 0)
        ++scaled_width;

    return {scaled_width, scaled_height};
}

D2D1_COLOR_F srgb_to_linear(const D2D1_COLOR_F& srgb)
{
    auto convert_component = [](float c) -> float {
        if (c <= 0.04045f) {
            return c / 12.92f;
        } else {
            return std::pow((c + 0.055f) / 1.055f, 2.4f);
        }
    };

    return D2D1::ColorF(convert_component(srgb.r), convert_component(srgb.g), convert_component(srgb.b),
        srgb.a // Alpha remains unchanged
    );
}

std::optional<UINT32> GetSDRWhiteLevel()
{
    // Step 1: Get the required buffer sizes
    UINT32 pathCount = 0;
    UINT32 modeCount = 0;
    LONG result = GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount);
    if (result != ERROR_SUCCESS) {
        return std::nullopt;
    }

    // Step 2: Allocate buffers
    std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);

    // Step 3: Query the display configuration
    result = QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, paths.data(), &modeCount, modes.data(), nullptr);
    if (result != ERROR_SUCCESS) {
        return std::nullopt;
    }

    // Step 4: For each active path, try to get the SDR white level
    for (UINT32 i = 0; i < pathCount; ++i) {
        DISPLAYCONFIG_SDR_WHITE_LEVEL sdrWhiteLevel = {};
        sdrWhiteLevel.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SDR_WHITE_LEVEL;
        sdrWhiteLevel.header.size = sizeof(DISPLAYCONFIG_SDR_WHITE_LEVEL);
        sdrWhiteLevel.header.adapterId = paths[i].targetInfo.adapterId;
        sdrWhiteLevel.header.id = paths[i].targetInfo.id;

        result = DisplayConfigGetDeviceInfo(&sdrWhiteLevel.header);
        if (result == ERROR_SUCCESS) {
            return sdrWhiteLevel.SDRWhiteLevel;
        }
    }

    return std::nullopt;
}

std::optional<UINT32> GetSDRWhiteLevelForMonitorDirect(HMONITOR hMonitor)
{
    if (!hMonitor) {
        return std::nullopt;
    }

    // Get monitor info to get the device name
    MONITORINFOEXW monitorInfo = {};
    monitorInfo.cbSize = sizeof(MONITORINFOEXW);

    if (!GetMonitorInfoW(hMonitor, &monitorInfo)) {
        return std::nullopt;
    }

    // Step 1: Get the required buffer sizes
    UINT32 pathCount = 0;
    UINT32 modeCount = 0;
    LONG result = GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount);
    if (result != ERROR_SUCCESS) {
        return std::nullopt;
    }

    // Step 2: Allocate buffers
    std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);

    // Step 3: Query the display configuration
    result = QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, paths.data(), &modeCount, modes.data(), nullptr);
    if (result != ERROR_SUCCESS) {
        return std::nullopt;
    }

    // Step 4: Find the path that corresponds to our monitor's device name
    for (UINT32 i = 0; i < pathCount; ++i) {
        // Get the source device name for this path
        DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName = {};
        sourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
        sourceName.header.size = sizeof(DISPLAYCONFIG_SOURCE_DEVICE_NAME);
        sourceName.header.adapterId = paths[i].sourceInfo.adapterId;
        sourceName.header.id = paths[i].sourceInfo.id;

        result = DisplayConfigGetDeviceInfo(&sourceName.header);
        if (result == ERROR_SUCCESS) {
            // Compare with our monitor's device name
            if (wcscmp(sourceName.viewGdiDeviceName, monitorInfo.szDevice) == 0) {
                // Found the matching path, now get the SDR white level
                DISPLAYCONFIG_SDR_WHITE_LEVEL sdrWhiteLevel = {};
                sdrWhiteLevel.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SDR_WHITE_LEVEL;
                sdrWhiteLevel.header.size = sizeof(DISPLAYCONFIG_SDR_WHITE_LEVEL);
                sdrWhiteLevel.header.adapterId = paths[i].targetInfo.adapterId;
                sdrWhiteLevel.header.id = paths[i].targetInfo.id;

                result = DisplayConfigGetDeviceInfo(&sdrWhiteLevel.header);
                if (result == ERROR_SUCCESS) {
                    return sdrWhiteLevel.SDRWhiteLevel;
                }
            }
        }
    }

    return std::nullopt;
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
            invalidate_window();
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
        if (m_artwork_reader)
            m_artwork_reader->deinitialise();
        m_artwork_reader.reset();
        m_d2d_device_context.reset();
        m_d2d_factory.reset();
        m_d3d_device.reset();
        m_dxgi_swap_chain.reset();
        break;
    case WM_ERASEBKGND:
        return FALSE;
    case WM_WINDOWPOSCHANGED: {
        auto lpwp = (LPWINDOWPOS)lp;
        if (!(lpwp->flags & SWP_NOSIZE)) {
            if (m_dxgi_swap_chain) {
                RECT client_rect{};
                GetClientRect(wnd, &client_rect);

                m_d2d_device_context.reset();

                HRESULT hr = m_dxgi_swap_chain->ResizeBuffers(2, gsl::narrow<unsigned>(wil::rect_width(client_rect)),
                    gsl::narrow<unsigned>(wil::rect_height(client_rect)), DXGI_FORMAT_R16G16B16A16_FLOAT, 0);

                if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
                    m_dxgi_swap_chain.reset();
                    m_d3d_device.reset();
                    m_d2d_device.reset();
                } else if (FAILED(hr))
                    LOG_HR(hr);
            }

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
        const auto background_colour = colours::helper(g_guid_colour_client).get_colour(colours::colour_background);

        try {
            create_d2d_render_target();

            if (!m_d2d_device_context)
                return 0;

            const auto context = m_d2d_device_context.query<ID2D1DeviceContext>();
            const auto context_4 = m_d2d_device_context.query<ID2D1DeviceContext5>();
            wil::com_ptr<ID2D1Effect> colour_management_effect;
            wil::com_ptr<ID2D1Effect> scale_effect;
            wil::com_ptr<ID2D1Effect> white_level_adjustment_effect;

            THROW_IF_FAILED(context->CreateEffect(CLSID_D2D1ColorManagement, colour_management_effect.put()));
            wil::com_ptr<ID2D1ColorContext1> destColorContext;
            THROW_IF_FAILED(context_4->CreateColorContextFromDxgiColorSpace(
                DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709, &destColorContext));
            THROW_IF_FAILED(context->CreateEffect(CLSID_D2D1Scale, scale_effect.put()));
            THROW_IF_FAILED(context->CreateEffect(CLSID_D2D1WhiteLevelAdjustment, white_level_adjustment_effect.put()));

            const auto d2d_background_colour = srgb_to_linear(uih::d2d::colorref_to_d2d_color(background_colour));

            m_d2d_device_context->BeginDraw();
            m_d2d_device_context->Clear(d2d_background_colour);

            const auto& bitmap = m_artwork_decoder.get_image();
            const auto& color_context = m_artwork_decoder.get_color_context();

            if (bitmap) {
                if (color_context) {
                    console::print("have color_context");

                    THROW_IF_FAILED(colour_management_effect->SetValue(
                        D2D1_COLORMANAGEMENT_PROP_SOURCE_COLOR_CONTEXT, color_context.get()));
                }

                THROW_IF_FAILED(colour_management_effect->SetValue(
                    D2D1_COLORMANAGEMENT_PROP_DESTINATION_COLOR_CONTEXT, destColorContext.get()));
                THROW_IF_FAILED(colour_management_effect->SetValue(
                    D2D1_COLORMANAGEMENT_PROP_QUALITY, D2D1_COLORMANAGEMENT_QUALITY_BEST));

                auto monitor_sdr_level = GetSDRWhiteLevel();
                auto input_level = monitor_sdr_level ? (float)(*monitor_sdr_level) / 1000.f * 80.f : 80.f;

                console::print(input_level);

                THROW_IF_FAILED(white_level_adjustment_effect->SetValue(
                    D2D1_WHITELEVELADJUSTMENT_PROP_INPUT_WHITE_LEVEL, input_level));
                THROW_IF_FAILED(
                    white_level_adjustment_effect->SetValue(D2D1_WHITELEVELADJUSTMENT_PROP_OUTPUT_WHITE_LEVEL, 80.0f));

                colour_management_effect->SetInput(0, bitmap.get());

                auto [bitmap_width, bitmap_height] = bitmap->GetPixelSize();
                auto [render_target_width, render_target_height] = m_d2d_device_context->GetPixelSize();

                float dpi_x{};
                float dpi_y{};
                m_d2d_device_context->GetDpi(&dpi_x, &dpi_y);

                const auto [scaled_width, scaled_height] = calculate_scaled_size(gsl::narrow<int>(bitmap_width),
                    gsl::narrow<int>(bitmap_height), gsl::narrow<int>(render_target_width),
                    gsl::narrow<int>(render_target_height), m_preserve_aspect_ratio);

                auto left = (render_target_width - scaled_width) * .5f * 96.0f / dpi_x;
                auto top = (render_target_height - scaled_height) * .5f * 96.0f / dpi_x;

                THROW_IF_FAILED(scale_effect->SetValue(D2D1_SCALE_PROP_SCALE,
                    D2D1::Vector2F(scaled_width / gsl::narrow_cast<float>(bitmap_width),
                        scaled_height / gsl::narrow_cast<float>(bitmap_height))));
                scale_effect->SetInputEffect(0, colour_management_effect.get());

                white_level_adjustment_effect->SetInputEffect(0, scale_effect.get());

                auto rect
                    = D2D1::RectF(left, top, left + scaled_width * 96.0f / dpi_x, top + scaled_height * 96.0f / dpi_y);

                if (context_4) {
                    auto offset = D2D1::Point2F(left, top);
                    context_4->DrawImage(white_level_adjustment_effect.get(), &offset, nullptr,
                        D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);
                } else
                    context->DrawBitmap(bitmap.get(), &rect, 1.f, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);
            }

            auto result = m_d2d_device_context->EndDraw();

            if (result != D2DERR_RECREATE_TARGET)
                THROW_IF_FAILED(result);

            result = m_dxgi_swap_chain->Present(1, 0);

            if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET) {
                m_d2d_device_context.reset();
                m_d2d_device.reset();
                m_d3d_device.reset();
                m_artwork_decoder.reset();
                refresh_image();
                return 0;
            }
        }
        CATCH_LOG();

        ValidateRect(wnd, nullptr);

        return 0;
    }
    }
    return DefWindowProc(wnd, msg, wp, lp);
}
void ArtworkPanel::create_d2d_render_target()
{
    if (!m_d3d_device) {
        D3D_FEATURE_LEVEL feature_levels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1};

        THROW_IF_FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT | (_DEBUG ? D3D11_CREATE_DEVICE_DEBUG : 0), feature_levels,
            std::size(feature_levels), D3D11_SDK_VERSION, &m_d3d_device, nullptr, nullptr));
    }

    if (!m_d2d_factory) {
        D2D1_FACTORY_OPTIONS options{};

#ifdef _DEBUG
        options.debugLevel = IsDebuggerPresent() ? D2D1_DEBUG_LEVEL_INFORMATION : D2D1_DEBUG_LEVEL_NONE;
#endif

        THROW_IF_FAILED(D2D1CreateFactory(
            D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory1), &options, m_d2d_factory.put_void()));
    }

    const auto dxgi_device = m_d3d_device.query<IDXGIDevice1>();

    if (!m_d2d_device) {
        THROW_IF_FAILED(m_d2d_factory->CreateDevice(dxgi_device.get(), &m_d2d_device));
    }

    const auto set_swap_chain = !m_dxgi_swap_chain || !m_d2d_device_context;

    if (!m_dxgi_swap_chain) {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
        swapChainDesc.Width = 0; // use automatic sizing
        swapChainDesc.Height = 0;
        swapChainDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        swapChainDesc.Stereo = false;
        swapChainDesc.SampleDesc.Count = 1; // don't use multi-sampling
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2; // use double buffering to enable flip
        swapChainDesc.Scaling = DXGI_SCALING_NONE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // all apps must use this SwapEffect
        swapChainDesc.Flags = 0;

        wil::com_ptr<IDXGIAdapter> dxgi_adapter;
        THROW_IF_FAILED(dxgi_device->GetAdapter(&dxgi_adapter));

        wil::com_ptr<IDXGIFactory2> dxgi_factory;
        THROW_IF_FAILED(dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), dxgi_factory.put_void()));

        THROW_IF_FAILED(dxgi_factory->CreateSwapChainForHwnd(
            m_d3d_device.get(), get_wnd(), &swapChainDesc, nullptr, nullptr, &m_dxgi_swap_chain));

        auto swap_chain_3 = m_dxgi_swap_chain.query<IDXGISwapChain3>();

        UINT colour_space_flags{};
        THROW_IF_FAILED(
            swap_chain_3->CheckColorSpaceSupport(DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709, &colour_space_flags));

        if ((colour_space_flags & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT)) {
            console::print("Set colour space");

            THROW_IF_FAILED(swap_chain_3->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709));
        }

        THROW_IF_FAILED(dxgi_device->SetMaximumFrameLatency(1));

        // wil::com_ptr<ID3D11Texture2D> backBuffer;
        // THROW_IF_FAILED(m_dxgi_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), backBuffer.put_void()));
        //
    }

    if (!m_d2d_device_context) {
        THROW_IF_FAILED(m_d2d_device->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS, &m_d2d_device_context));
    }

    if (set_swap_chain) {
        auto dpi = gsl::narrow_cast<float>(uih::get_system_dpi_cached().cx);
        D2D1_BITMAP_PROPERTIES1 bitmapProperties
            = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(DXGI_FORMAT_R16G16B16A16_FLOAT, D2D1_ALPHA_MODE_PREMULTIPLIED), dpi, dpi);

        wil::com_ptr<IDXGISurface> dxgiBackBuffer;
        THROW_IF_FAILED(m_dxgi_swap_chain->GetBuffer(0, __uuidof(IDXGISurface), dxgiBackBuffer.put_void()));

        wil::com_ptr<ID2D1Bitmap1> m_d2dTargetBitmap;
        THROW_IF_FAILED(m_d2d_device_context->CreateBitmapFromDxgiSurface(
            dxgiBackBuffer.get(), &bitmapProperties, &m_d2dTargetBitmap));

        m_d2d_device_context->SetTarget(m_d2dTargetBitmap.get());
    }

#if 0
    if (!m_d2d_render_target) {
        RECT rect{};
        GetClientRect(get_wnd(), &rect);

        const auto base_properties = D2D1::RenderTargetProperties();
        const auto hwnd_properties = D2D1::HwndRenderTargetProperties(
            get_wnd(), {gsl::narrow<unsigned>(wil::rect_width(rect)), gsl::narrow<unsigned>(wil::rect_height(rect))});
        THROW_IF_FAILED(
            m_d2d_factory->CreateHwndRenderTarget(&base_properties, &hwnd_properties, &m_d2d_render_target));
    }
#endif
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

    if (handle.is_valid())
        request_artwork(handle, is_from_playback);
    else
        clear_image();
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
    if (!m_artwork_reader || !m_artwork_reader->is_ready())
        return;

    album_art_data_ptr data;

    if (m_artwork_reader->status() != ArtworkReaderStatus::Failed) {
        const auto artwork_type_id = g_artwork_types[get_displayed_artwork_type_index()];
        data = m_artwork_reader->get_stub_image(artwork_type_id);
    }

    if (!data.is_valid()) {
        m_artwork_decoder.reset();
        invalidate_window();
        return;
    }

    try {
        create_d2d_render_target();
    }
    CATCH_LOG()

    if (m_d2d_device_context)
        m_artwork_decoder.decode(m_d2d_device_context, data, [this, self{ptr{this}}] { invalidate_window(); });
}

void ArtworkPanel::refresh_image()
{
    TRACK_CALL_TEXT("cui::ArtworkPanel::refresh_image");

    if (!m_artwork_reader || !m_artwork_reader->is_ready())
        return;

    const auto artwork_type_index = get_displayed_artwork_type_index();
    const auto artwork_type_id = g_artwork_types[artwork_type_index];
    const auto data = m_artwork_reader->get_image(artwork_type_id);

    if (data.is_empty())
        return;

    try {
        create_d2d_render_target();
    }
    CATCH_LOG()

    if (m_d2d_device_context)
        m_artwork_decoder.decode(m_d2d_device_context, data, [this, self{ptr{this}}] { invalidate_window(); });
}

void ArtworkPanel::clear_image()
{
    m_artwork_decoder.reset();

    if (m_artwork_reader)
        m_artwork_reader->reset();

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
