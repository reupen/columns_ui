#include "pch.h"
#include "ng_playlist_artwork.h"
#include "resource_utils.h"
#include "wic.h"

namespace cui::panels::playlist_view {

namespace {
wil::unique_hbitmap g_create_hbitmap_from_image(
    Gdiplus::Bitmap& bm, int& cx, int& cy, COLORREF cr_back, bool b_reflection)
{
    HDC dc = nullptr;
    HDC dcc = nullptr;
    dc = GetDC(nullptr);
    dcc = CreateCompatibleDC(dc);

    if (b_reflection)
        cy = cx;

    int ocx = cx;
    int ocy = cy;

    int cx_source = gsl::narrow<int>(bm.GetWidth());
    int cy_source = gsl::narrow<int>(bm.GetHeight());

    double ar_source = (double)cx_source / (double)cy_source;
    double ar_dest = (double)ocx / (double)ocy;

    if (ar_dest < ar_source)
        cy = (unsigned)floor((double)ocx / ar_source);
    else if (ar_dest > ar_source)
        cx = (unsigned)floor((double)ocy * ar_source);

    if ((ocx - cx) % 2)
        cx++;

    int reflect_cy = b_reflection ? (cy * 3) / 11 : 0;
    wil::unique_hbitmap bitmap(CreateCompatibleBitmap(dc, cx, cy + reflect_cy));
    HBITMAP bm_old = SelectBitmap(dcc, bitmap.get());

    Gdiplus::Graphics graphics(dcc);
    graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

    Gdiplus::SolidBrush br(Gdiplus::Color(LOBYTE(LOWORD(cr_back)), HIBYTE(LOWORD(cr_back)), LOBYTE(HIWORD(cr_back))));
    graphics.FillRectangle(&br, 0, 0, cx, cy + reflect_cy);

    Gdiplus::ImageAttributes imageAttributes;
    imageAttributes.SetWrapMode(Gdiplus::WrapModeTileFlipXY);
    Gdiplus::Rect destRect(0, 0, cx, cy);
    graphics.DrawImage(&bm, destRect, 0, 0, cx_source, cy_source, Gdiplus::UnitPixel, &imageAttributes);
    Gdiplus::Bitmap scaled(bitmap.get(), nullptr);
    if (reflect_cy) {
        Gdiplus::Rect rectref(0, cy, cx, reflect_cy);
        Gdiplus::Color cr_end(255, LOBYTE(LOWORD(cr_back)), HIBYTE(LOWORD(cr_back)), LOBYTE(HIWORD(cr_back)));
        Gdiplus::Color cr_start(148, LOBYTE(LOWORD(cr_back)), HIBYTE(LOWORD(cr_back)), LOBYTE(HIWORD(cr_back)));
        Gdiplus::Rect destRect(0, cy, cx, reflect_cy);
        graphics.DrawImage(&scaled, destRect, 0, cy, cx, 0 - reflect_cy, Gdiplus::UnitPixel);
        Gdiplus::LinearGradientBrush lgb(rectref, cr_start, cr_end, Gdiplus::LinearGradientModeVertical);
        graphics.FillRectangle(&lgb, rectref);
    }

    SelectBitmap(dcc, bm_old);

    DeleteDC(dcc);
    ReleaseDC(nullptr, dc);

    return bitmap;
}

wil::unique_hbitmap g_create_hbitmap_from_data(
    const album_art_data_ptr& data, int& cx, int& cy, COLORREF cr_back, bool b_reflection)
{
    std::unique_ptr<Gdiplus::Bitmap> bitmap;
    try {
        const auto bitmap_data = wic::decode_image_data(data->get_ptr(), data->get_size());
        bitmap = gdip::create_bitmap_from_wic_data(bitmap_data);
    } catch (const std::exception& ex) {
        fbh::print_to_console("Playlist view – loading image failed: ", ex.what());
        return nullptr;
    }

    return g_create_hbitmap_from_image(*bitmap, cx, cy, cr_back, b_reflection);
}

} // namespace

bool g_get_default_nocover_bitmap_data(album_art_data_ptr& p_out, abort_callback& p_abort)
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

wil::unique_hbitmap g_get_nocover_bitmap(int cx, int cy, COLORREF cr_back, bool b_reflection, abort_callback& p_abort)
{
    album_art_extractor_instance_v2::ptr p_extractor = album_art_manager_v2::get()->open_stub(p_abort);

    album_art_data_ptr data;
    wil::unique_hbitmap ret;
    try {
        data = p_extractor->query(album_art_ids::cover_front, p_abort);
        ret = g_create_hbitmap_from_data(data, cx, cy, cr_back, b_reflection);
    } catch (const exception_aborted&) {
        throw;
    } catch (exception_io_not_found const&) {
    } catch (pfc::exception const&) {
    }
    if (!ret) {
        if (g_get_default_nocover_bitmap_data(data, p_abort))
            ret = g_create_hbitmap_from_data(data, cx, cy, cr_back, b_reflection);
    }
    return ret;
}

void ArtworkReaderManager::request(const metadb_handle_ptr& p_handle, std::shared_ptr<ArtworkReader>& p_out, int cx,
    int cy, COLORREF cr_back, bool b_reflection, BaseArtworkCompletionNotify::ptr_t p_notify)
{
    auto p_new_reader = std::make_shared<ArtworkReader>();
    p_new_reader->initialise(p_handle, cx, cy, cr_back, b_reflection, std::move(p_notify), shared_from_this());
    m_pending_readers.add_item(p_new_reader);
    p_out = p_new_reader;
    flush_pending();
}

void ArtworkReaderManager::on_reader_completion(const ArtworkReader* ptr)
{
    size_t index;
    if (find_current_reader(ptr, index)) {
        m_current_readers[index]->wait_for_and_release_thread();
        m_current_readers[index]->send_completion_notification(m_current_readers[index]);
        m_current_readers.remove_by_idx(index);
    } else {
        if (find_aborting_reader(ptr, index)) {
            m_aborting_readers[index]->wait_for_and_release_thread();
            m_aborting_readers.remove_by_idx(index);
        }
    }
    flush_pending();
}
void ArtworkReaderManager::on_reader_abort(const ArtworkReader* ptr)
{
    on_reader_completion(ptr);
}

class ArtworkReaderNotification : public main_thread_callback {
public:
    void callback_run() override
    {
        if (m_aborted)
            m_manager->on_reader_abort(m_reader);
        else
            m_manager->on_reader_completion(m_reader);
    }

    static void g_run(std::shared_ptr<ArtworkReaderManager> p_manager, bool p_aborted, const ArtworkReader* p_reader)
    {
        service_ptr_t<ArtworkReaderNotification> ptr = new service_impl_t<ArtworkReaderNotification>;
        ptr->m_aborted = p_aborted;
        ptr->m_reader = p_reader;
        ptr->m_manager = std::move(p_manager);

        main_thread_callback_manager::get()->add_callback(ptr.get_ptr());
    }

    bool m_aborted;
    const ArtworkReader* m_reader;
    std::shared_ptr<ArtworkReaderManager> m_manager;
};

DWORD ArtworkReader::on_thread()
{
    TRACK_CALL_TEXT("artwork_reader_ng_t::on_thread");
    (void)mmh::set_thread_description(GetCurrentThread(), L"[Columns UI] Playlist view artwork reader");

    bool b_aborted = false;
    DWORD ret = -1;
    try {
        ret = read_artwork(m_abort);
        m_abort.check();
        m_succeeded = true;
    } catch (const exception_aborted&) {
        m_bitmaps.clear();
        b_aborted = true;
        ret = ERROR_PROCESS_ABORTED;
    } catch (pfc::exception const& e) {
        m_bitmaps.clear();
        console::formatter formatter;
        formatter << "Playlist view – unhandled error loading artwork: " << e.what();
        ret = -1;
    }
    // send this first so thread gets closed first
    ArtworkReaderNotification::g_run(m_manager, b_aborted, this);
    /*if (!b_aborted)
    {
    if (m_notify.is_valid())
    {
    m_notify->on_completion_async(m_succeeded ? ret : 1);
    }
    }
    m_notify.release();*/
    return ret;
}

unsigned ArtworkReader::read_artwork(abort_callback& p_abort)
{
    TRACK_CALL_TEXT("artwork_reader_ng_t::read_artwork");
    constexpr GUID artwork_type_id = album_art_ids::cover_front;

    m_bitmaps.clear();

    const auto p_album_art_manager_v2 = album_art_manager_v2::get();

    album_art_data_ptr data;

    try {
        const auto artwork_api_v2 = p_album_art_manager_v2->open(pfc::list_single_ref_t<metadb_handle_ptr>(m_handle),
            pfc::list_single_ref_t<GUID>(artwork_type_id), p_abort);
        data = artwork_api_v2->query(artwork_type_id, p_abort);
    } catch (const exception_aborted&) {
        throw;
    } catch (exception_io_not_found const&) {
    } catch (pfc::exception const& e) {
        console::formatter formatter;
        formatter << "Playlist view – error loading artwork: " << e.what();
    }

    // Warning: Broken input components can intitialise COM as apartment-threaded. Hence, COM intialisation
    // is done here, after input components have been called, to avoid conflicts.
    auto _ = wil::CoInitializeEx(COINIT_MULTITHREADED);

    if (data.is_valid() && data->get_size() > 0) {
        wil::shared_hbitmap bitmap = g_create_hbitmap_from_data(data, m_cx, m_cy, m_back, m_reflection);
        m_bitmaps.insert_or_assign(artwork_type_id, std::move(bitmap));
        GdiFlush();
    }
    if (!m_bitmaps.count(artwork_type_id)) {
        auto bm = m_manager->request_nocover_image(m_cx, m_cy, m_back, m_reflection, p_abort);
        if (bm) {
            m_bitmaps.insert_or_assign(artwork_type_id, std::move(bm));
            GdiFlush();
        }
    }
    return 1;
}

wil::shared_hbitmap ArtworkReaderManager::request_nocover_image(
    int cx, int cy, COLORREF cr_back, bool b_reflection, abort_callback& p_abort)
{
    insync(m_nocover_sync);
    if (m_nocover_bitmap && m_nocover_cx == cx && m_nocover_cy == cy)
        return m_nocover_bitmap;

    auto bm = g_get_nocover_bitmap(cx, cy, cr_back, b_reflection, p_abort);
    if (bm) {
        m_nocover_cx = cx;
        m_nocover_cy = cy;
        m_nocover_bitmap = std::move(bm);
        return m_nocover_bitmap;
    }
    return nullptr;
}

}; // namespace cui::panels::playlist_view
