#include "pch.h"

#include "ng_playlist.h"
#include "ng_playlist_groups.h"

namespace cui::panels::playlist_view {

// CONFIG

struct edit_view_param {
    Group value;
    bool b_new{};
};

static INT_PTR CALLBACK EditViewProc(edit_view_param& state, HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        SetWindowText(wnd, state.b_new ? L"Add New Group" : L"Edit Group");

        uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_ADDSTRING, 0, "Show on all playlists");
        uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_ADDSTRING, 0, "Show only on playlists:");
        uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_ADDSTRING, 0, "Hide on playlists:");

        EnableWindow(GetDlgItem(wnd, IDC_PLAYLIST_FILTER_STRING), state.value.filter_type != FILTER_NONE);

        SendDlgItemMessage(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_SETCURSEL, (size_t)state.value.filter_type, 0);
        uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_STRING, WM_SETTEXT, 0, state.value.filter_playlists);

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
    const auto dialog_result = uih::modal_dialog_box(IDD_EDIT_GROUP, parent,
        [&param](auto&&... args) { return EditViewProc(param, std::forward<decltype(args)>(args)...); });

    return dialog_result > 0;
}

BOOL GroupsPreferencesTab::ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        m_wnd = wnd;
        m_groups_list_view.create(wnd, {7, 51, 313, 195}, true);

        LOGFONT font{};
        GetObject(GetWindowFont(wnd), sizeof(font), &font);
        m_groups_list_view.set_font(&font);

        std::vector<uih::ListView::InsertItem> insert_items;
        insert_items.reserve(g_groups.get_groups().get_count());

        auto& groups = g_groups.get_groups();
        std::transform(groups.begin(), groups.end(), std::back_inserter(insert_items), [](const auto& group) {
            return uih::ListView::InsertItem{{group.string}, {}};
        });

        m_groups_list_view.insert_items(0, insert_items.size(), insert_items.data());
        ShowWindow(m_groups_list_view.get_wnd(), SW_SHOWNORMAL);

        Button_SetCheck(GetDlgItem(wnd, IDC_GROUPING), cfg_grouping ? BST_CHECKED : BST_UNCHECKED);

        break;
    }
    case WM_COMMAND:
        switch (wp) {
        case IDC_GROUPING: {
            cfg_grouping = Button_GetCheck(HWND(lp)) == BST_CHECKED;
            PlaylistView::g_on_groups_change();
        } break;
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

            if (index + 1 >= groups.size())
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
