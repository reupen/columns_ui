#include "../stdafx.h"

#include "ng_playlist.h"
#include "ng_playlist_groups.h"

namespace pvt
{

    //CONFIG

    // {5738843B-15BA-4827-B7B7-E9970322FFFF}
    const GUID g_preferences_page_guid = 
    { 0x5738843b, 0x15ba, 0x4827, { 0xb7, 0xb7, 0xe9, 0x97, 0x3, 0x22, 0xff, 0xff } };

    struct edit_view_param
    {
        unsigned idx;
        group_t value;
        bool b_new;
    };

    static BOOL CALLBACK EditViewProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
    {
        switch(msg)
        {
        case WM_INITDIALOG:
            SetWindowLongPtr(wnd,DWLP_USER,lp);
            {
                edit_view_param * ptr = reinterpret_cast<edit_view_param*>(lp);
                SetWindowText(wnd, ptr->b_new ? L"Add New Group" : L"Edit Group");

                uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_ADDSTRING, 0, "Show on all playlists");
                uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_ADDSTRING, 0, "Show only on playlists:");
                uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_ADDSTRING, 0, "Hide on playlists:");

                EnableWindow(GetDlgItem(wnd, IDC_PLAYLIST_FILTER_STRING), ptr->value.filter_type != FILTER_NONE);

                SendDlgItemMessage(wnd,IDC_PLAYLIST_FILTER_TYPE,CB_SETCURSEL,(t_size)ptr->value.filter_type,0);
                uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_STRING, WM_SETTEXT, 0, ptr->value.filter_playlists);

                uSetDlgItemText(wnd,IDC_VALUE,ptr->value.string);
            }
            break;
        case WM_COMMAND:
            switch(wp)
            {
            case IDCANCEL:
                EndDialog(wnd,0);
                break;
            case (CBN_SELCHANGE<<16)|IDC_PLAYLIST_FILTER_TYPE:
                {
                    if (1) 
                    {
                        EnableWindow(GetDlgItem(wnd, IDC_PLAYLIST_FILTER_STRING), ((playlist_filter_type)SendMessage((HWND)lp,CB_GETCURSEL,0,0)) != FILTER_NONE);
                    }
                }
                break;
            case IDOK:
                {
                    edit_view_param * ptr = reinterpret_cast<edit_view_param*>(GetWindowLongPtr(wnd,DWLP_USER));
                    uGetDlgItemText(wnd,IDC_VALUE,ptr->value.string);
                    ptr->value.filter_type = ((playlist_filter_type)SendDlgItemMessage(wnd, IDC_PLAYLIST_FILTER_TYPE,CB_GETCURSEL,0,0));
                    ptr->value.filter_playlists = (string_utf8_from_window(wnd, IDC_PLAYLIST_FILTER_STRING));
                    EndDialog(wnd,1);

                }
                break;            
            }
            break;
        }
        return FALSE;
    }

    static bool run_edit_view(edit_view_param & param,HWND parent)
    {
        return uDialogBox(IDD_EDIT_GROUP,parent,EditViewProc,reinterpret_cast<LPARAM>(&param)) != 0;
    }

    BOOL CALLBACK preferences_tab_impl::ConfigProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
    {
        switch(msg)
        {

        case WM_INITDIALOG:

            {
                HWND list = uGetDlgItem(wnd,IDC_GROUPS);

                ListView_SetExtendedListViewStyleEx(list, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
                uih::SetListViewWindowExplorerTheme(list);

                Button_SetCheck(GetDlgItem(wnd, IDC_GROUPING), cfg_grouping ? BST_CHECKED : BST_UNCHECKED);
                uih::ListView_InsertColumnText(list, 0, _T("Script"), 400);

                unsigned n,m = g_groups.get_groups().get_count();
                pfc::string8_fastalloc temp;
                for(n=0;n<m;n++)
                {
                    uih::ListView_InsertItemText(list, n, 0, g_groups.get_groups()[n].string.get_ptr());
                }
            }
            //initialised=true;

            break;
        case WM_NOTIFY:
            {
                LPNMHDR lpnm = (LPNMHDR)lp;
                if (lpnm->idFrom == IDC_GROUPS)
                {
                    if (lpnm->code == NM_DBLCLK)
                    {
                        LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lp;
                        if (lpnmia->iItem != -1 && (t_size)lpnmia->iItem < g_groups.get_groups().get_count())
                        {
                            edit_view_param p;
                            p.b_new = false;
                            p.idx = lpnmia->iItem;
                            p.value = g_groups.get_groups()[lpnmia->iItem];
                            if (run_edit_view(p,wnd))
                            {
                                g_groups.replace_group(lpnmia->iItem, p.value);
                                uih::ListView_InsertItemText(lpnm->hwndFrom, lpnmia->iItem, 0, p.value.string, true);
                            }
                        }

                    }
                }
            }
            break;
        case WM_COMMAND:
            switch(wp)
            {
            case IDC_GROUPING:
                {
                    cfg_grouping = Button_GetCheck(HWND(lp)) == BST_CHECKED;
                    ng_playlist_view_t::g_on_groups_change();
                }
                break;
            case IDC_GROUP_UP:
                {
                    HWND list = uGetDlgItem(wnd,IDC_GROUPS);
                    unsigned idx = ListView_GetNextItem(list, -1, LVNI_SELECTED);
                    if (idx != LB_ERR && idx>0)
                    {
                        g_groups.swap(idx,idx-1);
                        uih::ListView_InsertItemText(list, idx, 0, g_groups.get_groups()[idx].string.get_ptr(), true);
                        uih::ListView_InsertItemText(list, idx-1, 0, g_groups.get_groups()[idx-1].string.get_ptr(), true);
                        ListView_SetItemState(list, idx-1, LVIS_SELECTED, LVIS_SELECTED);
                    }
                }
                break;
            case IDC_GROUP_DOWN:
                {
                    HWND list = uGetDlgItem(wnd,IDC_GROUPS);
                    unsigned idx = ListView_GetNextItem(list, -1, LVNI_SELECTED);
                    if (idx != LB_ERR && idx+1<g_groups.get_groups().get_count())
                    {
                        g_groups.swap(idx,idx+1);
                        uih::ListView_InsertItemText(list, idx, 0, g_groups.get_groups()[idx].string.get_ptr(), true);
                        uih::ListView_InsertItemText(list, idx+1, 0, g_groups.get_groups()[idx+1].string.get_ptr(), true);
                        ListView_SetItemState(list, idx+1, LVIS_SELECTED, LVIS_SELECTED);
                    }
                }
                break;
            case IDC_GROUP_DELETE:
                {
                    HWND list = uGetDlgItem(wnd,IDC_GROUPS);
                    unsigned idx = ListView_GetNextItem(list, -1, LVNI_SELECTED);
                    if (idx != -1)
                    {
                        g_groups.remove_group(idx);
                        ListView_DeleteItem(list, idx);
                        if (idx && ListView_GetNextItem(list, -1, LVNI_SELECTED) == -1)
                            ListView_SetItemState(list, idx-1, LVIS_SELECTED, LVIS_SELECTED);
                    }
                }
                break;
            case IDC_GROUP_NEW:
                {
                    edit_view_param p;
                    p.b_new = true;
                    p.idx = -1;
                    if (run_edit_view(p,wnd))
                    {
                        HWND list = uGetDlgItem(wnd,IDC_GROUPS);
                        unsigned n = g_groups.add_group(group_t(p.value));
                        uih::ListView_InsertItemText(list, n, 0, p.value.string.get_ptr());
                        ListView_SetItemState(list, n, LVIS_SELECTED, LVIS_SELECTED);
                    }
                }
                break;
            }
            break;
        case WM_DESTROY:
            //initialised=false;
            break;
        }
        return 0;
    }

}