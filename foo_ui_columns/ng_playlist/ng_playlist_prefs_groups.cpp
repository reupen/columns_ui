#include "pch.h"

#include "dark_mode_dialog.h"
#include "ng_playlist.h"
#include "ng_playlist_groups.h"

namespace cui::panels::playlist_view {

struct edit_view_param {
    Group value;
    bool b_new{};
};

static INT_PTR CALLBACK EditViewProc(edit_view_param& state, HWND wnd, UINT msg, WPARAM wp, LPARAM lp) noexcept
{
    switch (msg) {
    case WM_INITDIALOG: {
        SetWindowText(wnd, state.b_new ? L"Add new group" : L"Edit group");

        uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_ADDSTRING, 0, "Show on all playlists");
        uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_ADDSTRING, 0, "Show only on playlists:");
        uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_ADDSTRING, 0, "Hide on playlists:");

        EnableWindow(GetDlgItem(wnd, IDC_PLAYLIST_FILTER_STRING), state.value.filter_type != FILTER_NONE);

        SendDlgItemMessage(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_SETCURSEL, (size_t)state.value.filter_type, 0);
        uih::enhance_edit_control(wnd, IDC_PLAYLIST_FILTER_STRING);
        uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_STRING, WM_SETTEXT, 0, state.value.filter_playlists);

        uih::enhance_edit_control(wnd, IDC_VALUE);
        uSetDlgItemText(wnd, IDC_VALUE, state.value.string);
    } break;
    case WM_COMMAND:
        switch (wp) {
        case IDCANCEL:
            EndDialog(wnd, 0);
            break;
        case (CBN_SELCHANGE << 16) | IDC_PLAYLIST_FILTER_TYPE: {
            if (true) {
                EnableWindow(GetDlgItem(wnd, IDC_PLAYLIST_FILTER_STRING),
                    ((PlaylistFilterType)SendMessage((HWND)lp, CB_GETCURSEL, 0, 0)) != FILTER_NONE);
            }
        } break;
        case IDOK: {
            uGetDlgItemText(wnd, IDC_VALUE, state.value.string);
            state.value.filter_type
                = ((PlaylistFilterType)SendDlgItemMessage(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_GETCURSEL, 0, 0));
            state.value.filter_playlists = (uGetDlgItemText(wnd, IDC_PLAYLIST_FILTER_STRING));
            EndDialog(wnd, 1);

        } break;
        }
        break;
    }
    return FALSE;
}

static bool run_edit_view(edit_view_param& param, HWND parent)
{
    dark::DialogDarkModeConfig dark_mode_config{.button_ids = {IDOK, IDCANCEL},
        .combo_box_ids = {IDC_PLAYLIST_FILTER_TYPE},
        .edit_ids = {IDC_VALUE, IDC_PLAYLIST_FILTER_STRING}};
    const auto dialog_result = modal_dialog_box(IDD_EDIT_GROUP, dark_mode_config, parent,
        [&param](auto&&... args) { return EditViewProc(param, std::forward<decltype(args)>(args)...); });

    return dialog_result > 0;
}

BOOL GroupsPreferencesTab::ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        m_wnd = wnd;
        m_groups_list_view.create(wnd, {7, 138, 313, 107}, true);
        SetWindowPos(m_groups_list_view.get_wnd(), GetDlgItem(wnd, IDC_H2_TITLE), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

        LOGFONT font{};
        GetObject(GetWindowFont(wnd), sizeof(font), &font);
        try {
            auto context = uih::direct_write::Context::s_create();
            m_groups_list_view.set_font_from_log_font(font);
        }
        CATCH_LOG()

        std::vector<uih::ListView::InsertItem> insert_items;
        insert_items.reserve(g_groups.get_groups().get_count());

        auto& groups = g_groups.get_groups();
        std::transform(groups.begin(), groups.end(), std::back_inserter(insert_items),
            [](const auto& group) { return uih::ListView::InsertItem{{group.string}, {}}; });

        m_groups_list_view.insert_items(0, insert_items.size(), insert_items.data());
        ShowWindow(m_groups_list_view.get_wnd(), SW_SHOWNORMAL);

        Button_SetCheck(GetDlgItem(wnd, IDC_GROUPING), cfg_grouping ? BST_CHECKED : BST_UNCHECKED);
        Button_SetCheck(GetDlgItem(wnd, IDC_INDENT_GROUPS), cfg_indent_groups ? BST_CHECKED : BST_UNCHECKED);
        Button_SetCheck(GetDlgItem(wnd, IDC_USE_CUSTOM_INDENTATION),
            cfg_use_custom_group_indentation_amount ? BST_CHECKED : BST_UNCHECKED);

        uih::enhance_edit_control(wnd, IDC_INDENTATION_AMOUNT);

        SendDlgItemMessage(wnd, IDC_INDENTATION_AMOUNT_SPIN, UDM_SETRANGE32, 0, 256);
        SendDlgItemMessage(wnd, IDC_INDENTATION_AMOUNT_SPIN, UDM_SETPOS32, NULL, cfg_custom_group_indentation_amount);

        uih::enhance_edit_control(wnd, IDC_ROOT_INDENTATION_AMOUNT);

        SendDlgItemMessage(wnd, IDC_ROOT_INDENTATION_AMOUNT_SPIN, UDM_SETRANGE32, 0, 256);
        SendDlgItemMessage(
            wnd, IDC_ROOT_INDENTATION_AMOUNT_SPIN, UDM_SETPOS32, NULL, cfg_root_group_indentation_amount);

        refresh_enabled_options();

        m_initialised = true;
        break;
    }
    case WM_COMMAND:
        switch (wp) {
        case IDC_GROUPING: {
            cfg_grouping = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
            refresh_enabled_options();
            PlaylistView::g_on_groups_change();
            break;
        }
        case IDC_INDENT_GROUPS:
            cfg_indent_groups = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
            refresh_enabled_options();
            break;
        case IDC_USE_CUSTOM_INDENTATION:
            cfg_use_custom_group_indentation_amount = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
            refresh_enabled_options();
            break;
        case (EN_CHANGE << 16) | IDC_INDENTATION_AMOUNT:
            if (m_initialised) {
                cfg_custom_group_indentation_amount
                    = mmh::strtoul_n(uGetWindowText(reinterpret_cast<HWND>(lp)).get_ptr(), pfc_infinite);
                PlaylistView::s_on_group_indentation_amount_change();
            }
            break;
        case (EN_CHANGE << 16) | IDC_ROOT_INDENTATION_AMOUNT:
            if (m_initialised) {
                cfg_root_group_indentation_amount
                    = mmh::strtoul_n(uGetWindowText(reinterpret_cast<HWND>(lp)).get_ptr(), pfc_infinite);
                PlaylistView::s_on_root_group_indentation_amount_change();
            }
            break;
        case IDC_GROUP_UP: {
            const auto index = m_groups_list_view.get_selected_item_single();
            auto& groups = g_groups.get_groups();

            if (index == 0 || index >= groups.size())
                break;

            g_groups.swap(index, index - 1);

            const std::vector<uih::ListView::InsertItem> insert_items{
                {{groups[index - 1].string}, {}}, {{groups[index].string}, {}}};
            m_groups_list_view.replace_items(index - 1, insert_items.size(), insert_items.data());
            m_groups_list_view.set_item_selected_single(index - 1);
            m_groups_list_view.ensure_visible(index - 1);
            break;
        }
        case IDC_GROUP_DOWN: {
            const auto index = m_groups_list_view.get_selected_item_single();
            auto& groups = g_groups.get_groups();

            if (index == std::numeric_limits<size_t>::max() || index + 1 >= groups.size())
                break;

            g_groups.swap(index, index + 1);

            const std::vector<uih::ListView::InsertItem> insert_items{
                {{groups[index].string}, {}}, {{groups[index + 1].string}, {}}};
            m_groups_list_view.replace_items(index, insert_items.size(), insert_items.data());
            m_groups_list_view.set_item_selected_single(index + 1);
            m_groups_list_view.ensure_visible(index + 1);
            break;
        }
        case IDC_GROUP_DELETE: {
            const auto index = m_groups_list_view.get_selected_item_single();
            auto& groups = g_groups.get_groups();

            if (index >= groups.size())
                break;

            g_groups.remove_group(index);
            m_groups_list_view.remove_item(index);

            if (index > 0 && index == groups.size()) {
                m_groups_list_view.set_item_selected_single(index - 1);
            } else if (groups.size() > 0) {
                m_groups_list_view.set_item_selected_single(index);
            }
            break;
        }
        case IDC_GROUP_NEW: {
            edit_view_param p;
            p.b_new = true;
            if (run_edit_view(p, wnd)) {
                const auto n = g_groups.add_group(Group(p.value));
                const std::vector<uih::ListView::InsertItem> insert_items{{{p.value.string}, {}}};
                m_groups_list_view.insert_items(n, 1, insert_items.data());
                m_groups_list_view.set_item_selected_single(n);
                m_groups_list_view.ensure_visible(n);
            }
        } break;
        }
        break;
    case WM_DESTROY:
        m_groups_list_view.destroy();
        m_wnd = nullptr;
        break;
    }
    return 0;
}

void GroupsPreferencesTab::refresh_enabled_options() const
{
    EnableWindow(GetDlgItem(m_wnd, IDC_INDENT_GROUPS), cfg_grouping);
    EnableWindow(GetDlgItem(m_wnd, IDC_USE_CUSTOM_INDENTATION), cfg_grouping && cfg_indent_groups);
    const auto enable_custom_indentation_amount
        = cfg_grouping && cfg_indent_groups && cfg_use_custom_group_indentation_amount;
    EnableWindow(GetDlgItem(m_wnd, IDC_INDENTATION_AMOUNT), enable_custom_indentation_amount);
    EnableWindow(GetDlgItem(m_wnd, IDC_INDENTATION_AMOUNT_PX_STATIC), enable_custom_indentation_amount);
    EnableWindow(GetDlgItem(m_wnd, IDC_INDENTATION_AMOUNT_SPIN), enable_custom_indentation_amount);
    EnableWindow(GetDlgItem(m_wnd, IDC_ROOT_INDENTATION_STATIC), cfg_grouping);
    EnableWindow(GetDlgItem(m_wnd, IDC_ROOT_INDENTATION_PX_STATIC), cfg_grouping);
    EnableWindow(GetDlgItem(m_wnd, IDC_ROOT_INDENTATION_AMOUNT), cfg_grouping);
    EnableWindow(GetDlgItem(m_wnd, IDC_ROOT_INDENTATION_AMOUNT_SPIN), cfg_grouping);
}

void GroupsPreferencesTab::on_group_default_action(size_t index)
{
    edit_view_param p;
    p.b_new = false;
    p.value = g_groups.get_groups()[index];
    if (run_edit_view(p, m_wnd)) {
        g_groups.replace_group(index, p.value);
        const std::vector<uih::ListView::InsertItem> insert_items{{{p.value.string}, {}}};
        m_groups_list_view.replace_items(index, 1, insert_items.data());
    }
}

} // namespace cui::panels::playlist_view
