#include "pch.h"
#include "ng_playlist_artwork.h"

#include "gdi.h"
#include "imaging.h"
#include "resource_utils.h"
#include "wic.h"
#include "foo_ui_columns/d3d.h"

namespace cui::panels::playlist_view {

namespace {

[[nodiscard]] D2D1_RECT_U create_hbitmap_from_d2d_bitmap(const ArtworkRenderingContext::Ptr& context,
    const wil::com_ptr<ID2D1Bitmap1>& d2d_bitmap, const int target_width, const int target_height, bool show_reflection)
{
    const auto bitmap_pixel_size = d2d_bitmap->GetPixelSize();
    const auto bitmap_size = d2d_bitmap->GetSize();

    const auto [render_width_px, render_height_px]
        = cui::utils::calculate_scaled_image_size(static_cast<int>(bitmap_pixel_size.width),
            static_cast<int>(bitmap_pixel_size.height), target_width, target_height, true, false);

    const auto render_width_dip = static_cast<float>(render_width_px);
    const auto render_height_dip = static_cast<float>(render_height_px);

    const auto reflection_height_px = show_reflection ? (target_width * 3) / 11 : 0;
    const auto reflection_height_dip = static_cast<float>(reflection_height_px);

    wil::com_ptr<ID2D1RectangleGeometry> reflection_rect_geometry;
    wil::com_ptr<ID2D1LinearGradientBrush> reflection_linear_gradient_brush;
    wil::com_ptr<ID2D1BitmapBrush1> reflection_bitmap_brush;

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

        const auto bitmap_brush_properties = D2D1::BitmapBrushProperties1(
            D2D1_EXTEND_MODE_CLAMP, D2D1_EXTEND_MODE_CLAMP, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);
        THROW_IF_FAILED(context->d2d_device_context->CreateBitmapBrush(
            d2d_bitmap.get(), bitmap_brush_properties, &reflection_bitmap_brush));

        const auto reflection_rect
            = D2D1::RectF(0, render_height_dip, render_width_dip, render_height_dip + reflection_height_dip);

        THROW_IF_FAILED(context->d2d_factory->CreateRectangleGeometry(reflection_rect, &reflection_rect_geometry));
    }

    context->d2d_device_context->BeginDraw();
    context->d2d_device_context->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.f));

    const auto dest_rect = D2D1::RectF(0, 0, render_width_dip, render_height_dip);
    context->d2d_device_context->DrawBitmap(
        d2d_bitmap.get(), dest_rect, 1.0f, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);

    if (reflection_height_px > 0) {
        reflection_bitmap_brush->SetTransform(
            D2D1::Matrix3x2F::Scale(render_width_dip / bitmap_size.width, -render_height_dip / bitmap_size.height)
            * D2D1::Matrix3x2F::Translation(0.f, 2.f * render_height_dip));

        context->d2d_device_context->FillGeometry(
            reflection_rect_geometry.get(), reflection_bitmap_brush.get(), reflection_linear_gradient_brush.get());
    }

    THROW_IF_FAILED(context->d2d_device_context->EndDraw());

    return D2D1::RectU(0, 0, render_width_px, render_height_px + reflection_height_px);
}

wil::unique_hbitmap create_hbitmap_from_image_data(const ArtworkRenderingContext::Ptr& context,
    const album_art_data_ptr& data, const int width, const int height, bool b_reflection)
{
    try {
        const auto decoder = wic::create_decoder_from_data(data->get_ptr(), data->get_size(), context->wic_factory);
        const auto source = wic::get_image_frame(decoder, GUID_WICPixelFormat32bppPBGRA);

        wil::com_ptr<ID2D1Bitmap1> d2d_bitmap;
        const auto hr = context->d2d_device_context->CreateBitmapFromWicBitmap(source.get(), nullptr, &d2d_bitmap);

        if (hr != E_NOTIMPL)
            THROW_IF_FAILED(hr);

        if (hr == E_NOTIMPL) {
            // For lossy images, the Microsoft JXL codec does not support some interface or
            // method that D2D expects (but making a copy of the bitmap and trying again works fine).
            // (ID2D1DeviceContext2::CreateImageSourceFromWic() doesn't have this problem.)
            wil::com_ptr<IWICBitmap> wic_bitmap_copy;
            THROW_IF_FAILED(
                context->wic_factory->CreateBitmapFromSource(source.get(), WICBitmapCacheOnDemand, &wic_bitmap_copy));

            THROW_IF_FAILED(
                context->d2d_device_context->CreateBitmapFromWicBitmap(wic_bitmap_copy.get(), nullptr, &d2d_bitmap));
        }

        const auto render_rect = create_hbitmap_from_d2d_bitmap(context, d2d_bitmap, width, height, b_reflection);

        wil::com_ptr<ID2D1Bitmap1> cpu_bitmap;
        THROW_IF_FAILED(context->d2d_device_context->CreateBitmap(
            {gsl::narrow<unsigned>(width), gsl::narrow<unsigned>(height)}, nullptr, 0,
            D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_CPU_READ,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
            &cpu_bitmap));

        THROW_IF_FAILED(cpu_bitmap->CopyFromRenderTarget(nullptr, context->d2d_device_context.get(), &render_rect));

        D2D1_MAPPED_RECT mapped_rect{};
        THROW_IF_FAILED(cpu_bitmap->Map(D2D1_MAP_OPTIONS_READ, &mapped_rect));

        auto _ = gsl::finally([&] { THROW_IF_FAILED(cpu_bitmap->Unmap()); });

        return gdi::create_hbitmap_from_32bpp_data(
            width, height, mapped_rect.bits, mapped_rect.pitch * height, mapped_rect.pitch);
    } catch (const std::exception& ex) {
        fbh::print_to_console("Playlist view – loading image failed: ", ex.what());
        return nullptr;
    }
}

wil::unique_hbitmap get_placeholder_hbitmap(const ArtworkRenderingContext::Ptr& context, const int width,
    const int height, bool b_reflection, abort_callback& p_abort)
{
    album_art_extractor_instance_v2::ptr extractor = album_art_manager_v2::get()->open_stub(p_abort);
    wil::unique_hbitmap gdi_bitmap;

    try {
        const auto data = extractor->query(album_art_ids::cover_front, p_abort);
        gdi_bitmap = create_hbitmap_from_image_data(context, data, width, height, b_reflection);
    } catch (const exception_aborted&) {
        throw;
    } catch (exception_io_not_found const&) {
    } catch (pfc::exception const&) {
    }

    if (!gdi_bitmap) {
        if (album_art_data_ptr data; get_default_artwork_placeholder_data(data, p_abort))
            gdi_bitmap = create_hbitmap_from_image_data(context, data, width, height, b_reflection);
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

void ArtworkReaderManager::request(const metadb_handle_ptr& p_handle, ArtworkReader::Ptr& p_out, int cx, int cy,
    bool b_reflection, OnArtworkLoadedCallback callback)
{
    auto p_new_reader
        = std::make_shared<ArtworkReader>(p_handle, cx, cy, b_reflection, std::move(callback), shared_from_this());
    m_pending_readers.emplace_back(std::move(p_new_reader));
    p_out = p_new_reader;
    flush_pending();
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

    flush_pending();
}

ArtworkRenderingContext::Ptr ArtworkRenderingContext::s_create(unsigned width, unsigned height)
{
    constexpr std::array feature_levels = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1};

    wil::com_ptr<ID3D11Device> d3d_device;
    try {
        // Prefer a WARP renderer, it has more consistent performance here
        THROW_IF_FAILED(d3d::create_d3d_device(D3D_DRIVER_TYPE_WARP, feature_levels, &d3d_device));
    } catch (const wil::ResultException&) {
        THROW_IF_FAILED(d3d::create_d3d_device(D3D_DRIVER_TYPE_HARDWARE, feature_levels, &d3d_device));
        console::print(
            "Playlist view artwork – failed to create a software (WARP) renderer, using a hardware renderer instead");
    }

    wil::com_ptr<ID2D1Factory1> d2d_factory;
    THROW_IF_FAILED(
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), d2d_factory.put_void()));

    const auto dxgi_device = d3d_device.query<IDXGIDevice1>();

    wil::com_ptr<ID2D1Device> d2d_device;
    THROW_IF_FAILED(d2d_factory->CreateDevice(dxgi_device.get(), &d2d_device));

    wil::com_ptr<ID2D1DeviceContext> d2d_device_context;
    THROW_IF_FAILED(d2d_device->CreateDeviceContext(
        D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS, &d2d_device_context));

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

            // Warning: Broken input components can intitialise COM as apartment-threaded. Hence, COM intialisation
            // is done here, after input components have been called, to avoid conflicts.
            auto _ = wil::CoInitializeEx(COINIT_MULTITHREADED);

            try {
                if (!context)
                    context = ArtworkRenderingContext::s_create(m_width, m_height);
                else
                    context->enlarge(gsl::narrow<uint32_t>(m_width), gsl::narrow<uint32_t>(m_height));
                m_abort.check();

                render_artwork(context, data, m_abort);
            } catch (const wil::ResultException& ex) {
                if (ex.GetErrorCode() != D2DERR_RECREATE_TARGET)
                    throw;

                m_abort.check();
                context = ArtworkRenderingContext::s_create(m_width, m_height);
                render_artwork(context, data, m_abort);
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

        fb2k::inMainThread([this, context{std::move(context)}] { m_manager->on_reader_done(context, this); });
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

void ArtworkReader::render_artwork(
    const ArtworkRenderingContext::Ptr& context, album_art_data_ptr data, abort_callback& p_abort)
{
    if (data.is_valid() && data->get_size() > 0) {
        wil::shared_hbitmap bitmap
            = create_hbitmap_from_image_data(context, data, m_width, m_height, m_show_reflection);
        m_bitmaps.insert_or_assign(artwork_type_id, std::move(bitmap));
        GdiFlush();
    }

    if (!m_bitmaps.contains(artwork_type_id)) {
        auto bm = m_manager->request_nocover_image(context, m_width, m_height, m_show_reflection, p_abort);
        if (bm) {
            m_bitmaps.insert_or_assign(artwork_type_id, std::move(bm));
            GdiFlush();
        }
    }
}

wil::shared_hbitmap ArtworkReaderManager::request_nocover_image(
    const ArtworkRenderingContext::Ptr& context, int cx, int cy, bool b_reflection, abort_callback& p_abort)
{
    std::scoped_lock lock(m_nocover_mutex);

    if (m_nocover_bitmap && m_nocover_cx == cx && m_nocover_cy == cy)
        return m_nocover_bitmap;

    auto bm = get_placeholder_hbitmap(context, cx, cy, b_reflection, p_abort);
    if (bm) {
        m_nocover_cx = cx;
        m_nocover_cy = cy;
        m_nocover_bitmap = std::move(bm);
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
