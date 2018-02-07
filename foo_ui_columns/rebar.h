#ifndef _COLUMNS_REBAR_H_
#define _COLUMNS_REBAR_H_

/*!
 * \file rebar.h
 *
 * \author musicmusic
 * \date 1 March 2015
 *
 * Classes used for the toolbars (rebar control) of the main window
 */

#include "stdafx.h"
#include "extern.h"
#include "rebar_band.h"

struct band_cache_entry {
    GUID guid{};
    unsigned width{};
};

class band_cache : public pfc::list_t<band_cache_entry> {
public:
    void add_entry(const GUID& guid, unsigned width);
    unsigned get_width(const GUID& guid);
    void write(stream_writer* out, abort_callback& p_abort);
    void read(stream_reader* data, abort_callback& p_abort);

    void copy(band_cache& in)
    {
        remove_all();
        add_items(in);
    }
};

class cfg_band_cache_t : public cfg_var {
private:
    band_cache entries;

    void get_data_raw(stream_writer* out, abort_callback& p_abort) override;
    void set_data_raw(stream_reader* p_reader, unsigned p_sizehint, abort_callback& p_abort) override;

public:
    explicit cfg_band_cache_t(const GUID& p_guid) : cfg_var(p_guid) { reset(); };
    void get_band_cache(band_cache& out);
    void set_band_cache(band_cache& in);
    void reset();
};

class cfg_rebar : public cfg_var {
private:
    enum class StreamVersion : uint32_t { Version0 = 0, Version1 = 1, VersionCurrent = Version1 };

    std::vector<RebarBandInfo> m_entries;

    void get_data_raw(stream_writer* out, abort_callback& p_abort) override;
    void set_data_raw(stream_reader* p_reader, unsigned p_sizehint, abort_callback& p_abort) override;

public:
    void export_config(
        stream_writer* p_out, t_uint32 mode, cui::fcl::t_export_feedback& feedback, abort_callback& p_abort);
    void import_config(
        stream_reader* p_reader, t_size size, t_uint32 mode, pfc::list_base_t<GUID>& panels, abort_callback& p_abort);

    explicit cfg_rebar(const GUID& p_guid) : cfg_var(p_guid) { reset(); };

    const std::vector<RebarBandInfo>& get_rebar_info() { return m_entries; }

    template <typename Container>
    void set_rebar_info(Container&& in)
    {
        m_entries = in;
    }

    void reset();
};

class rebar_window {
private:
    void destroy_bands();

public:
    HWND wnd_rebar{nullptr};
    std::vector<RebarBandInfo> m_bands;
    band_cache cache;

    rebar_window() = default;
    rebar_window(const rebar_window&) = delete;
    rebar_window& operator=(const rebar_window&) = delete;
    rebar_window(rebar_window&&) = delete;
    rebar_window& operator=(rebar_window&&) = delete;
    ~rebar_window() = default;

    HWND init();

    void add_band(
        const GUID& guid, unsigned width = 100, const ui_extension::window_ptr& p_ext = ui_extension::window_ptr_null);
    void insert_band(unsigned idx, const GUID& guid, unsigned width = 100,
        const ui_extension::window_ptr& p_ext = ui_extension::window_ptr_null);
    void update_bands();
    void delete_band(HWND wnd, bool destroy = true);

    void update_band(unsigned n, bool size = false);

    bool check_band(const GUID& id);
    bool find_band(const GUID& id, unsigned& out);
    bool delete_band(const GUID& id);
    void delete_band(unsigned idx);

    void on_themechanged();

    bool on_menu_char(unsigned short c);
    void show_accelerators();
    bool set_menu_focus();
    void hide_accelerators();
    bool is_menu_focused();
    bool get_previous_menu_focus_window(HWND& wnd_previous) const;

    // save bands on layout changed - easier

    void save_bands();
    void destroy();
    void refresh_bands(bool force_destroy_bands = true);

    auto find_band_by_hwnd(HWND wnd)
    {
        return std::find_if(std::begin(m_bands), std::end(m_bands), [&wnd](auto&& item) { return item.m_wnd == wnd; });
    }
};

ui_extension::window_host& get_rebar_host();

#endif
