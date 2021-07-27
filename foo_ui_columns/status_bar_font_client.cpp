#include "stdafx.h"
#include "status_bar.h"

extern HWND g_status;

namespace cui::status_bar {

// {B9D5EA18-5827-40be-A896-302A71BCAA9C}
static const GUID font_client_status_guid
    = {0xb9d5ea18, 0x5827, 0x40be, {0xa8, 0x96, 0x30, 0x2a, 0x71, 0xbc, 0xaa, 0x9c}};

HFONT g_status_font = nullptr;

class StatusBarFontClient : public cui::fonts::client {
public:
    const GUID& get_client_guid() const override { return font_client_status_guid; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Status bar"; }

    cui::fonts::font_type_t get_default_font_type() const override { return cui::fonts::font_type_labels; }

    void on_font_changed() const override { on_status_font_change(); }
};

StatusBarFontClient::factory<StatusBarFontClient> g_font_client_status;

void on_status_font_change()
{
    if (g_status_font != nullptr) {
        if (g_status)
            SendMessage(g_status, WM_SETFONT, (WPARAM)0, MAKELPARAM(0, 0));
        DeleteObject(g_status_font);
        g_status_font = nullptr;
    }

    if (g_status) {
        g_status_font = static_api_ptr_t<cui::fonts::manager>()->get_font(font_client_status_guid);
        SendMessage(g_status, WM_SETFONT, (WPARAM)g_status_font, MAKELPARAM(1, 0));
        set_part_sizes(cui::status_bar::t_parts_all);
        main_window.resize_child_windows();
    }
}

} // namespace cui::status_bar
