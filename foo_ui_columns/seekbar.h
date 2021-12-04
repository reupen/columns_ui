#pragma once

namespace cui::toolbars::seekbar {

class SeekBarToolbar : public ui_extension::container_ui_extension {
    bool initialised{false};
    uih::Trackbar m_child;

    class SeekBarTrackbarCallback : public uih::TrackbarCallback {
        void on_position_change(unsigned pos, bool b_tracking) override;
        void get_tooltip_text(unsigned pos, uih::TrackbarString& out) override;
    } m_track_bar_host;

public:
    static pfc::ptr_list_t<SeekBarToolbar> windows;

    HWND wnd_seekbar{nullptr};

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;

    void disable_seek();
    void update_seek();
    void update_seek_pos();
    SeekBarToolbar();
    ~SeekBarToolbar();

    class_data& get_class_data() const override
    {
        __implement_get_class_data(_T("{89A3759F-348A-4e3f-BF43-3D16BC059186}"), true);
    }

    const GUID& get_extension_guid() const override;

    void get_name(pfc::string_base& out) const override;
    void get_category(pfc::string_base& out) const override;

    unsigned get_type() const override;

    static void update_seek_timer();
    static unsigned g_seek_timer;
    static VOID CALLBACK SeekTimerProc(HWND wnd, UINT msg, UINT event, DWORD time);
    static void update_seekbars(bool positions_only = false);
};

} // namespace cui::toolbars::seekbar
