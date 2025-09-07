#include "pch.h"
#include "ng_playlist_artwork.h"

#include "d2d_utils.h"
#include "d3d_utils.h"
#include "gdi.h"
#include "imaging.h"
#include "ng_playlist.h"
#include "resource_utils.h"
#include "wcs.h"
#include "wic.h"
#include "win32.h"

namespace cui::panels::playlist_view {

namespace {

[[nodiscard]] D2D1_RECT_U render_d2d_bitmap(const ArtworkRenderingContext::Ptr& context,
    const wil::com_ptr<ID2D1Bitmap1>& d2d_bitmap, const wil::com_ptr<ID2D1ColorContext>& dest_colour_context,
    const int target_width, const int target_height, bool show_reflection)
{
    const auto bitmap_pixel_size = d2d_bitmap->GetPixelSize();
    const auto bitmap_size = d2d_bitmap->GetSize();

    const auto reflection_height_px = show_reflection ? (target_width * 3) / 11 : 0;
    const auto reflection_height_dip = static_cast<float>(reflection_height_px);
    const auto target_height_without_reflection = std::max(0, target_height - reflection_height_px);

    const auto [render_width_px, render_height_px]
        = cui::utils::calculate_scaled_image_size(static_cast<int>(bitmap_pixel_size.width),
            static_cast<int>(bitmap_pixel_size.height), target_width, target_height_without_reflection, true, false);

    const auto render_width_dip = static_cast<float>(render_width_px);
    const auto render_height_dip = static_cast<float>(render_height_px);

    const auto scale_effect = d2d::create_scale_effect(context->d2d_device_context,
        D2D1::Vector2F(render_width_dip / bitmap_size.width, render_height_dip / bitmap_size.height));
    scale_effect->SetInput(0, d2d_bitmap.get());

    wil::com_ptr<ID2D1ColorContext> source_colour_context;
    d2d_bitmap->GetColorContext(&source_colour_context);

    const auto colour_management_effect
        = d2d::create_colour_management_effect(context->d2d_device_context, source_colour_context, dest_colour_context);
    colour_management_effect->SetInputEffect(0, scale_effect.get());

    wil::com_ptr<ID2D1RectangleGeometry> reflection_rect_geometry;
    wil::com_ptr<ID2D1LinearGradientBrush> reflection_linear_gradient_brush;
    wil::com_ptr<ID2D1ImageBrush> reflection_image_brush;
    wil::com_ptr<ID2D1BitmapBrush1> reflection_bitmap_brush;

    // Windows versions before 10 don't support image brushes for ID2D1RenderTarget::FillGeometry()
    const auto device_context_2 = context->d2d_device_context.try_query<ID2D1DeviceContext2>();
    const auto use_image_brush = static_cast<bool>(device_context_2);

    if (reflection_height_px > 0) {
        static const std::array gradient_stops{
            D2D1::GradientStop(0.f, D2D1::ColorF(D2D1::ColorF::Black, 0.42f)),
            D2D1::GradientStop(1.f, D2D1::ColorF(D2D1::ColorF::White, 0.f)),
        };

        wil::com_ptr<ID2D1GradientStopCollection> gradient_stops_collection;
        THROW_IF_FAILED(context->d2d_device_context->CreateGradientStopCollection(
            gradient_stops.data(), gsl::narrow<uint32_t>(gradient_stops.size()), &gradient_stops_collection));

        THROW_IF_FAILED(context->d2d_device_context->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties(
                D2D1::Point2F(0.f, render_height_dip), D2D1::Point2F(0, render_height_dip + reflection_height_dip)),
            gradient_stops_collection.get(), &reflection_linear_gradient_brush));

        if (use_image_brush) {
            const auto effect_image = colour_management_effect.query<ID2D1Image>();

            const auto bitmap_brush_properties
                = D2D1::ImageBrushProperties({0.f, 0.f, render_width_dip, render_height_dip}, D2D1_EXTEND_MODE_CLAMP,
                    D2D1_EXTEND_MODE_CLAMP, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);
            THROW_IF_FAILED(context->d2d_device_context->CreateImageBrush(
                effect_image.get(), bitmap_brush_properties, &reflection_image_brush));

            reflection_image_brush->SetTransform(
                D2D1::Matrix3x2F::Scale(1.f, -1.f) * D2D1::Matrix3x2F::Translation(0.f, 2.f * render_height_dip));
        }

        const auto reflection_rect
            = D2D1::RectF(0, render_height_dip, render_width_dip, render_height_dip + reflection_height_dip);

        THROW_IF_FAILED(context->d2d_factory->CreateRectangleGeometry(reflection_rect, &reflection_rect_geometry));
    }

    context->d2d_device_context->BeginDraw();
    context->d2d_device_context->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.f));

    context->d2d_device_context->DrawImage(
        colour_management_effect.get(), {}, {}, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);

    if (reflection_height_px > 0) {
        try {
            if (use_image_brush) {
                context->d2d_device_context->FillGeometry(reflection_rect_geometry.get(), reflection_image_brush.get(),
                    reflection_linear_gradient_brush.get());
            } else {
                THROW_IF_FAILED(context->d2d_device_context->Flush());

                wil::com_ptr<ID2D1Bitmap1> bitmap_brush_source;
                const auto bitmap_brush_source_properties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_NONE,
                    D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
                THROW_IF_FAILED(context->d2d_device_context->CreateBitmap(
                    {gsl::narrow<uint32_t>(render_width_px), gsl::narrow<uint32_t>(reflection_height_px)}, nullptr, 0,
                    bitmap_brush_source_properties, &bitmap_brush_source));

                const auto source_rect = D2D1::RectU(0u, gsl::narrow<uint32_t>(render_height_px - reflection_height_px),
                    gsl::narrow<uint32_t>(render_width_px), gsl::narrow<uint32_t>(render_height_px));
                THROW_IF_FAILED(bitmap_brush_source->CopyFromRenderTarget(
                    nullptr, context->d2d_device_context.get(), &source_rect));

                THROW_IF_FAILED(context->d2d_device_context->CreateBitmapBrush(
                    bitmap_brush_source.get(), nullptr, nullptr, &reflection_bitmap_brush));

                reflection_bitmap_brush->SetTransform(D2D1::Matrix3x2F::Scale(1.f, -1.f)
                    * D2D1::Matrix3x2F::Translation(0.f, render_height_dip + reflection_height_dip));

                context->d2d_device_context->FillGeometry(reflection_rect_geometry.get(), reflection_bitmap_brush.get(),
                    reflection_linear_gradient_brush.get());
            }
        } catch (...) {
            LOG_IF_FAILED(context->d2d_device_context->EndDraw());
            throw;
        }
    }

    THROW_IF_FAILED(context->d2d_device_context->EndDraw());

    return D2D1::RectU(0, 0, render_width_px, render_height_px + reflection_height_px);
}

[[nodiscard]] wil::com_ptr<ID2D1ColorContext> create_colour_context_for_image(
    const ArtworkRenderingContext::Ptr& context, const wil::com_ptr<IWICBitmapFrameDecode>& bitmap_frame_decode)
{
    const auto wic_colour_context = wic::get_bitmap_source_colour_context(context->wic_factory, bitmap_frame_decode);

    wil::com_ptr<ID2D1ColorContext> d2d_colour_context;

    if (wic_colour_context) {
        LOG_IF_FAILED(context->d2d_device_context->CreateColorContextFromWicColorContext(
            wic_colour_context.get(), &d2d_colour_context));
    } else {
        LOG_IF_FAILED(
            context->d2d_device_context->CreateColorContext(D2D1_COLOR_SPACE_SRGB, nullptr, 0, &d2d_colour_context));
    }

    return d2d_colour_context;
}

[[nodiscard]] wil::com_ptr<ID2D1ColorContext> get_or_create_colour_context_for_display(
    const ArtworkRenderingContext::Ptr& context, const std::wstring& display_profile_name)
{
    wil::com_ptr<ID2D1ColorContext> d2d_colour_context;

    if (const auto iter = context->colour_contexts.find(display_profile_name); iter != context->colour_contexts.end()) {
        d2d_colour_context = iter->second;
        return d2d_colour_context;
    }

    const auto colour_profile = wcs::get_display_colour_profile(display_profile_name);

    if (!colour_profile.empty())
        LOG_IF_FAILED(context->d2d_device_context->CreateColorContext(D2D1_COLOR_SPACE_CUSTOM, colour_profile.data(),
            gsl::narrow<uint32_t>(colour_profile.size()), &d2d_colour_context));

    if (!d2d_colour_context) {
        // ID2D1DeviceContext::CreateColorContext() isn't implemented on Wine
        LOG_IF_FAILED(
            context->d2d_device_context->CreateColorContext(D2D1_COLOR_SPACE_SRGB, nullptr, 0, &d2d_colour_context));
    }

    context->colour_contexts.try_emplace(display_profile_name, d2d_colour_context);

    return d2d_colour_context;
}

[[nodiscard]] wil::unique_hbitmap create_hbitmap_from_image_data(const ArtworkRenderingContext::Ptr& context,
    const std::wstring& display_profile_name, const album_art_data_ptr& data, const int width, const int height,
    bool b_reflection)
{
    try {
        const auto decoder = wic::create_decoder_from_data(data->get_ptr(), data->get_size(), context->wic_factory);

        wil::com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode;
        THROW_IF_FAILED(decoder->GetFrame(0, &bitmap_frame_decode));

        wil::com_ptr<IWICBitmapSource> converted_bitmap;
        THROW_IF_FAILED(
            WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, bitmap_frame_decode.get(), &converted_bitmap));

        const auto d2d_colour_context = create_colour_context_for_image(context, bitmap_frame_decode);

        auto bitmap_properties = D2D1::BitmapProperties1();
        bitmap_properties.colorContext = d2d_colour_context.get();

        wil::com_ptr<ID2D1Bitmap1> d2d_bitmap;
        const auto hr = context->d2d_device_context->CreateBitmapFromWicBitmap(
            converted_bitmap.get(), bitmap_properties, &d2d_bitmap);

        if (hr != E_NOTIMPL)
            THROW_IF_FAILED(hr);

        if (hr == E_NOTIMPL) {
            // For lossy images, the Microsoft JXL codec does not support some interface or
            // method that D2D expects (but making a copy of the bitmap and trying again works fine).
            // (ID2D1DeviceContext2::CreateImageSourceFromWic() doesn't have this problem.)
            wil::com_ptr<IWICBitmap> wic_bitmap_copy;
            THROW_IF_FAILED(context->wic_factory->CreateBitmapFromSource(
                converted_bitmap.get(), WICBitmapCacheOnDemand, &wic_bitmap_copy));

            THROW_IF_FAILED(context->d2d_device_context->CreateBitmapFromWicBitmap(
                wic_bitmap_copy.get(), bitmap_properties, &d2d_bitmap));
        }

        const auto dest_colour_context = get_or_create_colour_context_for_display(context, display_profile_name);
        const auto render_rect
            = render_d2d_bitmap(context, d2d_bitmap, dest_colour_context, width, height, b_reflection);

        const auto render_width = render_rect.right;
        const auto render_height = render_rect.bottom;

        wil::com_ptr<ID2D1Bitmap1> cpu_bitmap;
        THROW_IF_FAILED(context->d2d_device_context->CreateBitmap({render_width, render_height}, nullptr, 0,
            D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_CPU_READ,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
            &cpu_bitmap));

        THROW_IF_FAILED(cpu_bitmap->CopyFromRenderTarget(nullptr, context->d2d_device_context.get(), &render_rect));

        D2D1_MAPPED_RECT mapped_rect{};
        THROW_IF_FAILED(cpu_bitmap->Map(D2D1_MAP_OPTIONS_READ, &mapped_rect));

        auto _ = gsl::finally([&] { THROW_IF_FAILED(cpu_bitmap->Unmap()); });

        return gdi::create_hbitmap_from_32bpp_data(
            render_width, render_height, mapped_rect.bits, mapped_rect.pitch * render_height, mapped_rect.pitch);
    } catch (const std::exception& ex) {
        fbh::print_to_console("Playlist view – loading image failed: ", ex.what());
        return nullptr;
    }
}

wil::unique_hbitmap get_placeholder_hbitmap(const ArtworkRenderingContext::Ptr& context,
    const std::wstring& display_profile_name, const int width, const int height, bool b_reflection,
    abort_callback& p_abort)
{
    album_art_extractor_instance_v2::ptr extractor = album_art_manager_v2::get()->open_stub(p_abort);
    wil::unique_hbitmap gdi_bitmap;

    try {
        const auto data = extractor->query(album_art_ids::cover_front, p_abort);
        gdi_bitmap = create_hbitmap_from_image_data(context, display_profile_name, data, width, height, b_reflection);
    } catch (const exception_aborted&) {
        throw;
    } catch (exception_io_not_found const&) {
    } catch (pfc::exception const&) {
    }

    if (!gdi_bitmap) {
        if (album_art_data_ptr data; get_default_artwork_placeholder_data(data, p_abort))
            gdi_bitmap
                = create_hbitmap_from_image_data(context, display_profile_name, data, width, height, b_reflection);
    }

    return gdi_bitmap;
}

} // namespace

bool get_default_artwork_placeholder_data(album_art_data_ptr& p_out, abort_callback& p_abort)
{
    bool ret = false;
    const WORD resource_id = colours::is_dark_mode_active() ? IDB_DARK_NOCOVER : IDB_LIGHT_NOCOVER;
    const auto [data, size] = resource_utils::get_resource_data(resource_id, L"PNG");

    if (data && size) {
        p_out = album_art_data_impl::g_create(data, size);
        ret = true;
    }
    return ret;
}

void ArtworkReaderManager::request(PlaylistViewGroup::ptr group, const metadb_handle_ptr& p_handle, HMONITOR monitor,
    int cx, int cy, bool b_reflection, OnArtworkLoadedCallback callback)
{
    auto p_new_reader = std::make_shared<ArtworkReader>(
        std::move(group), p_handle, monitor, cx, cy, b_reflection, std::move(callback), shared_from_this());
    m_pending_readers.emplace_back(std::move(p_new_reader));
    start_pending_tasks();
}

void ArtworkReaderManager::cancel_for_group(PlaylistViewGroup* group)
{
    const auto is_same_group = [group](auto&& item) { return item->get_group() == group; };

    auto matching_current_readers = m_current_readers | std::ranges::views::filter(is_same_group);

    for (auto&& reader : matching_current_readers) {
        if (!reader->is_running())
            continue;

        reader->abort();
        m_aborting_readers.emplace_back(reader);
    }

    std::erase_if(m_current_readers, is_same_group);
    std::erase_if(m_pending_readers, is_same_group);
}

void ArtworkReaderManager::on_reader_done(ArtworkRenderingContext::Ptr context, const ArtworkReader* ptr)
{
    if (const auto iter = std::ranges::find_if(m_current_readers, [ptr](auto&& item) { return item.get() == ptr; });
        iter != m_current_readers.end()) {
        (*iter)->send_completion_notification();
        m_current_readers.erase(iter);
    } else {
        std::erase_if(m_aborting_readers, [ptr](auto&& item) { return item.get() == ptr; });
    }

    if (context)
        m_rendering_contexts.emplace_back(std::move(context));

    start_pending_tasks();
}

ArtworkRenderingContext::Ptr ArtworkRenderingContext::s_create(unsigned width, unsigned height)
{
    constexpr std::array feature_levels
        = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};

    const auto d3d_device = d3d::create_d3d_device(feature_levels);
    const auto d2d_factory = d2d::create_factory(D2D1_FACTORY_TYPE_SINGLE_THREADED);
    const auto dxgi_device = d3d_device.query<IDXGIDevice1>();

    wil::com_ptr<ID2D1Device> d2d_device;
    THROW_IF_FAILED(d2d_factory->CreateDevice(dxgi_device.get(), &d2d_device));

    wil::com_ptr<ID2D1DeviceContext> d2d_device_context;
    THROW_IF_FAILED(d2d_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2d_device_context));

    const auto bitmap_properties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

    wil::com_ptr<ID2D1Bitmap1> target_bitmap;
    THROW_IF_FAILED(d2d_device_context->CreateBitmap({width, height}, nullptr, 0, &bitmap_properties, &target_bitmap));

    d2d_device_context->SetTarget(target_bitmap.get());
    auto wic_factory = wic::create_factory();

    return std::make_shared<ArtworkRenderingContext>(
        d3d_device, d2d_factory, d2d_device, d2d_device_context, wic_factory);
}

void ArtworkRenderingContext::enlarge(unsigned width, unsigned height) const
{
    if (const auto [current_width, current_height] = d2d_device_context->GetPixelSize();
        current_width >= width && current_height >= height)
        return;

    const auto bitmap_properties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

    wil::com_ptr<ID2D1Bitmap1> target_bitmap;
    THROW_IF_FAILED(d2d_device_context->CreateBitmap({width, height}, nullptr, 0, &bitmap_properties, &target_bitmap));

    d2d_device_context->SetTarget(target_bitmap.get());
}

void ArtworkReader::start(ArtworkRenderingContext::Ptr context)
{
    m_thread = std::jthread([this, context{std::move(context)}]() mutable {
        TRACK_CALL_TEXT("cui::playlist_view::ArtworkReader::thread");
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
        (void)mmh::set_thread_description(GetCurrentThread(), L"[Columns UI] Playlist view artwork worker");

        try {
            const auto data = read_artwork(m_abort);
            m_abort.check();

            const auto display_device_key = win32::get_display_device_key(m_monitor);
            const auto display_profile_name = wcs::get_display_colour_profile_name(display_device_key.c_str());

            // Warning: Broken input components can intitialise COM as apartment-threaded. Hence, COM intialisation
            // is done here, after input components have been called, to avoid conflicts.
            auto _ = wil::CoInitializeEx(COINIT_MULTITHREADED);

            try {
                if (!context)
                    context = ArtworkRenderingContext::s_create(m_width, m_height);
                else
                    context->enlarge(gsl::narrow<uint32_t>(m_width), gsl::narrow<uint32_t>(m_height));
                m_abort.check();

                render_artwork(context, display_profile_name, data, m_abort);
            } catch (const wil::ResultException& ex) {
                if (ex.GetErrorCode() != D2DERR_RECREATE_TARGET)
                    throw;

                m_abort.check();
                context = ArtworkRenderingContext::s_create(m_width, m_height);
                render_artwork(context, display_profile_name, data, m_abort);
            }

            m_abort.check();
            m_status = ArtworkReaderStatus::Succeeded;
        } catch (const exception_aborted&) {
            m_status = ArtworkReaderStatus::Aborted;
        } catch (const std::exception& ex) {
            console::print("Playlist view – unexpected error reading artwork: ", ex.what());
            m_status = ArtworkReaderStatus::Failed;

            try {
                throw;
            } catch (const wil::ResultException& wil_ex) {
                if (wil_ex.GetErrorCode() == D2DERR_RECREATE_TARGET)
                    context.reset();
            } catch (...) {
            }
        }

        if (m_status != ArtworkReaderStatus::Succeeded)
            m_bitmaps.clear();

        fb2k::inMainThread(
            [this, manager{m_manager}, context{std::move(context)}] { manager->on_reader_done(context, this); });
    });
}

album_art_data_ptr ArtworkReader::read_artwork(abort_callback& p_abort)
{
    constexpr GUID artwork_type_id = album_art_ids::cover_front;

    m_bitmaps.clear();

    const auto p_album_art_manager_v2 = album_art_manager_v2::get();

    try {
        const auto artwork_api_v2 = p_album_art_manager_v2->open(pfc::list_single_ref_t<metadb_handle_ptr>(m_handle),
            pfc::list_single_ref_t<GUID>(artwork_type_id), p_abort);
        return artwork_api_v2->query(artwork_type_id, p_abort);
    } catch (const exception_aborted&) {
        throw;
    } catch (exception_io_not_found const&) {
    } catch (pfc::exception const& e) {
        console::formatter formatter;
        formatter << "Playlist view – error loading artwork: " << e.what();
    }

    return {};
}

void ArtworkReader::render_artwork(const ArtworkRenderingContext::Ptr& context,
    const std::wstring& display_profile_name, album_art_data_ptr data, abort_callback& p_abort)
{
    if (data.is_valid() && data->get_size() > 0) {
        wil::shared_hbitmap bitmap
            = create_hbitmap_from_image_data(context, display_profile_name, data, m_width, m_height, m_show_reflection);
        m_bitmaps.insert_or_assign(artwork_type_id, std::move(bitmap));
        GdiFlush();
    }

    if (!m_bitmaps.contains(artwork_type_id)) {
        auto bm = m_manager->request_nocover_image(
            context, display_profile_name, m_width, m_height, m_show_reflection, p_abort);
        if (bm) {
            m_bitmaps.insert_or_assign(artwork_type_id, std::move(bm));
            GdiFlush();
        }
    }
}

wil::shared_hbitmap ArtworkReaderManager::request_nocover_image(const ArtworkRenderingContext::Ptr& context,
    const std::wstring& display_profile_name, int cx, int cy, bool b_reflection, abort_callback& p_abort)
{
    {
        std::scoped_lock lock(m_nocover_mutex);
        p_abort.check();

        if (m_nocover_bitmap && m_nocover_cx == cx && m_nocover_cy == cy)
            return m_nocover_bitmap;
    }

    auto bitmap = get_placeholder_hbitmap(context, display_profile_name, cx, cy, b_reflection, p_abort);

    if (bitmap) {
        std::scoped_lock lock(m_nocover_mutex);
        p_abort.check();

        m_nocover_cx = cx;
        m_nocover_cy = cy;
        m_nocover_bitmap = std::move(bitmap);
        return m_nocover_bitmap;
    }
    return nullptr;
}

std::shared_ptr<ArtworkRenderingContext> ArtworkReaderManager::get_d2d_device_context()
{
    if (m_rendering_contexts.empty())
        return {};

    const auto rendering_context = m_rendering_contexts.back();
    m_rendering_contexts.pop_back();
    return rendering_context;
}

}; // namespace cui::panels::playlist_view
