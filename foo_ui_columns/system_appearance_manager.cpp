#include "stdafx.h"

#include "system_appearance_manager.h"

#include "config_appearance.h"

namespace cui::system_appearance_manager {

namespace {

class AppearanceMessageWindow : public ui_helpers::container_window_autorelease_t {
public:
    static void s_initialise()
    {
        if (!s_initialised) {
            auto ptr = new AppearanceMessageWindow;
            ptr->create(HWND_MESSAGE);
            s_initialised = true;
        }
    }

private:
    class_data& get_class_data() const override
    {
        __implement_get_class_data_ex(_T("{BDCEC7A3-7230-4671-A5F7-B19A989DCA81}"), _T(""), false, 0, 0, 0, 0);
    }

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override
    {
        switch (msg) {
        case WM_SYSCOLORCHANGE: {
            ColoursClientList m_colours_client_list;
            ColoursClientList::g_get_list(m_colours_client_list);
            t_size count = m_colours_client_list.get_count();
            bool b_global_custom = g_colour_manager_data.m_global_entry->colour_mode == colours::colour_mode_custom;
            if (!b_global_custom)
                g_colour_manager_data.g_on_common_colour_changed(colours::colour_flag_all);
            for (t_size i = 0; i < count; i++) {
                ColourManagerData::entry_ptr_t p_data;
                g_colour_manager_data.find_by_guid(m_colours_client_list[i].m_guid, p_data);
                if (p_data->colour_mode == colours::colour_mode_system
                    || p_data->colour_mode == colours::colour_mode_themed
                    || (p_data->colour_mode == colours::colour_mode_global && !b_global_custom)) {
                    m_colours_client_list[i].m_ptr->on_colour_changed(colours::colour_flag_all);
                }
            }
        } break;
        case WM_SETTINGCHANGE:
            if ((wp == SPI_GETICONTITLELOGFONT && g_font_manager_data.m_common_items_entry
                    && g_font_manager_data.m_common_items_entry->font_mode == fonts::font_mode_system)
                || (wp == SPI_GETNONCLIENTMETRICS && g_font_manager_data.m_common_labels_entry
                    && g_font_manager_data.m_common_labels_entry->font_mode == fonts::font_mode_system)) {
                FontsClientList m_fonts_client_list;
                FontsClientList::g_get_list(m_fonts_client_list);
                t_size count = m_fonts_client_list.get_count();
                g_font_manager_data.g_on_common_font_changed(
                    wp == SPI_GETICONTITLELOGFONT ? fonts::font_type_flag_items : fonts::font_type_flag_labels);
                for (t_size i = 0; i < count; i++) {
                    FontManagerData::entry_ptr_t p_data;
                    g_font_manager_data.find_by_guid(m_fonts_client_list[i].m_guid, p_data);
                    if (wp == SPI_GETNONCLIENTMETRICS && p_data->font_mode == fonts::font_mode_common_items)
                        m_fonts_client_list[i].m_ptr->on_font_changed();
                    else if (wp == SPI_GETICONTITLELOGFONT && p_data->font_mode == fonts::font_mode_common_labels)
                        m_fonts_client_list[i].m_ptr->on_font_changed();
                }
            }
            break;
        case WM_CLOSE:
            destroy();
            delete this;
            return 0;
        }
        return DefWindowProc(wnd, msg, wp, lp);
    }

    static bool s_initialised;
};

bool AppearanceMessageWindow::s_initialised = false;

} // namespace

void initialise()
{
    AppearanceMessageWindow::s_initialise();
}

} // namespace cui
