#pragma once

#include "pch.h"
#include "rebar_band.h"

namespace cui::rebar {

extern class RebarWindow* g_rebar_window;
extern class ConfigRebar g_cfg_rebar;
extern HWND g_rebar;

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
    void set_data_raw(stream_reader* p_reader, size_t p_sizehint, abort_callback& p_abort) override;

public:
    explicit ConfigBandCache(const GUID& p_guid) : cfg_var(p_guid) { reset(); }
    void get_band_cache(BandCache& out);
    void set_band_cache(BandCache& in);
    void reset();
};

class ConfigRebar : public cfg_var {
private:
    enum class StreamVersion : uint32_t {
        Version0 = 0,
        Version1 = 1,
        VersionCurrent = Version1
    };

    std::vector<RebarBandState> m_entries;

    void get_data_raw(stream_writer* out, abort_callback& p_abort) override;
    void set_data_raw(stream_reader* p_reader, size_t p_sizehint, abort_callback& p_abort) override;

public:
    void export_config(stream_writer* p_out, uint32_t mode, fcl::t_export_feedback& feedback, abort_callback& p_abort);
    void import_config(
        stream_reader* p_reader, size_t size, uint32_t mode, pfc::list_base_t<GUID>& panels, abort_callback& p_abort);

    explicit ConfigRebar(const GUID& p_guid) : cfg_var(p_guid) { reset(); }

    const std::vector<RebarBandState>& get_rebar_info() { return m_entries; }

    template <typename Container>
    void set_rebar_info(Container&& in)
    {
        m_entries = in;
    }

    void reset();
};

class RebarWindow {
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

    void update_band(size_t n, bool size = false);

    bool check_band(const GUID& id);
    bool find_band(const GUID& id, size_t& out);
    bool delete_band(const GUID& id);
    void delete_band(size_t idx);

    void on_themechanged();
    std::optional<LRESULT> handle_custom_draw(LPNMCUSTOMDRAW lpnmcd) const;

    bool on_menu_char(unsigned short c);
    void show_accelerators();
    bool set_menu_focus();
    void hide_accelerators();
    bool is_menu_focused();
    bool get_previous_menu_focus_window(HWND& wnd_previous) const;

    // save bands on layout changed - easier

    void save_bands();
    void destroy();
    void refresh_bands();

    auto find_band_by_hwnd(HWND wnd)
    {
        return std::ranges::find_if(m_bands, [&wnd](auto&& item) { return item.m_wnd == wnd; });
    }

private:
    static LRESULT WINAPI s_handle_hooked_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    LRESULT WINAPI handle_hooked_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    void destroy_bands();

    /**
     * For some reason, the z-order gets messed up after adding bands to the rebar control.
     * This makes sure that the z-order for bands goes from top to bottom.
     */
    void fix_z_order();

    WNDPROC m_rebar_wnd_proc{nullptr};
    std::vector<RebarBand> m_bands;
    std::unique_ptr<colours::dark_mode_notifier> m_dark_mode_notifier;

    friend class RebarWindowHost;
};

void create_rebar();
void destroy_rebar(bool save_config = true);
ui_extension::window_host& get_rebar_host();

} // namespace cui::rebar
