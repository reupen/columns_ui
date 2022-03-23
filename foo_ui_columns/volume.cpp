#include "pch.h"
#include "volume.h"

class VolumeBarToolbarAttributes {
public:
    static const TCHAR* get_class_name() { return _T("volume_toolbar"); }
    static bool get_show_caption() { return false; }
};

class VolumeBarToolbar : public uie::window {
    const GUID& get_extension_guid() const override
    {
        // {B3259290-CB68-4d37-B0F1-8094862A9524}
        static const GUID ret = {0xb3259290, 0xcb68, 0x4d37, {0xb0, 0xf1, 0x80, 0x94, 0x86, 0x2a, 0x95, 0x24}};
        return ret;
    }

    void get_name(pfc::string_base& out) const override { out = "Volume"; }
    void get_category(pfc::string_base& out) const override { out = "Toolbars"; }

    unsigned get_type() const override { return uie::type_toolbar; }

    bool is_available(const uie::window_host_ptr& p) const override { return true; }

    HWND get_wnd() const final { return m_volume_bar.get_wnd(); }

    HWND create_or_transfer_window(
        HWND parent, const uie::window_host_ptr& host, const ui_helpers::window_position_t& position) final
    {
        if (get_wnd()) {
            ShowWindow(get_wnd(), SW_HIDE);
            SetParent(get_wnd(), parent);
            m_host->relinquish_ownership(get_wnd());
            m_host = host;

            SetWindowPos(get_wnd(), nullptr, position.x, position.y, position.cx, position.cy, SWP_NOZORDER);
        } else {
            m_host = host;
            m_volume_bar.create(parent, position.x, position.y, position.cx, position.cy);
        }

        return get_wnd();
    }

    void destroy_window() final
    {
        m_volume_bar.destroy();
        m_host.release();
    }

    VolumeBar<false, false, VolumeBarToolbarAttributes> m_volume_bar;
    uie::window_host_ptr m_host;
};

uie::window_factory<VolumeBarToolbar> g_volume_panel;
