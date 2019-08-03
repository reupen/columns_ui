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
#include "rebar_band.h"

struct BandCacheEntry {
    GUID guid{};
    unsigned width{};
};

class BandCache : public pfc::list_t<BandCacheEntry> {
public:
    void add_entry(const GUID& guid, unsigned width);
    unsigned get_width(const GUID& guid);
    void write(stream_writer* out, abort_callback& p_abort);
    void read(stream_reader* data, abort_callback& p_abort);

    void copy(BandCache& in)
    {
        remove_all();
        add_items(in);
    }
};

class ConfigBandCache : public cfg_var {
private:
    BandCache entries;

    void get_data_raw(stream_writer* out, abort_callback& p_abort) override;
    void set_data_raw(stream_reader* p_reader, unsigned p_sizehint, abort_callback& p_abort) override;

public:
    explicit ConfigBandCache(const GUID& p_guid) : cfg_var(p_guid) { reset(); };
    void get_band_cache(BandCache& out);
    void set_band_cache(BandCache& in);
    void reset();
};

class ConfigRebar : public cfg_var {
private:
    enum class StreamVersion : uint32_t { Version0 = 0, Version1 = 1, VersionCurrent = Version1 };

    std::vector<RebarBandState> m_entries;

    void get_data_raw(stream_writer* out, abort_callback& p_abort) override;
    void set_data_raw(stream_reader* p_reader, unsigned p_sizehint, abort_callback& p_abort) override;

public:
    void export_config(
        stream_writer* p_out, t_uint32 mode, cui::fcl::t_export_feedback& feedback, abort_callback& p_abort);
    void import_config(
        stream_reader* p_reader, t_size size, t_uint32 mode, pfc::list_base_t<GUID>& panels, abort_callback& p_abort);

    explicit ConfigRebar(const GUID& p_guid) : cfg_var(p_guid) { reset(); };

    const std::vector<RebarBandState>& get_rebar_info() { return m_entries; }

    template <typename Container>
    void set_rebar_info(Container&& in)
    {
        m_entries = in;
    }

    void reset();
};

class RebarWindow {
private:
    void destroy_bands();

public:
    HWND wnd_rebar{nullptr};
    BandCache cache;

    RebarWindow() = default;
    RebarWindow(const RebarWindow&) = delete;
    RebarWindow& operator=(const RebarWindow&) = delete;
    RebarWindow(RebarWindow&&) = delete;
    RebarWindow& operator=(RebarWindow&&) = delete;
    ~RebarWindow() = default;

    HWND init();

    void refresh_band_configs();
    const std::vector<RebarBand>& get_bands() const { return m_bands; }

    [[nodiscard]] std::vector<RebarBandState> get_band_states() const;

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

private:
    /**
     * For some reason, the z-order gets messed up after adding bands to the rebar control.
     * This makes sure that the z-order for bands goes from top to bottom.
     */
    void fix_z_order();

    std::vector<RebarBand> m_bands;

    friend class RebarWindowHost;
};

ui_extension::window_host& get_rebar_host();

#endif
