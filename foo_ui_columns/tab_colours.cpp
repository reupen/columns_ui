#include "pch.h"

#include "tab_colours.h"

#include "colour_utils.h"
#include "dark_mode.h"
#include "main_window.h"

bool TabColours::is_active()
{
    return m_wnd != nullptr;
}

bool TabColours::get_help_url(pfc::string_base& p_out)
{
    p_out = "http://yuo.be/wiki/columns_ui:config:colours_and_fonts:colours";
    return true;
}

const char* TabColours::get_name()
{
    return "Colours";
}

HWND TabColours::create(HWND wnd)
{
    return m_helper.create(
        wnd, IDD_PREFS_COLOURS, [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); });
}

void TabColours::apply() {}

INT_PTR TabColours::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        m_wnd = wnd;
        m_wnd_colour_scheme = GetDlgItem(wnd, IDC_COLOURS_SCHEME);
        m_wnd_colours_element = GetDlgItem(wnd, IDC_COLOURS_ELEMENT);

        constexpr auto y_start = 94;
        constexpr auto y_spacing = 23;
        constexpr auto x_col_1 = 140;
        constexpr auto x_col_2 = 245;

        g_fill_text.create(wnd, uih::WindowPosition{x_col_1, y_start, 18, 14}, nullptr, true);
        g_fill_selection_text.create(wnd, uih::WindowPosition{x_col_1, y_start + y_spacing, 18, 14}, nullptr, true);
        g_fill_selection_text_inactive.create(
            wnd, uih::WindowPosition{x_col_1, y_start + y_spacing * 2, 18, 14}, nullptr, true);

        g_fill_active_item_frame.create(wnd, uih::WindowPosition{x_col_1, y_start + 92, 18, 14}, nullptr, true);

        g_fill_background.create(wnd, uih::WindowPosition{x_col_2, y_start, 18, 14}, nullptr, true);
        g_fill_selection_background.create(
            wnd, uih::WindowPosition{x_col_2, y_start + y_spacing, 18, 14}, nullptr, true);
        g_fill_selection_background_inactive.create(
            wnd, uih::WindowPosition{x_col_2, y_start + y_spacing * 2, 18, 14}, nullptr, true);

        ComboBox_AddString(m_wnd_colours_element, L"Global");
        ColoursClientList::g_get_list(m_colours_client_list);
        size_t count = m_colours_client_list.get_count();
        for (size_t i = 0; i < count; i++)
            ComboBox_AddString(
                m_wnd_colours_element, pfc::stringcvt::string_os_from_utf8(m_colours_client_list[i].m_name));

        ComboBox_SetCurSel(m_wnd_colours_element, 0);

        m_element_ptr = g_colour_manager_data.get_entry(m_element_guid);

        update_title();
        update_scheme_combobox();
        update_fills();
        update_buttons();
    } break;
    case WM_DESTROY: {
        m_colours_client_list.remove_all();
        m_element_guid = pfc::guid_null;
        m_element_ptr.reset();
        m_element_api.release();
        m_wnd = nullptr;
    } break;
    case WM_COMMAND:
        switch (wp) {
        case IDC_COLOURS_ELEMENT | (CBN_SELCHANGE << 16): {
            int idx = ComboBox_GetCurSel((HWND)lp);
            m_element_api.release();
            if (idx != -1) {
                if (idx == 0)
                    m_element_guid = pfc::guid_null;
                else {
                    m_element_guid = m_colours_client_list[idx - 1].m_guid;
                    m_element_api = m_colours_client_list[idx - 1].m_ptr;
                }
                m_element_ptr = g_colour_manager_data.get_entry(m_element_guid);
            }
            update_fills();
            update_buttons();
            update_scheme_combobox();
            return 0;
        }
        case IDC_COLOURS_SCHEME | (CBN_SELCHANGE << 16): {
            int idx = ComboBox_GetCurSel((HWND)lp);
            m_element_ptr->colour_set.colour_scheme = (cui::colours::ColourScheme)ComboBox_GetItemData((HWND)lp, idx);
            update_fills();
            update_buttons();
            on_colour_changed();
            return 0;
        }
        case IDC_CHANGE_TEXT_BACK:
        case IDC_CHANGE_FRAME:
        case IDC_CHANGE_TEXT_FORE:
        case IDC_CHANGE_SELBACK:
        case IDC_CHANGE_SEL_FORE:
        case IDC_CHANGE_SEL_INACTIVE_BACK:
        case IDC_CHANGE_SEL_INACTIVE_FORE:
        case IDC_CUSTOM_FRAME: {
            auto& colour_set = m_element_ptr->colour_set;
            std::unordered_map<WPARAM, std::tuple<COLORREF&, cui::colours::colour_identifier_t>> colour_map
                = {{IDC_CHANGE_TEXT_BACK, {colour_set.background, cui::colours::colour_background}},
                    {IDC_CHANGE_TEXT_FORE, {colour_set.text, cui::colours::colour_text}},
                    {IDC_CHANGE_SELBACK, {colour_set.selection_background, cui::colours::colour_selection_background}},
                    {IDC_CHANGE_SEL_FORE, {colour_set.selection_text, cui::colours::colour_selection_text}},
                    {IDC_CHANGE_SEL_INACTIVE_BACK,
                        {colour_set.inactive_selection_background, cui::colours::colour_inactive_selection_background}},
                    {IDC_CHANGE_SEL_INACTIVE_FORE,
                        {colour_set.inactive_selection_text, cui::colours::colour_inactive_selection_text}},
                    {IDC_CHANGE_FRAME, {colour_set.active_item_frame, cui::colours::colour_active_item_frame}}};

            bool b_changed = false;

            if (wp == IDC_CUSTOM_FRAME) {
                b_changed = true;
                colour_set.use_custom_active_item_frame = (Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED);
            } else {
                auto&& [colour, colour_id] = colour_map.at(wp);
                b_changed = colour_picker(wnd, colour,
                    cui::dark::get_system_colour(get_system_colour_id(colour_id), cui::colours::is_dark_mode_active()));
            }

            if (b_changed) {
                update_fills();
                on_colour_changed();
            }

            if (wp == IDC_CUSTOM_FRAME)
                update_buttons();

            return 0;
        }
        }
        break;
    }
    return 0;
}

void TabColours::on_colour_changed()
{
    if (m_element_api.is_valid())
        m_element_api->on_colour_changed(cui::colours::colour_flag_all);
    else if (m_element_guid == pfc::guid_null) {
        cui::colours::common_colour_callback_manager.s_on_common_colour_changed(cui::colours::colour_flag_all);
        size_t count = m_colours_client_list.get_count();
        for (size_t i = 0; i < count; i++) {
            const auto p_data = g_colour_manager_data.get_entry(m_colours_client_list[i].m_guid);
            if (p_data->colour_set.colour_scheme == cui::colours::ColourSchemeGlobal)
                m_colours_client_list[i].m_ptr->on_colour_changed(cui::colours::colour_flag_all);
        }
    }
}

void TabColours::update_scheme_combobox()
{
    ComboBox_ResetContent(m_wnd_colour_scheme);
    size_t index;
    if (m_element_guid != pfc::guid_null) {
        index = ComboBox_AddString(m_wnd_colour_scheme, L"Global");
        ComboBox_SetItemData(m_wnd_colour_scheme, index, cui::colours::ColourSchemeGlobal);
    }
    index = ComboBox_AddString(m_wnd_colour_scheme, L"System");
    ComboBox_SetItemData(m_wnd_colour_scheme, index, cui::colours::ColourSchemeSystem);
    if (!m_element_api.is_valid() || m_element_api->get_themes_supported()) {
        index = ComboBox_AddString(m_wnd_colour_scheme, L"Themed");
        ComboBox_SetItemData(m_wnd_colour_scheme, index, cui::colours::ColourSchemeThemed);
    }
    index = ComboBox_AddString(m_wnd_colour_scheme, L"Custom");
    ComboBox_SetItemData(m_wnd_colour_scheme, index, cui::colours::ColourSchemeCustom);

    ComboBox_SetCurSel(m_wnd_colour_scheme,
        uih::combo_box_find_item_by_data(m_wnd_colour_scheme, m_element_ptr->colour_set.colour_scheme));
}

void TabColours::update_title() const
{
    if (cui::dark::are_private_apis_allowed())
        uSetDlgItemText(
            m_wnd, IDC_TITLE1, cui::colours::is_dark_mode_active() ? "Dark mode colours" : "Light mode colours");
}

void TabColours::handle_external_configuration_change()
{
    if (!is_active())
        return;

    m_element_ptr = g_colour_manager_data.get_entry(m_element_guid);
    update_title();
    update_scheme_combobox();
    update_buttons();
    update_fills();
}

void TabColours::update_buttons()
{
    EnableWindow(GetDlgItem(m_wnd, IDC_CHANGE_TEXT_BACK), get_change_colour_enabled(cui::colours::colour_background));
    EnableWindow(GetDlgItem(m_wnd, IDC_CHANGE_TEXT_FORE), get_change_colour_enabled(cui::colours::colour_text));
    EnableWindow(
        GetDlgItem(m_wnd, IDC_CHANGE_SELBACK), get_change_colour_enabled(cui::colours::colour_selection_background));
    EnableWindow(
        GetDlgItem(m_wnd, IDC_CHANGE_SEL_FORE), get_change_colour_enabled(cui::colours::colour_selection_text));
    EnableWindow(GetDlgItem(m_wnd, IDC_CHANGE_SEL_INACTIVE_BACK),
        get_change_colour_enabled(cui::colours::colour_inactive_selection_background));
    EnableWindow(GetDlgItem(m_wnd, IDC_CHANGE_SEL_INACTIVE_FORE),
        get_change_colour_enabled(cui::colours::colour_inactive_selection_text));
    EnableWindow(
        GetDlgItem(m_wnd, IDC_CHANGE_FRAME), get_change_colour_enabled(cui::colours::colour_active_item_frame));

    EnableWindow(g_fill_background.get_wnd(), get_colour_patch_enabled(cui::colours::colour_background));
    EnableWindow(g_fill_text.get_wnd(), get_colour_patch_enabled(cui::colours::colour_text));
    EnableWindow(
        g_fill_selection_background.get_wnd(), get_colour_patch_enabled(cui::colours::colour_selection_background));
    EnableWindow(g_fill_selection_text.get_wnd(), get_colour_patch_enabled(cui::colours::colour_selection_text));
    EnableWindow(g_fill_selection_background_inactive.get_wnd(),
        get_colour_patch_enabled(cui::colours::colour_inactive_selection_background));
    EnableWindow(g_fill_selection_text_inactive.get_wnd(),
        get_colour_patch_enabled(cui::colours::colour_inactive_selection_text));

    EnableWindow(g_fill_active_item_frame.get_wnd(), get_colour_patch_enabled(cui::colours::colour_active_item_frame));
    EnableWindow(GetDlgItem(m_wnd, IDC_CUSTOM_FRAME),
        m_element_ptr->colour_set.colour_scheme != cui::colours::ColourSchemeGlobal
            && (!m_element_api.is_valid()
                || (m_element_api->get_supported_bools() & cui::colours::bool_flag_use_custom_active_item_frame)));
}

bool TabColours::get_colour_patch_enabled(cui::colours::colour_identifier_t p_identifier)
{
    cui::colours::helper colour_helper(m_element_guid);

    if (p_identifier == cui::colours::colour_active_item_frame) {
        if (!m_element_api.is_valid()) {
            return m_element_ptr->colour_set.colour_scheme == cui::colours::ColourSchemeCustom
                || colour_helper.get_bool(cui::colours::bool_use_custom_active_item_frame);
        }
        const bool is_colour_supported = (m_element_api->get_supported_colours() & (1 << p_identifier)) != 0;
        const bool use_custom_frame_supported
            = (m_element_api->get_supported_bools() & cui::colours::bool_flag_use_custom_active_item_frame) != 0;

        return is_colour_supported
            && (!use_custom_frame_supported || colour_helper.get_bool(cui::colours::bool_use_custom_active_item_frame));
    }
    return !m_element_api.is_valid() || (m_element_api->get_supported_colours() & (1 << p_identifier));
}

bool TabColours::get_change_colour_enabled(cui::colours::colour_identifier_t p_identifier)
{
    if (p_identifier == cui::colours::colour_active_item_frame) {
        cui::colours::helper colours(m_element_guid);

        if (m_element_api.is_valid()) {
            return m_element_ptr->colour_set.colour_scheme != cui::colours::ColourSchemeGlobal
                && m_element_api->get_supported_colours() & cui::colours::colour_flag_active_item_frame
                && (!(m_element_api->get_supported_bools() & cui::colours::bool_flag_use_custom_active_item_frame)
                    || colours.get_bool(cui::colours::bool_use_custom_active_item_frame));
        }

        return m_element_ptr->colour_set.colour_scheme == cui::colours::ColourSchemeCustom
            || colours.get_bool(cui::colours::bool_use_custom_active_item_frame);
    }

    return (m_element_ptr->colour_set.colour_scheme == cui::colours::ColourSchemeCustom
        && (!m_element_api.is_valid() || (m_element_api->get_supported_colours() & (1 << p_identifier))));
}

void TabColours::update_fills()
{
    cui::colours::helper p_manager(m_element_guid);

    if (p_manager.get_themed() && (!m_element_api.is_valid() || m_element_api->get_themes_supported())) {
        const auto is_dark = cui::colours::is_dark_mode_active();
        const auto list_view_theme_class = is_dark ? L"DarkMode_ItemsView::ListView"sv : L"Explorer::ListView"sv;

        g_fill_selection_background.set_fill_themed(uih::FillWindow::Mode::ThemeBackgroundFill,
            list_view_theme_class.data(), LVP_LISTITEM, LISS_SELECTED, is_dark,
            p_manager.get_colour(cui::colours::colour_selection_background));

        g_fill_selection_background_inactive.set_fill_themed(uih::FillWindow::Mode::ThemeBackgroundFill,
            list_view_theme_class.data(), LVP_LISTITEM, LISS_SELECTEDNOTFOCUS, is_dark,
            p_manager.get_colour(cui::colours::colour_inactive_selection_background));

        g_fill_selection_text.set_fill_themed(uih::FillWindow::Mode::ThemeTextColourFill, list_view_theme_class.data(),
            LVP_LISTITEM, LISS_SELECTED, is_dark, p_manager.get_colour(cui::colours::colour_selection_text));

        g_fill_selection_text_inactive.set_fill_themed(uih::FillWindow::Mode::ThemeTextColourFill,
            list_view_theme_class.data(), LVP_LISTITEM, LISS_SELECTEDNOTFOCUS, is_dark,
            p_manager.get_colour(cui::colours::colour_inactive_selection_text));

    } else {
        g_fill_selection_text.set_fill_colour(p_manager.get_colour(cui::colours::colour_selection_text));
        g_fill_selection_background.set_fill_colour(p_manager.get_colour(cui::colours::colour_selection_background));
        g_fill_selection_background_inactive.set_fill_colour(
            p_manager.get_colour(cui::colours::colour_inactive_selection_background));
        g_fill_selection_text_inactive.set_fill_colour(
            p_manager.get_colour(cui::colours::colour_inactive_selection_text));
    }

    g_fill_text.set_fill_colour(p_manager.get_colour(cui::colours::colour_text));
    g_fill_background.set_fill_colour(p_manager.get_colour(cui::colours::colour_background));
    g_fill_active_item_frame.set_fill_colour(p_manager.get_colour(cui::colours::colour_active_item_frame));

    Button_SetCheck(
        GetDlgItem(m_wnd, IDC_CUSTOM_FRAME), p_manager.get_bool(cui::colours::bool_use_custom_active_item_frame));
}
