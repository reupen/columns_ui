#include "stdafx.h"
#include "rebar.h"
#include "main_window.h"

// extern HBITMAP buttons_images;

extern rebar_window* g_rebar_window;

extern cfg_rebar g_cfg_rebar;
extern cfg_band_cache_t cfg_band_cache;

constexpr const auto default_toolbar_width = 21;
constexpr const auto default_toolbar_height = 21;

void destroy_rebar(bool save_config)
{
    if (g_rebar_window) {
        g_rebar_window->destroy();
        if (save_config) {
            g_cfg_rebar.set_rebar_info(g_rebar_window->get_bands());
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
            g_rebar_window = new rebar_window;
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

void cfg_rebar::export_config(
    stream_writer* p_out, t_uint32 mode, cui::fcl::t_export_feedback& feedback, abort_callback& p_abort)
{
    enum { stream_version = 0 };
    p_out->write_lendian_t((t_uint32)stream_version, p_abort);

    if (g_rebar_window) {
        g_rebar_window->refresh_band_configs();
        m_entries = g_rebar_window->get_bands();
    }

    t_size count = m_entries.size();
    p_out->write_lendian_t(count, p_abort);
    for (t_size i = 0; i < count; i++) {
        feedback.add_required_panel(m_entries[i].m_guid);
        m_entries[i].export_to_fcl_stream(p_out, mode, p_abort);
    }
}

void cfg_rebar::import_config(
    stream_reader* p_reader, t_size size, t_uint32 mode, pfc::list_base_t<GUID>& panels, abort_callback& p_abort)
{
    t_uint32 version;
    std::vector<RebarBandInfo> new_entries;
    p_reader->read_lendian_t(version, p_abort);
    if (version > 0)
        throw exception_io_unsupported_format();
    t_size count;
    p_reader->read_lendian_t(count, p_abort);
    for (t_size i = 0; i < count; i++) {
        RebarBandInfo item;
        item.import_from_fcl_stream(p_reader, mode, p_abort);

        uie::window_ptr ptr;
        if (!uie::window::create_by_guid(item.m_guid, ptr))
            panels.add_item(item.m_guid);
        new_entries.push_back(std::move(item));
    }
    if (cui::main_window.get_wnd())
        destroy_rebar();

    m_entries = new_entries;
    cfg_band_cache.reset();

    if (cui::main_window.get_wnd()) {
        create_rebar();
        if (g_rebar) {
            ShowWindow(g_rebar, SW_SHOWNORMAL);
            UpdateWindow(g_rebar);
        }
        cui::main_window.resize_child_windows();
    }
}

void band_cache::add_entry(const GUID& guid, unsigned width)
{
    unsigned count = get_count();
    for (unsigned n = 0; n < count; n++) {
        band_cache_entry& p_bce = (*this)[n];
        if (p_bce.guid == guid) {
            p_bce.width = width;
            return;
        }
    }
    add_item({guid, width});
}

unsigned band_cache::get_width(const GUID& guid)
{
    unsigned rv = 100;
    unsigned count = get_count();
    for (unsigned n = 0; n < count; n++) {
        const auto& p_bce = get_item_ref(n);
        if (p_bce.guid == guid) {
            rv = p_bce.width;
        }
    }
    return rv;
}

void band_cache::write(stream_writer* out, abort_callback& p_abort)
{
    unsigned count = get_count();
    out->write_lendian_t(count, p_abort);
    for (unsigned n = 0; n < count; n++) {
        const auto& p_bce = get_item_ref(n);
        out->write_lendian_t(p_bce.guid, p_abort);
        out->write_lendian_t(p_bce.width, p_abort);
    }
}

void band_cache::read(stream_reader* data, abort_callback& p_abort)
{
    remove_all();
    unsigned count;
    data->read_lendian_t(count, p_abort);
    for (unsigned n = 0; n < count; n++) {
        GUID guid;
        unsigned width;
        data->read_lendian_t(guid, p_abort);
        data->read_lendian_t(width, p_abort);
        band_cache_entry item;
        item.guid = guid;
        item.width = width;
        add_item(item);
    }
}

void cfg_band_cache_t::get_data_raw(stream_writer* out, abort_callback& p_abort)
{
    if (g_rebar_window)
        entries.copy(g_rebar_window->cache);
    return entries.write(out, p_abort);
}

void cfg_band_cache_t::set_data_raw(stream_reader* p_reader, unsigned p_sizehint, abort_callback& p_abort)
{
    return entries.read(p_reader, p_abort);
}

void cfg_band_cache_t::get_band_cache(band_cache& out)
{
    out.remove_all();
    out.add_items(entries);
}

void cfg_band_cache_t::set_band_cache(band_cache& in)
{
    entries.remove_all();
    entries.add_items(in);
}

void cfg_band_cache_t::reset()
{
    entries.remove_all();
}

void cfg_rebar::get_data_raw(stream_writer* out, abort_callback& p_abort)
{
    if (g_rebar_window) {
        g_rebar_window->refresh_band_configs();
        m_entries = g_rebar_window->get_bands();
    }

    auto num = gsl::narrow<uint32_t>(m_entries.size());
    out->write_lendian_t(num, p_abort);
    for (uint32_t n = 0; n < num; n++) {
        m_entries[n].write_to_stream(out, p_abort);
    }

    // Extra data added in version 0.5.0
    out->write_lendian_t(static_cast<uint32_t>(StreamVersion::VersionCurrent), p_abort);

    for (t_size n = 0; n < num; n++) {
        stream_writer_memblock extraData;
        m_entries[n].write_extra(&extraData, p_abort);
        out->write_lendian_t(gsl::narrow<uint32_t>(extraData.m_data.get_size()), p_abort);
        out->write(extraData.m_data.get_ptr(), extraData.m_data.get_size(), p_abort);
    }
}

void cfg_rebar::set_data_raw(stream_reader* p_reader, unsigned p_sizehint, abort_callback& p_abort)
{
    m_entries.clear();

    uint32_t itemCount;
    p_reader->read_lendian_t(itemCount, p_abort);

    for (uint32_t i = 0; i < itemCount; i++) {
        RebarBandInfo item;
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
            pfc::array_staticsize_t<t_uint8> columnExtraData(extraDataSize);
            p_reader->read(columnExtraData.get_ptr(), columnExtraData.get_size(), p_abort);
            stream_reader_memblock_ref columnReader(columnExtraData);
            m_entries[i].read_extra(&columnReader, p_abort);
        }
    }
}

void cfg_rebar::reset()
{
    m_entries = {
        {cui::toolbars::guid_menu, 9999},
        {cui::toolbars::guid_buttons, 100, true},
        {cui::toolbars::guid_seek_bar, 9999},
        {cui::toolbars::guid_playback_order, 100},
        {cui::toolbars::guid_spectrum_analyser, 125},
    };
}

// {3D3C8D68-3AB9-4ad5-A4FA-22427ABAEBF4}
static const GUID rebar_guid = {0x3d3c8d68, 0x3ab9, 0x4ad5, {0xa4, 0xfa, 0x22, 0x42, 0x7a, 0xba, 0xeb, 0xf4}};

class ui_ext_host_rebar : public ui_extension::window_host_with_control {
public:
    void get_name(pfc::string_base& out) const override { out.set_string("Columns UI/Toolbars"); };

    bool is_available() const override { return g_rebar_window != nullptr; }

    unsigned get_supported_types() const override { return ui_extension::type_toolbar; }

    void insert_extension(const GUID& in, unsigned height, unsigned width) override
    {
        if (g_rebar_window) {
            g_rebar_window->add_band(in, width);
        }
    };

    void insert_extension(ui_extension::window_ptr& p_ext, unsigned height, unsigned width) override
    {
        if (g_rebar_window) {
            g_rebar_window->add_band(p_ext->get_extension_guid(), width, p_ext);
        }
    };

    unsigned is_resize_supported(HWND wnd) const override { return ui_extension::size_width; }

    bool request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height) override
    {
        if ((flags & ui_extension::size_width) && !(flags & ui_extension::size_height) && g_rebar_window) {
            auto iterator = g_rebar_window->find_band_by_hwnd(wnd);
            if (iterator != g_rebar_window->m_bands.end()) {
                const auto index = std::distance(g_rebar_window->m_bands.begin(), iterator);
                iterator->m_width = width;
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
    };

    const GUID& get_host_guid() const override { return rebar_guid; }

    bool override_status_text_create(service_ptr_t<ui_status_text_override>& p_out) override
    {
        static_api_ptr_t<ui_control> api;
        return api->override_status_text_create(p_out);
    }

    virtual bool on_key(UINT msg, LPARAM lp, WPARAM wp, bool process_keyboard_shortcuts)
    {
        return process_keydown(msg, lp, wp, false, process_keyboard_shortcuts);
    };

    void relinquish_ownership(HWND wnd) override
    {
        if (g_rebar_window) {
            g_rebar_window->delete_band(wnd, false);
        }
    };
};

ui_extension::window_host_factory_single<ui_ext_host_rebar> g_ui_ext_host_rebar;

HWND rebar_window::init()
{
    HWND rv = nullptr;

    m_bands = g_cfg_rebar.get_rebar_info();

    if (!wnd_rebar) {
        rv = wnd_rebar = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_CONTROLPARENT, REBARCLASSNAME, nullptr,
            WS_BORDER | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | RBS_VARHEIGHT | RBS_DBLCLKTOGGLE | RBS_AUTOSIZE
                | RBS_BANDBORDERS | CCS_NODIVIDER | CCS_NOPARENTALIGN | 0,
            0, 0, 0, 0, cui::main_window.get_wnd(), (HMENU)ID_REBAR, core_api::get_my_instance(), nullptr);
        // SetWindowTheme(wnd_rebar, L"Default", NULL);
    }

    refresh_bands();

    return rv;
}

void rebar_window::refresh_band_configs()
{
    for (auto&& band : m_bands) {
        if (band.m_wnd && band.m_window.is_valid()) {
            try {
                abort_callback_dummy aborter;
                stream_writer_memblock_ref writer(band.m_config);
                band.m_config.set_size(0);
                band.m_window->get_config(&writer, aborter);
            } catch (const exception_io&) {
            }
        }
    }
}

bool rebar_window::on_menu_char(unsigned short c)
{
    bool rv = false;
    for (auto&& band : m_bands) {
        if (band.m_window.is_valid() && band.m_wnd) {
            service_ptr_t<uie::menu_window> p_menu_ext;
            if (band.m_window->service_query_t(p_menu_ext)) {
                rv = p_menu_ext->on_menuchar(c);
                if (rv)
                    break;
            }
        }
    }
    return rv;
}

void rebar_window::show_accelerators()
{
    for (auto&& band : m_bands) {
        if (band.m_window.is_valid() && band.m_wnd) {
            service_ptr_t<uie::menu_window> p_menu_ext;
            if (band.m_window->service_query_t(p_menu_ext)) {
                p_menu_ext->show_accelerators();
            }
        }
    }
}

void rebar_window::hide_accelerators()
{
    for (auto&& band : m_bands) {
        if (band.m_window.is_valid() && band.m_wnd) {
            service_ptr_t<uie::menu_window> p_menu_ext;
            if (band.m_window->service_query_t(p_menu_ext)) {
                p_menu_ext->hide_accelerators();
            }
        }
    }
}

bool rebar_window::is_menu_focused()
{
    for (auto&& band : m_bands) {
        if (band.m_window.is_valid() && band.m_wnd) {
            service_ptr_t<uie::menu_window> p_menu_ext;
            if (band.m_window->service_query_t(p_menu_ext)) {
                if (p_menu_ext->is_menu_focused())
                    return true;
            }
        }
    }
    return false;
}

bool rebar_window::get_previous_menu_focus_window(HWND& wnd_previous) const
{
    for (auto&& band : m_bands) {
        if (band.m_window.is_valid() && band.m_wnd) {
            service_ptr_t<uie::menu_window_v2> p_menu_ext;
            if (band.m_window->service_query_t(p_menu_ext)) {
                if (p_menu_ext->is_menu_focused()) {
                    wnd_previous = p_menu_ext->get_previous_focus_window();
                    return true;
                }
            }
        }
    }
    return false;
}

bool rebar_window::set_menu_focus()
{
    bool rv = false;

    for (auto&& band : m_bands) {
        if (band.m_window.is_valid() && band.m_wnd) {
            service_ptr_t<uie::menu_window> p_menu_ext;
            if (band.m_window->service_query_t(p_menu_ext)) {
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

void rebar_window::on_themechanged()
{
    SetWindowPos(wnd_rebar, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOZORDER | SWP_NOSIZE | SWP_FRAMECHANGED);
    RedrawWindow(wnd_rebar, nullptr, nullptr, RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
}

void rebar_window::save_bands()
{
    REBARBANDINFO rbbi;
    memset(&rbbi, 0, sizeof(rbbi));

    rbbi.cbSize = REBARBANDINFOW_V6_SIZE;
    rbbi.fMask = RBBIM_SIZE | RBBIM_STYLE | RBBIM_LPARAM;

    const auto band_count = m_bands.size();
    mmh::Permutation order(band_count);

    UINT count = SendMessage(wnd_rebar, RB_GETBANDCOUNT, 0, 0);

    bool b_death = false;

    if (count && band_count == count) {
        for (uint32_t n = 0; n < count; n++) {
            BOOL b_OK = SendMessage(wnd_rebar, RB_GETBANDINFO, n, reinterpret_cast<LPARAM>(&rbbi));
            const auto band_index = static_cast<uint32_t>(rbbi.lParam);
            if (b_OK && band_index < count) {
                order[n] = band_index;
                m_bands[band_index].m_width = rbbi.cx;
                m_bands[band_index].m_break_before_band = ((rbbi.fStyle & RBBS_BREAK) != 0);
            } else
                b_death = true;
        }

        if (!b_death)
            mmh::destructive_reorder(m_bands, order);
        refresh_bands(false);
    }
}

bool rebar_window::check_band(const GUID& id)
{
    return std::find_if(m_bands.begin(), m_bands.end(), [&id](auto&& band) { return band.m_guid == id; })
        != m_bands.end();
}

bool rebar_window::find_band(const GUID& id, unsigned& out)
{
    const auto iterator
        = std::find_if(m_bands.begin(), m_bands.end(), [&id](auto&& band) { return band.m_guid == id; });

    out = std::distance(m_bands.begin(), iterator);
    return iterator != m_bands.end();
}

bool rebar_window::delete_band(const GUID& id)
{
    unsigned n = 0;
    bool rv = find_band(id, n);
    if (rv)
        delete_band(n);
    return rv;
}

void rebar_window::destroy_bands()
{
    abort_callback_dummy abortCallbackDummy;

    UINT count = SendMessage(wnd_rebar, RB_GETBANDCOUNT, 0, 0);

    if (count > 0 && count == m_bands.size()) {
        for (auto&& band : m_bands) {
            SendMessage(wnd_rebar, RB_SHOWBAND, 0, FALSE);
            SendMessage(wnd_rebar, RB_DELETEBAND, 0, 0);
            if (band.m_window.is_valid()) {
                band.m_config.set_size(0);
                stream_writer_memblock_ref data(band.m_config);
                try {
                    band.m_window->get_config(&data, abortCallbackDummy);
                } catch (const pfc::exception&) {
                }
                band.m_window->destroy_window();
                band.m_wnd = nullptr;
                band.m_window.release();
            }
        }
    }
}

void rebar_window::destroy()
{
    destroy_bands();
    DestroyWindow(wnd_rebar);
    wnd_rebar = nullptr;
}

void rebar_window::update_bands()
{
    refresh_bands(false);
    uih::rebar_show_all_bands(wnd_rebar);
}

void rebar_window::delete_band(unsigned n)
{
    if (n < m_bands.size()) {
        SendMessage(wnd_rebar, RB_SHOWBAND, n, FALSE);
        SendMessage(wnd_rebar, RB_DELETEBAND, n, 0);
        ui_extension::window_ptr p_ext = m_bands[n].m_window;
        if (p_ext.is_valid()) {
            p_ext->destroy_window();
            p_ext.release();
        }
        cache.add_entry(m_bands[n].m_guid, m_bands[n].m_width);
        m_bands.erase(m_bands.begin() + n);
        refresh_bands(false);
    }
}

void rebar_window::delete_band(HWND wnd, bool destroy)
{
    auto iter = g_rebar_window->find_band_by_hwnd(wnd);
    if (iter != m_bands.end()) {
        auto index = std::distance(m_bands.begin(), iter);
        SendMessage(wnd_rebar, RB_SHOWBAND, index, FALSE);
        SendMessage(wnd_rebar, RB_DELETEBAND, index, 0);
        if (iter->m_window.is_valid()) {
            if (destroy)
                iter->m_window->destroy_window();
        }
        cache.add_entry(iter->m_guid, iter->m_width);
        m_bands.erase(iter);
        refresh_bands(false);
    }
}

void rebar_window::add_band(const GUID& guid, unsigned width, const ui_extension::window_ptr& p_ext)
{
    m_bands.emplace_back(guid, width, false, p_ext);
    refresh_bands(false);
}

void rebar_window::insert_band(unsigned idx, const GUID& guid, unsigned width, const ui_extension::window_ptr& p_ext)
{
    m_bands.emplace(m_bands.begin() + idx, guid, width, false, p_ext);
    refresh_bands(false);
}

void rebar_window::update_band(unsigned n, bool size)
{
    ui_extension::window_ptr p_ext = m_bands[n].m_window;
    if (p_ext.is_valid()) {
        uREBARBANDINFO rbbi;
        memset(&rbbi, 0, sizeof(rbbi));
        rbbi.cbSize = sizeof(uREBARBANDINFO);

        rbbi.fMask |= RBBIM_CHILDSIZE;

        MINMAXINFO mmi;
        memset(&mmi, 0, sizeof(MINMAXINFO));
        mmi.ptMaxTrackSize.x = MAXLONG;
        mmi.ptMaxTrackSize.y = MAXLONG;
        SendMessage(m_bands[n].m_wnd, WM_GETMINMAXINFO, 0, reinterpret_cast<LPARAM>(&mmi));

        if (mmi.ptMaxTrackSize.y < 0)
            mmi.ptMaxTrackSize.y = 0;
        if (mmi.ptMinTrackSize.y <= 0)
            mmi.ptMinTrackSize.y = min(uih::scale_dpi_value(default_toolbar_height), mmi.ptMaxTrackSize.y);
        if (mmi.ptMinTrackSize.x <= 0)
            mmi.ptMinTrackSize.x = uih::scale_dpi_value(default_toolbar_width);

        rbbi.cyMinChild = mmi.ptMinTrackSize.y;
        rbbi.cyMaxChild = mmi.ptMaxTrackSize.y;
        rbbi.cxMinChild = mmi.ptMinTrackSize.x;

        if (size) {
            rbbi.fMask |= RBBIM_SIZE;
            rbbi.cx = m_bands[n].m_width;
        }

        uRebar_InsertItem(wnd_rebar, n, &rbbi, false);
        SendMessage(wnd_rebar, RB_SHOWBAND, n, TRUE);
    }
}

void rebar_window::refresh_bands(bool force_destroy_bands)
{
    abort_callback_dummy abortCallbackDummy;

    if (force_destroy_bands)
        destroy_bands();

    auto count = m_bands.size();
    for (auto n = 0u; n < count;) {
        auto& band = m_bands[n];
        bool adding = false;

        uREBARBANDINFO rbbi;
        memset(&rbbi, 0, sizeof(rbbi));
        rbbi.cbSize = sizeof(uREBARBANDINFO);

        if (!band.m_wnd) {
            ui_extension::window_ptr p_ext = band.m_window;
            bool b_new = false;
            if (!p_ext.is_valid()) {
                ui_extension::window::create_by_guid(band.m_guid, p_ext);
                b_new = true;
            }

            if (p_ext.is_valid() && p_ext->is_available(&g_ui_ext_host_rebar.get_static_instance())) {
                adding = true;
                if (b_new) {
                    try {
                        p_ext->set_config_from_ptr(
                            band.m_config.get_ptr(), band.m_config.get_size(), abortCallbackDummy);
                    } catch (const exception_io& e) {
                        console::formatter formatter;
                        formatter << "Error setting panel config: " << e.what();
                    }
                }
                band.m_wnd = p_ext->create_or_transfer_window(
                    wnd_rebar, ui_extension::window_host_ptr(&g_ui_ext_host_rebar.get_static_instance()));
                if (band.m_wnd) {
                    band.m_window = p_ext;
                    ShowWindow(band.m_wnd, SW_SHOWNORMAL);

                    rbbi.fMask |= RBBIM_CHILDSIZE;

                    MINMAXINFO mmi;
                    memset(&mmi, 0, sizeof(MINMAXINFO));
                    mmi.ptMaxTrackSize.x = MAXLONG;
                    mmi.ptMaxTrackSize.y = MAXLONG;
                    SendMessage(band.m_wnd, WM_GETMINMAXINFO, 0, reinterpret_cast<LPARAM>(&mmi));

                    if (mmi.ptMaxTrackSize.y < 0)
                        mmi.ptMaxTrackSize.y = 0;
                    if (mmi.ptMinTrackSize.y <= 0)
                        mmi.ptMinTrackSize.y = min(uih::scale_dpi_value(default_toolbar_height), mmi.ptMaxTrackSize.y);
                    if (mmi.ptMinTrackSize.x <= 0)
                        mmi.ptMinTrackSize.x = uih::scale_dpi_value(default_toolbar_width);

                    rbbi.cyMinChild = mmi.ptMinTrackSize.y;
                    rbbi.cyMaxChild = mmi.ptMaxTrackSize.y;
                    rbbi.cxMinChild = mmi.ptMinTrackSize.x;
                } else {
                    p_ext.release();
                }
            } else {
                p_ext.release();
            }
        }

        if (band.m_wnd) {
            rbbi.fMask |= RBBIM_SIZE | RBBIM_CHILD | RBBIM_HEADERSIZE | RBBIM_LPARAM | RBBIM_STYLE;
            // rbbi.cyIntegral = 1;
            rbbi.cx = m_bands[n].m_width;
            rbbi.fStyle = /*RBBS_VARIABLEHEIGHT|*/ RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS
                | (band.m_break_before_band ? RBBS_BREAK : 0) | (cfg_lock ? RBBS_NOGRIPPER : 0);
            rbbi.lParam = n;
            rbbi.hwndChild = band.m_wnd;
            rbbi.cxHeader = cfg_lock ? 5 : 9;

            uRebar_InsertItem(wnd_rebar, n, &rbbi, adding);

            n++;
        } else {
            m_bands.erase(m_bands.begin() + n);
            --count;
        }
    }
}

ui_extension::window_host& get_rebar_host()
{
    return g_ui_ext_host_rebar.get_static_instance();
}
