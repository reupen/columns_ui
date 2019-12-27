#include "stdafx.h"
#include "ng_playlist/ng_playlist.h"
#include "setup_dialog.h"
#include "main_window.h"

BOOL CALLBACK QuickSetupDialog::g_SetupDialogProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    QuickSetupDialog* p_data = nullptr;
    if (msg == WM_INITDIALOG) {
        p_data = reinterpret_cast<QuickSetupDialog*>(lp);
        SetWindowLongPtr(wnd, DWLP_USER, lp);
    } else
        p_data = reinterpret_cast<QuickSetupDialog*>(GetWindowLongPtr(wnd, DWLP_USER));
    return p_data ? p_data->SetupDialogProc(wnd, msg, wp, lp) : FALSE;
}
BOOL QuickSetupDialog::SetupDialogProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        m_this = shared_from_this();

        modeless_dialog_manager::g_add(wnd);

        HWND wnd_lv = GetDlgItem(wnd, IDC_LIST);
        HWND wnd_theming = GetDlgItem(wnd, IDC_THEMING);
        HWND wnd_grouping = GetDlgItem(wnd, IDC_GROUPING);

        uih::list_view_set_explorer_theme(wnd_lv);

        RECT rc_work;
        RECT rc_dialog;
        SystemParametersInfo(SPI_GETWORKAREA, NULL, &rc_work, NULL);
        GetWindowRect(wnd, &rc_dialog);

        const unsigned cx = rc_dialog.right - rc_dialog.left;
        const unsigned cy = rc_dialog.bottom - rc_dialog.top;

        unsigned left = (rc_work.right - rc_work.left - cx) / 2;
        unsigned top = (rc_work.bottom - rc_work.top - cy) / 2;

        SetWindowPos(wnd, nullptr, left, top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

        ListView_SetExtendedListViewStyleEx(
            wnd_lv, LVS_EX_FULLROWSELECT | 0x10000000, LVS_EX_FULLROWSELECT | 0x10000000);

        LVCOLUMN lvc;
        memset(&lvc, 0, sizeof(LVCOLUMN));
        lvc.mask = LVCF_TEXT | LVCF_WIDTH;

        // MapDialogWidth(wnd, 228);

        RECT rc;
        GetClientRect(wnd_lv, &rc);

        uih::list_view_insert_column_text(wnd_lv, 0, _T("Preset"), RECT_CX(rc));

        SendMessage(wnd_lv, WM_SETREDRAW, FALSE, 0);

        cfg_layout.get_preset(cfg_layout.get_active(), m_previous_layout);

        LayoutWindow::g_get_default_presets(m_presets);

        LVITEM lvi;
        memset(&lvi, 0, sizeof(LVITEM));
        lvi.mask = LVIF_TEXT;
        t_size count = m_presets.get_count();
        for (t_size i = 0; i < count; i++) {
            uih::list_view_insert_item_text(wnd_lv, i, 0, m_presets[i].m_name, false);
        }
        ComboBox_InsertString(wnd_theming, 0, L"No");
        ComboBox_InsertString(wnd_theming, 1, L"Yes");
        m_previous_colour_mode = g_get_global_colour_mode();

        t_size select = -1;
        if (m_previous_colour_mode == cui::colours::colour_mode_themed)
            select = 1;
        else if (m_previous_colour_mode == cui::colours::colour_mode_system)
            select = 0;

        ComboBox_SetCurSel(wnd_theming, select);

        m_previous_show_grouping = pvt::cfg_grouping;
        m_previous_show_artwork = pvt::cfg_show_artwork;

        ComboBox_InsertString(wnd_grouping, 0, L"Disabled");
        ComboBox_InsertString(wnd_grouping, 1, L"Groups (without artwork)");
        ComboBox_InsertString(wnd_grouping, 2, L"Groups (with artwork)");

        select = pvt::cfg_grouping ? (pvt::cfg_show_artwork ? 2 : 1) : 0;
        ComboBox_SetCurSel(wnd_grouping, select);

        // ListView_SetItemState(wnd_lv, 0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
        SendMessage(wnd_lv, WM_SETREDRAW, TRUE, 0);
        RedrawWindow(wnd_lv, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
    }
        return TRUE;
    case WM_CTLCOLORSTATIC:
        SetBkColor((HDC)wp, GetSysColor(COLOR_WINDOW));
        SetTextColor((HDC)wp, GetSysColor(COLOR_WINDOWTEXT));
        return (BOOL)GetSysColorBrush(COLOR_WINDOW);
    case WM_PAINT:
        uih::handle_modern_background_paint(wnd, GetDlgItem(wnd, IDCANCEL));
        return TRUE;
    case WM_COMMAND:
        switch (wp) {
        case IDCANCEL:
            cfg_layout.set_preset(cfg_layout.get_active(), m_previous_layout.get_ptr());
            g_set_global_colour_mode(m_previous_colour_mode);
            pvt::cfg_show_artwork = m_previous_show_artwork;
            pvt::cfg_grouping = m_previous_show_grouping;
            pvt::PlaylistView::g_on_show_artwork_change();
            pvt::PlaylistView::g_on_groups_change();
        case IDOK:
            DestroyWindow(wnd);
            return 0;
        case (CBN_SELCHANGE << 16) | IDC_THEMING: {
            t_size selection = ComboBox_GetCurSel(HWND(lp));
            if (selection == 1)
                g_set_global_colour_mode(cui::colours::colour_mode_themed);
            else if (selection == 0)
                g_set_global_colour_mode(cui::colours::colour_mode_system);
        } break;
        case (CBN_SELCHANGE << 16) | IDC_GROUPING: {
            t_size selection = ComboBox_GetCurSel(HWND(lp));
            if (selection >= 2)
                pvt::cfg_show_artwork = true;
            if (selection >= 1)
                pvt::cfg_grouping = true;
            if (selection <= 1)
                pvt::cfg_show_artwork = false;
            if (selection == 0)
                pvt::cfg_grouping = false;

            pvt::PlaylistView::g_on_show_artwork_change();
            pvt::PlaylistView::g_on_groups_change();

        } break;
        }
        break;
    case WM_CLOSE:
        DestroyWindow(wnd);
        return 0;
    case WM_NOTIFY: {
        auto lpnm = (LPNMHDR)lp;
        switch (lpnm->idFrom) {
        case IDC_COLUMNS: {
            switch (lpnm->code) {
            case LVN_ITEMCHANGED: {
                auto lpnmlv = (LPNMLISTVIEW)lp;
                if (lpnmlv->iItem != -1 && lpnmlv->iItem >= 0 && (t_size)lpnmlv->iItem < m_presets.get_count()) {
                    if ((lpnmlv->uNewState & LVIS_SELECTED) && !(lpnmlv->uOldState & LVIS_SELECTED)) {
                        uie::splitter_item_ptr ptr;
                        m_presets[lpnmlv->iItem].get(ptr);
                        cfg_layout.set_preset(cfg_layout.get_active(), ptr.get_ptr());
                    }
                }
            }
                return 0;
            }
        } break;
        }
        break;
    } break;
    case WM_DESTROY:
        modeless_dialog_manager::g_remove(wnd);
        break;
    case WM_NCDESTROY:
        SetWindowLongPtr(wnd, DWLP_USER, NULL);
        m_this.reset();
        return FALSE;
    }

    return FALSE;
}

void QuickSetupDialog::g_run()
{
    QuickSetupDialog::ptr_t dialog = std::make_shared<QuickSetupDialog>();
    CreateDialogParam(mmh::get_current_instance(), MAKEINTRESOURCE(IDD_QUICK_SETUP), cui::main_window.get_wnd(),
        g_SetupDialogProc, reinterpret_cast<LPARAM>(dialog.get()));
}
