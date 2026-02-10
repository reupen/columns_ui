#include "pch.h"
#include "rebar.h"

#include "dark_mode.h"
#include "main_window.h"

// extern HBITMAP buttons_images;

namespace cui::rebar {

RebarWindow* g_rebar_window{};
HWND g_rebar{};

ConfigRebar g_cfg_rebar(GUID{0xd26d3aa5, 0x9157, 0xbf8e, {0xd5, 0x9f, 0x44, 0x86, 0x1c, 0x7a, 0x82, 0xc7}});
ConfigBandCache cfg_band_cache(GUID{0x76e74192, 0x6932, 0x2671, {0x90, 0x12, 0xcf, 0x18, 0xca, 0x02, 0x06, 0xe0}});

constexpr auto default_toolbar_width = 21;
constexpr auto default_toolbar_height = 21;

void destroy_rebar(bool save_config)
{
    if (g_rebar_window) {
        g_rebar_window->destroy();
        if (save_config) {
            g_cfg_rebar.set_rebar_info(g_rebar_window->get_band_states());
            cfg_band_cache.set_band_cache(g_rebar_window->cache);
        }
        delete g_rebar_window;
        g_rebar_window = nullptr;
        g_rebar = nullptr;
    }
}

void create_rebar()
{
    if (cfg_toolbars) {
        if (!g_rebar_window) {
            g_rebar_window = new RebarWindow;
            cfg_band_cache.get_band_cache(g_rebar_window->cache);
            g_rebar = g_rebar_window->init();
            if (!g_rebar) {
                delete g_rebar_window;
                g_rebar_window = nullptr;
            }
        }
    } else
        destroy_rebar();
}

void ConfigRebar::export_config(
    stream_writer* p_out, uint32_t mode, fcl::t_export_feedback& feedback, abort_callback& p_abort)
{
    enum {
        stream_version = 0
    };
    p_out->write_lendian_t((uint32_t)stream_version, p_abort);

    if (g_rebar_window) {
        g_rebar_window->refresh_band_configs();
        set_rebar_info(g_rebar_window->get_band_states());
    }

    size_t count = m_entries.size();
    p_out->write_lendian_t(gsl::narrow<uint32_t>(count), p_abort);
    for (size_t i = 0; i < count; i++) {
        feedback.add_required_panel(m_entries[i].m_guid);
        m_entries[i].export_to_fcl_stream(p_out, mode, p_abort);
    }
}

void ConfigRebar::import_config(
    stream_reader* p_reader, size_t size, uint32_t mode, pfc::list_base_t<GUID>& panels, abort_callback& p_abort)
{
    uint32_t version;
    std::vector<RebarBandState> new_entries;
    p_reader->read_lendian_t(version, p_abort);
    if (version > 0)
        throw exception_io_unsupported_format();
    const auto count = p_reader->read_lendian_t<uint32_t>(p_abort);
    for (size_t i = 0; i < count; i++) {
        RebarBandState item;
        item.import_from_fcl_stream(p_reader, mode, p_abort);

        uie::window_ptr ptr;
        if (!uie::window::create_by_guid(item.m_guid, ptr))
            panels.add_item(item.m_guid);
        new_entries.push_back(std::move(item));
    }
    if (main_window.get_wnd())
        destroy_rebar();

    m_entries = new_entries;
    cfg_band_cache.reset();

    if (main_window.get_wnd()) {
        create_rebar();
        if (g_rebar) {
            ShowWindow(g_rebar, SW_SHOWNORMAL);
            UpdateWindow(g_rebar);
        }
        main_window.resize_child_windows();
    }
}

void BandCache::add_entry(const GUID& guid, unsigned width)
{
    const auto count = get_count();
    for (size_t n = 0; n < count; n++) {
        BandCacheEntry& p_bce = (*this)[n];
        if (p_bce.guid == guid) {
            p_bce.width = width;
            return;
        }
    }
    add_item({guid, width});
}

unsigned BandCache::get_width(const GUID& guid)
{
    unsigned rv = 100;
    const auto count = get_count();
    for (size_t n = 0; n < count; n++) {
        const auto& p_bce = get_item_ref(n);
        if (p_bce.guid == guid) {
            rv = p_bce.width;
        }
    }
    return rv;
}

void BandCache::write(stream_writer* out, abort_callback& p_abort)
{
    const auto count = get_count();
    out->write_lendian_t(gsl::narrow<uint32_t>(count), p_abort);
    for (size_t n = 0; n < count; n++) {
        const auto& p_bce = get_item_ref(n);
        out->write_lendian_t(p_bce.guid, p_abort);
        out->write_lendian_t(p_bce.width, p_abort);
    }
}

void BandCache::read(stream_reader* data, abort_callback& p_abort)
{
    remove_all();
    unsigned count;
    data->read_lendian_t(count, p_abort);
    for (unsigned n = 0; n < count; n++) {
        GUID guid;
        unsigned width;
        data->read_lendian_t(guid, p_abort);
        data->read_lendian_t(width, p_abort);
        BandCacheEntry item;
        item.guid = guid;
        item.width = width;
        add_item(item);
    }
}

void ConfigBandCache::get_data_raw(stream_writer* out, abort_callback& p_abort)
{
    if (g_rebar_window)
        entries.copy(g_rebar_window->cache);
    return entries.write(out, p_abort);
}

void ConfigBandCache::set_data_raw(stream_reader* p_reader, size_t p_sizehint, abort_callback& p_abort)
{
    return entries.read(p_reader, p_abort);
}

void ConfigBandCache::get_band_cache(BandCache& out)
{
    out.remove_all();
    out.add_items(entries);
}

void ConfigBandCache::set_band_cache(BandCache& in)
{
    entries.remove_all();
    entries.add_items(in);
}

void ConfigBandCache::reset()
{
    entries.remove_all();
}

void ConfigRebar::get_data_raw(stream_writer* out, abort_callback& p_abort)
{
    if (g_rebar_window) {
        g_rebar_window->refresh_band_configs();
        set_rebar_info(g_rebar_window->get_band_states());
    }

    auto num = gsl::narrow<uint32_t>(m_entries.size());
    out->write_lendian_t(num, p_abort);
    for (uint32_t n = 0; n < num; n++) {
        m_entries[n].write_to_stream(out, p_abort);
    }

    // Extra data added in version 0.5.0
    out->write_lendian_t(static_cast<uint32_t>(StreamVersion::VersionCurrent), p_abort);

    for (size_t n = 0; n < num; n++) {
        stream_writer_memblock extraData;
        m_entries[n].write_extra(&extraData, p_abort);
        out->write_lendian_t(gsl::narrow<uint32_t>(extraData.m_data.get_size()), p_abort);
        out->write(extraData.m_data.get_ptr(), extraData.m_data.get_size(), p_abort);
    }
}

void ConfigRebar::set_data_raw(stream_reader* p_reader, size_t p_sizehint, abort_callback& p_abort)
{
    m_entries.clear();

    uint32_t itemCount;
    p_reader->read_lendian_t(itemCount, p_abort);

    for (uint32_t i = 0; i < itemCount; i++) {
        RebarBandState item;
        item.read_from_stream(p_reader, p_abort);
        m_entries.push_back(std::move(item));
    }

    // Extra data added in version 0.5.0
    StreamVersion streamVersion = StreamVersion::Version0;
    try {
        uint32_t streamVersion_;
        p_reader->read_lendian_t(streamVersion_, p_abort);
        streamVersion = static_cast<StreamVersion>(streamVersion_);
    } catch (const exception_io_data_truncation&) {
    }

    if (streamVersion >= StreamVersion::Version1) {
        for (uint32_t i = 0; i < itemCount; i++) {
            uint32_t extraDataSize;
            p_reader->read_lendian_t(extraDataSize, p_abort);
            pfc::array_staticsize_t<uint8_t> columnExtraData(extraDataSize);
            p_reader->read(columnExtraData.get_ptr(), columnExtraData.get_size(), p_abort);
            stream_reader_memblock_ref columnReader(columnExtraData);
            m_entries[i].read_extra(&columnReader, p_abort);
        }
    }
}

void ConfigRebar::reset()
{
    m_entries = {
        {toolbars::guid_menu, 9999},
        {toolbars::guid_buttons, 100, true},
        {toolbars::guid_seek_bar, 9999},
        {toolbars::guid_playback_order, 100},
        {toolbars::guid_spectrum_analyser, 125},
    };
}

// {3D3C8D68-3AB9-4ad5-A4FA-22427ABAEBF4}
static const GUID rebar_guid = {0x3d3c8d68, 0x3ab9, 0x4ad5, {0xa4, 0xfa, 0x22, 0x42, 0x7a, 0xba, 0xeb, 0xf4}};

class RebarWindowHost : public ui_extension::window_host_with_control {
public:
    void get_name(pfc::string_base& out) const override { out.set_string("Columns UI/Toolbars"); }

    bool is_available() const override { return g_rebar_window != nullptr; }

    unsigned get_supported_types() const override { return ui_extension::type_toolbar; }

    void insert_extension(const GUID& in, unsigned height, unsigned width) override
    {
        if (g_rebar_window) {
            g_rebar_window->add_band(in, width);
        }
    }

    void insert_extension(ui_extension::window_ptr& p_ext, unsigned height, unsigned width) override
    {
        if (g_rebar_window) {
            g_rebar_window->add_band(p_ext->get_extension_guid(), width, p_ext);
        }
    }

    unsigned is_resize_supported(HWND wnd) const override { return ui_extension::size_width; }

    bool request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height) override
    {
        if ((flags & ui_extension::size_width) && !(flags & ui_extension::size_height) && g_rebar_window) {
            auto iterator = g_rebar_window->find_band_by_hwnd(wnd);
            if (iterator != g_rebar_window->m_bands.end()) {
                const auto index = std::distance(g_rebar_window->m_bands.begin(), iterator);
                (*iterator)->m_state.m_width = width;
                g_rebar_window->update_band(index, true);
                return true;
            }
        }
        return false;
    }

    bool is_visible(HWND wnd) const override { return true; }

    bool is_visibility_modifiable(HWND wnd, bool desired_visibility) const override { return false; }

    bool set_window_visibility(HWND wnd, bool visibility) override { return false; }

    void on_size_limit_change(HWND wnd, unsigned flags) override
    {
        if (g_rebar_window) {
            auto iterator = g_rebar_window->find_band_by_hwnd(wnd);
            if (iterator != g_rebar_window->m_bands.end()) {
                const auto index = std::distance(g_rebar_window->m_bands.begin(), iterator);
                g_rebar_window->update_band(index);
            }
        }
    }

    const GUID& get_host_guid() const override { return rebar_guid; }

    bool override_status_text_create(service_ptr_t<ui_status_text_override>& p_out) override
    {
        const auto api = ui_control::get();
        return api->override_status_text_create(p_out);
    }

    virtual bool on_key(UINT msg, LPARAM lp, WPARAM wp, bool process_keyboard_shortcuts)
    {
        return process_keydown(msg, lp, wp, false, process_keyboard_shortcuts);
    }

    void relinquish_ownership(HWND wnd) override
    {
        if (g_rebar_window) {
            g_rebar_window->delete_band(wnd, false);
        }
    }
};

ui_extension::window_host_factory_single<RebarWindowHost> g_ui_ext_host_rebar;

HWND RebarWindow::init()
{
    auto& band_states = g_cfg_rebar.get_rebar_info();

    m_bands = band_states
        | ranges::views::transform([](auto&& band_state) { return std::make_shared<RebarBand>(band_state); })
        | ranges::to<std::vector>();

    if (!wnd_rebar) {
        RECT main_window_client_rect{};
        GetClientRect(main_window.get_wnd(), &main_window_client_rect);

        wnd_rebar = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_CONTROLPARENT, REBARCLASSNAME, nullptr,
            WS_BORDER | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | RBS_VARHEIGHT | RBS_DBLCLKTOGGLE | RBS_AUTOSIZE
                | RBS_BANDBORDERS | CCS_NODIVIDER | CCS_NOPARENTALIGN | 0,
            0, 0, wil::rect_width(main_window_client_rect), 0, main_window.get_wnd(),
            reinterpret_cast<HMENU>(static_cast<size_t>(ID_REBAR)), wil::GetModuleInstanceHandle(), nullptr);

        if (!wnd_rebar)
            return nullptr;

        SetWindowLongPtr(wnd_rebar, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        m_rebar_wnd_proc = reinterpret_cast<WNDPROC>(
            SetWindowLongPtr(wnd_rebar, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(s_handle_hooked_message)));

        m_dark_mode_notifier = std::make_unique<colours::dark_mode_notifier>([this] { on_themechanged(); });
    }

    refresh_bands();

    return wnd_rebar;
}

void RebarWindow::refresh_band_configs()
{
    for (auto&& band : m_bands) {
        if (band->m_wnd && band->m_window.is_valid()) {
            try {
                abort_callback_dummy aborter;
                stream_writer_memblock_ref writer(band->m_state.m_config);
                band->m_state.m_config.set_size(0);
                band->m_window->get_config(&writer, aborter);
            } catch (const exception_io&) {
            }
        }
    }
}

bool RebarWindow::on_menu_char(unsigned short c)
{
    bool rv = false;
    for (auto&& band : m_bands) {
        if (band->m_window.is_valid() && band->m_wnd) {
            service_ptr_t<uie::menu_window> p_menu_ext;
            if (band->m_window->service_query_t(p_menu_ext)) {
                rv = p_menu_ext->on_menuchar(c);
                if (rv)
                    break;
            }
        }
    }
    return rv;
}

void RebarWindow::show_accelerators()
{
    for (auto&& band : m_bands) {
        if (band->m_window.is_valid() && band->m_wnd) {
            service_ptr_t<uie::menu_window> p_menu_ext;
            if (band->m_window->service_query_t(p_menu_ext)) {
                p_menu_ext->show_accelerators();
            }
        }
    }
}

void RebarWindow::hide_accelerators()
{
    for (auto&& band : m_bands) {
        if (band->m_window.is_valid() && band->m_wnd) {
            service_ptr_t<uie::menu_window> p_menu_ext;
            if (band->m_window->service_query_t(p_menu_ext)) {
                p_menu_ext->hide_accelerators();
            }
        }
    }
}

bool RebarWindow::is_menu_focused()
{
    for (auto&& band : m_bands) {
        if (band->m_window.is_valid() && band->m_wnd) {
            service_ptr_t<uie::menu_window> p_menu_ext;
            if (band->m_window->service_query_t(p_menu_ext)) {
                if (p_menu_ext->is_menu_focused())
                    return true;
            }
        }
    }
    return false;
}

bool RebarWindow::get_previous_menu_focus_window(HWND& wnd_previous) const
{
    for (auto&& band : m_bands) {
        if (band->m_window.is_valid() && band->m_wnd) {
            service_ptr_t<uie::menu_window_v2> p_menu_ext;
            if (band->m_window->service_query_t(p_menu_ext)) {
                if (p_menu_ext->is_menu_focused()) {
                    wnd_previous = p_menu_ext->get_previous_focus_window();
                    return true;
                }
            }
        }
    }
    return false;
}

bool RebarWindow::set_menu_focus()
{
    bool rv = false;

    for (auto&& band : m_bands) {
        if (band->m_window.is_valid() && band->m_wnd) {
            service_ptr_t<uie::menu_window> p_menu_ext;
            if (band->m_window->service_query_t(p_menu_ext)) {
                if (!rv) {
                    p_menu_ext->set_focus();
                    rv = true;
                } else {
                    p_menu_ext->hide_accelerators();
                }
            }
        }
    }
    return rv;
}

void RebarWindow::on_themechanged()
{
    SetWindowPos(wnd_rebar, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOZORDER | SWP_NOSIZE | SWP_FRAMECHANGED);
    RedrawWindow(wnd_rebar, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
}

std::optional<LRESULT> RebarWindow::handle_custom_draw(const LPNMCUSTOMDRAW lpnmcd) const
{
    switch (lpnmcd->dwDrawStage) {
    case CDDS_PREERASE: {
        if (!colours::is_dark_mode_active())
            return {};

        const auto brush = get_colour_brush(dark::ColourID::RebarBackground, true);

        RECT rc{};
        GetClientRect(lpnmcd->hdr.hwndFrom, &rc);
        FillRect(lpnmcd->hdc, &rc, brush.get());

        const int row_count = gsl::narrow<int>(SendMessage(lpnmcd->hdr.hwndFrom, RB_GETROWCOUNT, 0, 0));

        const auto divider_brush = get_colour_brush_lazy(dark::ColourID::RebarBandBorder, true);
        const auto divider_width = uih::scale_dpi_value(1, USER_DEFAULT_SCREEN_DPI * 2);

        int row_index{};
        int row_bottom{};

        for (auto&& [band_index, band] : ranges::views::enumerate(m_bands)) {
            if (band_index == 0 || band->m_state.m_break_before_band) {
                const int row_height
                    = gsl::narrow<int>(SendMessage(lpnmcd->hdr.hwndFrom, RB_GETROWHEIGHT, band_index, 0));
                row_bottom += row_height;

                ++row_index;

                if (row_index < row_count) {
                    RECT row_divider = {rc.left, row_bottom, rc.right, row_bottom + divider_width};
                    FillRect(lpnmcd->hdc, &row_divider, *divider_brush);
                }

                continue;
            }

            RECT band_rect{};
            SendMessage(lpnmcd->hdr.hwndFrom, RB_GETRECT, band_index, reinterpret_cast<LPARAM>(&band_rect));

            MARGINS margins{};
            SendMessage(lpnmcd->hdr.hwndFrom, RB_GETBANDMARGINS, band_index, reinterpret_cast<LPARAM>(&margins));

            const auto left = band_rect.left - margins.cxLeftWidth;
            RECT band_divider = {left, band_rect.top, left + divider_width, band_rect.bottom};
            FillRect(lpnmcd->hdc, &band_divider, *divider_brush);
        }
        return CDRF_SKIPDEFAULT;
    }
    }
    return {};
}

void RebarWindow::save_bands()
{
    REBARBANDINFO rbbi{};

    rbbi.cbSize = REBARBANDINFOW_V6_SIZE;
    rbbi.fMask = RBBIM_SIZE | RBBIM_STYLE | RBBIM_LPARAM;

    const auto band_count = m_bands.size();
    mmh::Permutation order(band_count);

    const auto count = static_cast<UINT>(SendMessage(wnd_rebar, RB_GETBANDCOUNT, 0, 0));

    if (count == 0 || band_count != count)
        return;

    bool band_error_occurred{};

    for (uint32_t n = 0; n < count; n++) {
        if (!SendMessage(wnd_rebar, RB_GETBANDINFO, n, reinterpret_cast<LPARAM>(&rbbi))) {
            band_error_occurred = true;
            continue;
        }

        const auto band = reinterpret_cast<RebarBand*>(rbbi.lParam);
        const auto iter = ranges::find_if(m_bands, [band](auto&& item) { return item.get() == band; });

        if (iter == m_bands.end()) {
            band_error_occurred = true;
            continue;
        }

        order[n] = std::distance(m_bands.begin(), iter);
        band->m_state.m_width = rbbi.cx;
        band->m_state.m_break_before_band = ((rbbi.fStyle & RBBS_BREAK) != 0);
    }

    if (!band_error_occurred)
        destructive_reorder(m_bands, order);

    refresh_bands();
}

bool RebarWindow::check_band(const GUID& id)
{
    return std::find_if(m_bands.begin(), m_bands.end(), [&id](auto&& band) { return band->m_state.m_guid == id; })
        != m_bands.end();
}

bool RebarWindow::find_band(const GUID& id, size_t& out)
{
    const auto iterator
        = std::find_if(m_bands.begin(), m_bands.end(), [&id](auto&& band) { return band->m_state.m_guid == id; });

    out = std::distance(m_bands.begin(), iterator);
    return iterator != m_bands.end();
}

bool RebarWindow::delete_band(const GUID& id)
{
    size_t n = 0;
    bool rv = find_band(id, n);
    if (rv)
        delete_band(n);
    return rv;
}

void RebarWindow::destroy_bands()
{
    for (auto&& band : m_bands) {
        if (!band->m_window.is_valid())
            continue;

        band->m_state.m_config.set_size(0);
        stream_writer_memblock_ref data(band->m_state.m_config);
        try {
            band->m_window->get_config(&data, fb2k::noAbort);
        } catch (const pfc::exception&) {
        }
        band->m_window->destroy_window();
        band->m_wnd = nullptr;
        band->m_window.release();
    }
}

void RebarWindow::destroy()
{
    m_dark_mode_notifier.reset();
    destroy_bands();
    DestroyWindow(wnd_rebar);
    wnd_rebar = nullptr;
}

void RebarWindow::update_bands()
{
    refresh_bands();
    uih::rebar_show_all_bands(wnd_rebar);
}

void RebarWindow::delete_band(size_t n)
{
    if (n < m_bands.size()) {
        SendMessage(wnd_rebar, RB_SHOWBAND, n, FALSE);
        SendMessage(wnd_rebar, RB_DELETEBAND, n, 0);
        ui_extension::window_ptr p_ext = m_bands[n]->m_window;
        if (p_ext.is_valid()) {
            p_ext->destroy_window();
            p_ext.release();
        }
        cache.add_entry(m_bands[n]->m_state.m_guid, m_bands[n]->m_state.m_width);
        m_bands.erase(m_bands.begin() + n);
        refresh_bands();
    }
}

void RebarWindow::delete_band(HWND wnd, bool destroy)
{
    auto iter = g_rebar_window->find_band_by_hwnd(wnd);
    if (iter != m_bands.end()) {
        auto index = std::distance(m_bands.begin(), iter);
        SendMessage(wnd_rebar, RB_SHOWBAND, index, FALSE);
        SendMessage(wnd_rebar, RB_DELETEBAND, index, 0);

        const auto band = *iter;

        if (band->m_window.is_valid()) {
            if (destroy)
                band->m_window->destroy_window();
        }

        cache.add_entry(band->m_state.m_guid, band->m_state.m_width);
        m_bands.erase(iter);
        refresh_bands();
    }
}

std::vector<RebarBandState> RebarWindow::get_band_states() const
{
    return m_bands | ranges::views::transform([](auto&& band) { return band->m_state; }) | ranges::to<std::vector>();
}

void RebarWindow::add_band(const GUID& guid, unsigned width, const ui_extension::window_ptr& p_ext)
{
    m_bands.emplace_back(std::make_shared<RebarBand>(RebarBandState{guid, width}, p_ext));
    refresh_bands();
}

void RebarWindow::insert_band(unsigned idx, const GUID& guid, unsigned width, const ui_extension::window_ptr& p_ext)
{
    m_bands.emplace(m_bands.begin() + idx, std::make_shared<RebarBand>(RebarBandState{guid, width}, p_ext));
    refresh_bands();
}

void RebarWindow::update_band(size_t n, bool size)
{
    ui_extension::window_ptr p_ext = m_bands[n]->m_window;
    if (p_ext.is_valid()) {
        uREBARBANDINFO rbbi{};
        rbbi.cbSize = sizeof(uREBARBANDINFO);

        rbbi.fMask |= RBBIM_CHILDSIZE;

        MINMAXINFO mmi{};
        mmi.ptMaxTrackSize.x = MAXLONG;
        mmi.ptMaxTrackSize.y = MAXLONG;
        SendMessage(m_bands[n]->m_wnd, WM_GETMINMAXINFO, 0, reinterpret_cast<LPARAM>(&mmi));

        if (mmi.ptMaxTrackSize.y < 0)
            mmi.ptMaxTrackSize.y = 0;
        if (mmi.ptMinTrackSize.y <= 0)
            mmi.ptMinTrackSize.y
                = std::min(static_cast<long>(uih::scale_dpi_value(default_toolbar_height)), mmi.ptMaxTrackSize.y);
        if (mmi.ptMinTrackSize.x <= 0)
            mmi.ptMinTrackSize.x = uih::scale_dpi_value(default_toolbar_width);

        rbbi.cyMinChild = mmi.ptMinTrackSize.y;
        rbbi.cyMaxChild = mmi.ptMaxTrackSize.y;
        rbbi.cxMinChild = mmi.ptMinTrackSize.x;

        if (size) {
            rbbi.fMask |= RBBIM_SIZE;
            rbbi.cx = m_bands[n]->m_state.m_width;
        }

        uRebar_InsertItem(wnd_rebar, gsl::narrow<int>(n), &rbbi, false);
        SendMessage(wnd_rebar, RB_SHOWBAND, n, TRUE);

        fix_z_order();
    }
}

void RebarWindow::refresh_bands()
{
    auto _ = wil::scope_exit(
        [&, previous_value{m_refresh_children_in_progress}] { m_refresh_children_in_progress = previous_value; });

    m_refresh_children_in_progress = true;

    abort_callback_dummy aborter;

    auto& host = g_ui_ext_host_rebar.get_static_instance();

    // Create a copy to protect against window_host::relinquish_ownership() calls
    // during iteration
    const auto bands = m_bands;

    for (auto&& band : bands) {
        bool adding = false;

        uREBARBANDINFO rbbi{};
        rbbi.cbSize = sizeof(uREBARBANDINFO);

        if (!band->m_wnd) {
            ui_extension::window_ptr p_ext = band->m_window;
            bool b_new = false;
            if (!p_ext.is_valid()) {
                ui_extension::window::create_by_guid(band->m_state.m_guid, p_ext);
                b_new = true;
            }

            if (p_ext.is_valid() && p_ext->is_available(&host)) {
                adding = true;
                if (b_new) {
                    try {
                        p_ext->set_config_from_ptr(
                            band->m_state.m_config.get_ptr(), band->m_state.m_config.get_size(), aborter);
                    } catch (const exception_io& e) {
                        console::formatter formatter;
                        formatter << "Error setting panel config: " << e.what();
                    }
                }
                band->m_wnd = p_ext->create_or_transfer_window(wnd_rebar, &host);
                if (band->m_wnd) {
                    band->m_window = p_ext;
                    ShowWindow(band->m_wnd, SW_SHOWNORMAL);

                    rbbi.fMask |= RBBIM_CHILDSIZE;

                    MINMAXINFO mmi{};
                    mmi.ptMaxTrackSize.x = MAXLONG;
                    mmi.ptMaxTrackSize.y = MAXLONG;
                    SendMessage(band->m_wnd, WM_GETMINMAXINFO, 0, reinterpret_cast<LPARAM>(&mmi));

                    if (mmi.ptMaxTrackSize.y < 0)
                        mmi.ptMaxTrackSize.y = 0;
                    if (mmi.ptMinTrackSize.y <= 0)
                        mmi.ptMinTrackSize.y = std::min(
                            static_cast<long>(uih::scale_dpi_value(default_toolbar_height)), mmi.ptMaxTrackSize.y);
                    if (mmi.ptMinTrackSize.x <= 0)
                        mmi.ptMinTrackSize.x = uih::scale_dpi_value(default_toolbar_width);

                    rbbi.cyMinChild = mmi.ptMinTrackSize.y;
                    rbbi.cyMaxChild = mmi.ptMaxTrackSize.y;
                    rbbi.cxMinChild = mmi.ptMinTrackSize.x;
                }
            }
        }

        if (band->m_wnd) {
            rbbi.fMask |= RBBIM_SIZE | RBBIM_CHILD | RBBIM_HEADERSIZE | RBBIM_LPARAM | RBBIM_STYLE;
            rbbi.cx = band->m_state.m_width;
            rbbi.fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS | (band->m_state.m_break_before_band ? RBBS_BREAK : 0)
                | (cfg_lock ? RBBS_NOGRIPPER : 0);
            rbbi.lParam = reinterpret_cast<LPARAM>(band.get());
            rbbi.hwndChild = band->m_wnd;
            rbbi.cxHeader = cfg_lock ? 5 : 9;

            const auto index = std::distance(m_bands.begin(), ranges::find(m_bands, band));
            uRebar_InsertItem(wnd_rebar, gsl::narrow<int>(index), &rbbi, adding);
        } else {
            std::erase(m_bands, band);
        }
    }

    fix_z_order();
}

LRESULT RebarWindow::s_handle_hooked_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) noexcept
{
    const auto self = reinterpret_cast<RebarWindow*>(GetWindowLongPtr(wnd, GWLP_USERDATA));
    return self ? self->handle_hooked_message(wnd, msg, wp, lp) : DefWindowProc(wnd, msg, wp, lp);
}

LRESULT RebarWindow::handle_hooked_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (const auto result = uih::handle_subclassed_window_buffered_painting(m_rebar_wnd_proc, wnd, msg, wp, lp);
        result) {
        return *result;
    }

    switch (msg) {
    case WM_THEMECHANGED:
        on_themechanged();
        break;
    }
    return CallWindowProc(m_rebar_wnd_proc, wnd, msg, wp, lp);
}

void RebarWindow::fix_z_order()
{
    const auto dwp = BeginDeferWindowPos(gsl::narrow<int>(m_bands.size()));
    for (auto&& band : m_bands) {
        if (band->m_wnd)
            DeferWindowPos(dwp, band->m_wnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    EndDeferWindowPos(dwp);
}

ui_extension::window_host& get_rebar_host()
{
    return g_ui_ext_host_rebar.get_static_instance();
}

} // namespace cui::rebar
