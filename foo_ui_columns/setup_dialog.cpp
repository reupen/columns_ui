#include "pch.h"

#include "setup_dialog.h"

#include "dark_mode.h"
#include "ng_playlist/ng_playlist.h"
#include "main_window.h"

INT_PTR QuickSetupDialog::handle_dialog_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        m_wnd = wnd;
        modeless_dialog_manager::g_add(wnd);
        s_instances.emplace_back(this);

        m_presets_list_view.create(wnd, {14, 18, 240, 67}, true);
        LOGFONT font{};
        GetObject(GetWindowFont(wnd), sizeof(font), &font);
        m_presets_list_view.set_font(font);

        HWND wnd_mode = GetDlgItem(wnd, IDC_DARK_MODE);
        HWND wnd_theming = GetDlgItem(wnd, IDC_THEMING);
        HWND wnd_grouping = GetDlgItem(wnd, IDC_GROUPING);

        const auto monitor = MonitorFromWindow(wnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitor_info{};
        monitor_info.cbSize = sizeof(MONITORINFO);
        if (GetMonitorInfo(monitor, &monitor_info)) {
            const auto& rc_work = monitor_info.rcWork;

            RECT rc_dialog{};
            GetWindowRect(wnd, &rc_dialog);

            const unsigned cx = rc_dialog.right - rc_dialog.left;
            const unsigned cy = rc_dialog.bottom - rc_dialog.top;

            unsigned left = (rc_work.right - rc_work.left - cx) / 2;
            unsigned top = (rc_work.bottom - rc_work.top - cy) / 2;

            SetWindowPos(wnd, nullptr, left, top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        }

        LVCOLUMN lvc{};
        lvc.mask = LVCF_TEXT | LVCF_WIDTH;

        cfg_layout.get_preset(cfg_layout.get_active(), m_previous_layout);

        LayoutWindow::g_get_default_presets(m_presets);

        std::vector<uih::ListView::InsertItem> insert_items;
        insert_items.reserve(m_presets.size());

        std::ranges::transform(m_presets, std::back_inserter(insert_items),
            [](const auto& preset) { return uih::ListView::InsertItem{{preset.m_name.c_str()}, {}}; });

        m_presets_list_view.insert_items(0, insert_items.size(), insert_items.data());

        EnableWindow(wnd_mode, cui::dark::does_os_support_dark_mode());
        ComboBox_InsertString(wnd_mode, 0, L"Light");
        ComboBox_InsertString(wnd_mode, 1, L"Dark");
        ComboBox_InsertString(wnd_mode, 2, L"Use system setting");

        m_previous_mode = static_cast<cui::colours::DarkModeStatus>(cui::colours::dark_mode_status.get());
        ComboBox_SetCurSel(wnd_mode, cui::colours::dark_mode_status.get());

        ComboBox_InsertString(wnd_theming, 0, L"No");
        ComboBox_InsertString(wnd_theming, 1, L"Yes");

        m_previous_light_colour_scheme = g_get_global_colour_scheme(false);
        m_previous_dark_colour_scheme = g_get_global_colour_scheme(true);
        const auto active_colour_scheme = g_get_global_colour_scheme();

        size_t select = -1;
        if (active_colour_scheme == cui::colours::ColourSchemeThemed)
            select = 1;
        else if (active_colour_scheme == cui::colours::ColourSchemeSystem)
            select = 0;

        ComboBox_SetCurSel(wnd_theming, select);

        m_previous_show_grouping = cui::panels::playlist_view::cfg_grouping;
        m_previous_show_artwork = cui::panels::playlist_view::cfg_show_artwork;

        ComboBox_InsertString(wnd_grouping, 0, L"Disabled");
        ComboBox_InsertString(wnd_grouping, 1, L"Groups (without artwork)");
        ComboBox_InsertString(wnd_grouping, 2, L"Groups (with artwork)");

        select = cui::panels::playlist_view::cfg_grouping ? (cui::panels::playlist_view::cfg_show_artwork ? 2 : 1) : 0;
        ComboBox_SetCurSel(wnd_grouping, select);

        ShowWindow(m_presets_list_view.get_wnd(), SW_SHOWNORMAL);
        return TRUE;
    }
    case WM_COMMAND:
        switch (wp) {
        case IDCANCEL: {
            if (m_preset_changed)
                cfg_layout.set_preset(cfg_layout.get_active(), m_previous_layout.get_ptr());
            cui::colours::dark_mode_status.set(WI_EnumValue(m_previous_mode));
            g_set_global_colour_scheme(m_previous_light_colour_scheme, false);
            g_set_global_colour_scheme(m_previous_dark_colour_scheme, true);
            cui::panels::playlist_view::cfg_show_artwork = m_previous_show_artwork;
            cui::panels::playlist_view::cfg_grouping = m_previous_show_grouping;
            cui::panels::playlist_view::PlaylistView::g_on_show_artwork_change();
            cui::panels::playlist_view::PlaylistView::g_on_groups_change();

            DestroyWindow(wnd);
            return 0;
        }
        case IDOK:
            DestroyWindow(wnd);
            return 0;
        case CBN_SELCHANGE << 16 | IDC_THEMING: {
            const size_t selection = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp));
            if (selection == 1)
                g_set_global_colour_scheme(cui::colours::ColourSchemeThemed);
            else if (selection == 0)
                g_set_global_colour_scheme(cui::colours::ColourSchemeSystem);
            break;
        }
        case CBN_SELCHANGE << 16 | IDC_DARK_MODE: {
            const auto index = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp));
            cui::colours::dark_mode_status.set(index);
            break;
        }
        case CBN_SELCHANGE << 16 | IDC_GROUPING: {
            size_t selection = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp));
            if (selection >= 2)
                cui::panels::playlist_view::cfg_show_artwork = true;
            if (selection >= 1)
                cui::panels::playlist_view::cfg_grouping = true;
            if (selection <= 1)
                cui::panels::playlist_view::cfg_show_artwork = false;
            if (selection == 0)
                cui::panels::playlist_view::cfg_grouping = false;

            cui::panels::playlist_view::PlaylistView::g_on_show_artwork_change();
            cui::panels::playlist_view::PlaylistView::g_on_groups_change();
            break;
        }
        }
        break;
    case WM_CLOSE:
        DestroyWindow(wnd);
        return 0;
    case WM_DESTROY:
        m_presets_list_view.destroy();
        std::erase(s_instances, this);
        modeless_dialog_manager::g_remove(wnd);
        break;
    case WM_NCDESTROY:
        return FALSE;
    }

    return FALSE;
}

void QuickSetupDialog::on_preset_list_selection_change()
{
    const auto index = m_presets_list_view.get_selected_item_single();

    if (index == std::numeric_limits<size_t>::max())
        return;

    uie::splitter_item_ptr ptr;
    m_presets[index].get(ptr);
    cfg_layout.set_preset(cfg_layout.get_active(), ptr.get_ptr());
    m_preset_changed = true;
}

void QuickSetupDialog::s_run()
{
    const cui::dark::DialogDarkModeConfig dark_mode_config{
        .button_ids = {IDOK, IDCANCEL}, .combo_box_ids = {IDC_DARK_MODE, IDC_THEMING, IDC_GROUPING}};

    modeless_dialog_box(IDD_QUICK_SETUP, dark_mode_config, cui::main_window.get_wnd(),
        [dialog = std::make_shared<QuickSetupDialog>()](
            auto&&... args) { return dialog->handle_dialog_message(std::forward<decltype(args)>(args)...); });
}

void QuickSetupDialog::s_refresh()
{
    for (auto instance : s_instances) {
        ComboBox_SetCurSel(GetDlgItem(instance->m_wnd, IDC_DARK_MODE), cui::colours::dark_mode_status.get());
    }
}
