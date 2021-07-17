#include "stdafx.h"
#include "ng_playlist/ng_playlist.h"
#include "config.h"
#include "config_columns_v2.h"
#include "help.h"
#include "prefs_utils.h"

extern cfg_int g_cur_tab;
extern cfg_uint g_last_colour;

enum { MSG_COLUMN_NAME_CHANGED = WM_USER + 2, MSG_SELECTION_CHANGED };
struct ColumnTimes {
    service_ptr_t<titleformat_object> to_display;
    service_ptr_t<titleformat_object> to_colour;
    double time_display_compile;
    double time_colour_compile;
    double time_display;
    double time_colour;
};

class EditColumnWindowOptions : public ColumnTab {
public:
    void get_column(PlaylistViewColumn::ptr& p_out) override { p_out = m_column; };
    using self_t = EditColumnWindowOptions;
    HWND create(HWND wnd) override
    {
        return CreateDialogParam(mmh::get_current_instance(), MAKEINTRESOURCE(IDD_COLUMN_OPTIONS), wnd, g_on_message,
            reinterpret_cast<LPARAM>(this));
    }
    // virtual const char * get_name()=0;
    EditColumnWindowOptions(PlaylistViewColumn::ptr column)
        : initialising(false)
        , editproc(nullptr)
        , m_wnd(nullptr)
        , m_column(std::move(column)){};

    bool initialising;
    WNDPROC editproc;
    HWND m_wnd;

    PlaylistViewColumn::ptr m_column;

    void set_detail_enabled(HWND wnd, BOOL show)
    {
        if (show == FALSE) {
            pfc::vartoggle_t<bool> initialising_toggle(initialising, true);

            uSendDlgItemMessageText(wnd, IDC_NAME, WM_SETTEXT, 0, "");
            uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_STRING, WM_SETTEXT, 0, "");
            uSendDlgItemMessageText(wnd, IDC_EDITFIELD, WM_SETTEXT, 0, "");
            SetDlgItemInt(wnd, IDC_WIDTH, 0, false);
            SetDlgItemInt(wnd, IDC_PARTS, 0, false);
            SendDlgItemMessage(wnd, IDC_SHOW_COLUMN, BM_SETCHECK, 0, 0);
            SendDlgItemMessage(wnd, IDC_ALIGNMENT, CB_SETCURSEL, 0, 0);
            SendDlgItemMessage(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_SETCURSEL, 0, 0);
        }

        EnableWindow(GetDlgItem(wnd, IDC_STRING), show);
        EnableWindow(GetDlgItem(wnd, IDC_NAME), show);
        EnableWindow(GetDlgItem(wnd, IDC_WIDTH), show);
        EnableWindow(GetDlgItem(wnd, IDC_PARTS), show);
        EnableWindow(GetDlgItem(wnd, IDC_SHOW_COLUMN), show);
        EnableWindow(GetDlgItem(wnd, IDC_ALIGNMENT), show);
        EnableWindow(
            GetDlgItem(wnd, IDC_PLAYLIST_FILTER_STRING), show && m_column && m_column->filter_type != FILTER_NONE);
        EnableWindow(GetDlgItem(wnd, IDC_PLAYLIST_FILTER_TYPE), show);
        EnableWindow(GetDlgItem(wnd, IDC_EDITFIELD), show);
    }

    void refresh_me(HWND wnd, bool init = false)
    {
        initialising = true;

        if (m_column) {
            uSendDlgItemMessageText(wnd, IDC_NAME, WM_SETTEXT, 0, m_column->name);
            uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_STRING, WM_SETTEXT, 0, m_column->filter);
            uSendDlgItemMessageText(wnd, IDC_EDITFIELD, WM_SETTEXT, 0, m_column->edit_field);

            SendDlgItemMessage(wnd, IDC_SHOW_COLUMN, BM_SETCHECK, m_column->show, 0);
            SendDlgItemMessage(wnd, IDC_ALIGNMENT, CB_SETCURSEL, (t_size)m_column->align, 0);
            SendDlgItemMessage(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_SETCURSEL, (t_size)m_column->filter_type, 0);

            SetDlgItemInt(wnd, IDC_WIDTH, m_column->width, false);
            SetDlgItemInt(wnd, IDC_PARTS, m_column->parts, false);
        }

        initialising = false;

        set_detail_enabled(wnd, static_cast<bool>(m_column));
    }

    void set_column(const PlaylistViewColumn::ptr& column) override
    {
        if (m_column != column) {
            m_column = column;
            refresh_me(m_wnd);
        }
    }
    static BOOL CALLBACK g_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        self_t* p_data = nullptr;
        if (msg == WM_INITDIALOG) {
            p_data = reinterpret_cast<self_t*>(lp);
            SetWindowLongPtr(wnd, DWLP_USER, lp);
        } else
            p_data = reinterpret_cast<self_t*>(GetWindowLongPtr(wnd, DWLP_USER));
        return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
    }

    BOOL CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            m_wnd = wnd;
            uTCITEM tabs;
            memset(&tabs, 0, sizeof(tabs));

            uSendDlgItemMessageText(wnd, IDC_ALIGNMENT, CB_ADDSTRING, 0, "Left");
            uSendDlgItemMessageText(wnd, IDC_ALIGNMENT, CB_ADDSTRING, 0, "Centre");
            uSendDlgItemMessageText(wnd, IDC_ALIGNMENT, CB_ADDSTRING, 0, "Right");

            uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_ADDSTRING, 0, "Show on all playlists");
            uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_ADDSTRING, 0, "Show only on playlists:");
            uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_ADDSTRING, 0, "Hide on playlists:");

            SendDlgItemMessage(wnd, IDC_STRING, EM_LIMITTEXT, 0, 0);

            refresh_me(wnd, true);
        }

        break;
        case WM_DESTROY:
            m_wnd = nullptr;
            break;

        case WM_COMMAND:
            switch (wp) {
            case (CBN_SELCHANGE << 16) | IDC_ALIGNMENT: {
                if (!initialising && m_column) {
                    m_column->align = ((Alignment)SendMessage((HWND)lp, CB_GETCURSEL, 0, 0));
                }
            } break;
            case (CBN_SELCHANGE << 16) | IDC_PLAYLIST_FILTER_TYPE: {
                if (!initialising && m_column) {
                    m_column->filter_type = ((PlaylistFilterType)SendMessage((HWND)lp, CB_GETCURSEL, 0, 0));
                    EnableWindow(GetDlgItem(wnd, IDC_PLAYLIST_FILTER_STRING), m_column->filter_type != FILTER_NONE);
                }
            } break;
            case IDC_SHOW_COLUMN: {
                if (!initialising && m_column) {
                    m_column->show = ((SendMessage((HWND)lp, BM_GETCHECK, 0, 0) != 0));
                }
            } break;
            case (EN_CHANGE << 16) | IDC_SORT: {
                if (!initialising && m_column) {
                    m_column->sort_spec = (string_utf8_from_window((HWND)lp));
                }
            } break;
            case IDC_PICK_COLOUR:
                colour_code_gen(wnd, IDC_COLOUR, true, false);
                break;
            case (EN_CHANGE << 16) | IDC_WIDTH: {
                if (!initialising && m_column) {
                    m_column->width = (GetDlgItemInt(wnd, IDC_WIDTH, nullptr, false));
                }
            } break;
            case (EN_CHANGE << 16) | IDC_PARTS: {
                if (!initialising && m_column) {
                    m_column->parts = (GetDlgItemInt(wnd, IDC_PARTS, nullptr, false));
                }
            } break;
            case (EN_CHANGE << 16) | IDC_PLAYLIST_FILTER_STRING: {
                if (!initialising && m_column) {
                    m_column->filter = (string_utf8_from_window((HWND)lp));
                }
            } break;
            case (EN_CHANGE << 16) | IDC_EDITFIELD: {
                if (!initialising && m_column) {
                    m_column->edit_field = (string_utf8_from_window((HWND)lp));
                }
            } break;
            case (EN_CHANGE << 16) | IDC_NAME: {
                if (!initialising && m_column) {
                    m_column->name = string_utf8_from_window((HWND)lp);
                    SendMessage(GetAncestor(wnd, GA_PARENT), MSG_COLUMN_NAME_CHANGED, NULL, NULL);
                }
            } break;
            }
        }
        return 0;
    }
};

void show_title_formatting_help_menu(HWND wnd, unsigned edit_ctrl_id)
{
    RECT rc;
    GetWindowRect(GetDlgItem(wnd, IDC_TFHELP), &rc);
    const HMENU menu = CreatePopupMenu();

    enum { IDM_TFHELP = 1, IDM_STYLE_HELP, IDM_GLOBALS_HELP, IDM_SPEEDTEST, IDM_PREVIEW, IDM_EDITORFONT };

    uAppendMenu(menu, MF_STRING, IDM_TFHELP, "Title formatting &help");
    uAppendMenu(menu, MF_STRING, IDM_STYLE_HELP, "&Style script help");
    uAppendMenu(menu, MF_STRING, IDM_GLOBALS_HELP, "&Global variables help");
    uAppendMenu(menu, MF_SEPARATOR, 0, "");
    uAppendMenu(menu, MF_STRING, IDM_SPEEDTEST, "Speed &test");
    uAppendMenu(menu, MF_STRING, IDM_PREVIEW, "&Preview script");
    uAppendMenu(menu, MF_SEPARATOR, 0, "");
    uAppendMenu(menu, MF_STRING, IDM_EDITORFONT, "Change editor &font");

    const int cmd
        = TrackPopupMenu(menu, TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, rc.left, rc.bottom, 0, wnd, nullptr);
    DestroyMenu(menu);
    if (cmd == IDM_TFHELP) {
        standard_commands::main_titleformat_help();
    } else if (cmd == IDM_STYLE_HELP) {
        cui::help::open_colour_script_help(GetParent(wnd));
    } else if (cmd == IDM_GLOBALS_HELP) {
        cui::help::open_global_variables_help(GetParent(wnd));
    } else if (cmd == IDM_SPEEDTEST) {
        speedtest(g_columns, cfg_global != 0);
    } else if (cmd == IDM_PREVIEW) {
        preview_to_console(string_utf8_from_window(wnd, edit_ctrl_id), cfg_global != 0);
    } else if (cmd == IDM_EDITORFONT) {
        auto font_description = cui::fonts::select_font(GetParent(wnd), cfg_editor_font->log_font);
        if (font_description) {
            cfg_editor_font = *font_description;
            g_editor_font_notify.on_change();
        }
    }
}

class DisplayScriptTab : public ColumnTab {
public:
    using SelfType = DisplayScriptTab;

    void get_column(PlaylistViewColumn::ptr& p_out) override { p_out = m_column; };

    HWND create(HWND wnd) override
    {
        return CreateDialogParam(mmh::get_current_instance(), MAKEINTRESOURCE(IDD_COLUMN_DISPLAY_SCRIPT), wnd,
            s_on_message, reinterpret_cast<LPARAM>(this));
    }

    void set_column(const PlaylistViewColumn::ptr& column) override
    {
        if (m_column != column) {
            m_column = column;
            update_controls();
        }
    }

    explicit DisplayScriptTab(PlaylistViewColumn::ptr col)
        : m_wnd(nullptr)
        , initialising(false)
        , m_column(std::move(col))
    {
    }

private:
    static BOOL CALLBACK s_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        SelfType* p_data = nullptr;
        if (msg == WM_INITDIALOG) {
            p_data = reinterpret_cast<SelfType*>(lp);
            SetWindowLongPtr(wnd, DWLP_USER, lp);
        } else
            p_data = reinterpret_cast<SelfType*>(GetWindowLongPtr(wnd, DWLP_USER));
        return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
    }

    void update_controls()
    {
        pfc::vartoggle_t<bool> initialising_toggle(initialising, true);

        if (m_column) {
            uSetWindowText(edit_control(), m_column->spec);
        } else {
            uSendMessageText(edit_control(), WM_SETTEXT, 0, "");
        }
        EnableWindow(edit_control(), m_column ? TRUE : FALSE);
    }

    BOOL CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG:
            m_wnd = wnd;
            Edit_LimitText(edit_control(), 0);
            m_edit_control_hook.attach(edit_control());
            g_editor_font_notify.set(edit_control());
            colour_code_gen(wnd, IDC_COLOUR, true, true);
            update_controls();
            break;
        case WM_DESTROY:
            m_wnd = nullptr;
            g_editor_font_notify.release();
            break;
        case WM_COMMAND:
            switch (wp) {
            case IDC_TFHELP:
                show_title_formatting_help_menu(wnd, IDC_DISPLAY_SCRIPT);
                break;
            case IDC_PICK_COLOUR:
                colour_code_gen(wnd, IDC_COLOUR, true, false);
                break;
            case EN_CHANGE << 16 | IDC_DISPLAY_SCRIPT:
                if (!initialising && m_column)
                    m_column->spec = string_utf8_from_window(reinterpret_cast<HWND>(lp));
                break;
            }
        }
        return 0;
    }

    HWND edit_control() const { return GetDlgItem(m_wnd, IDC_DISPLAY_SCRIPT); }

    cui::prefs::EditControlSelectAllHook m_edit_control_hook;
    HWND m_wnd;
    bool initialising;
    PlaylistViewColumn::ptr m_column;
};

class StyleScriptTab : public ColumnTab {
public:
    using SelfType = StyleScriptTab;

    void get_column(PlaylistViewColumn::ptr& p_out) override { p_out = m_column; };

    HWND create(HWND wnd) override
    {
        return CreateDialogParam(mmh::get_current_instance(), MAKEINTRESOURCE(IDD_COLUMN_STYLE_SCRIPT), wnd,
            s_on_message, reinterpret_cast<LPARAM>(this));
    }

    void set_column(const PlaylistViewColumn::ptr& column) override
    {
        if (m_column != column) {
            m_column = column;
            update_controls();
        }
    }

    explicit StyleScriptTab(PlaylistViewColumn::ptr col) : m_wnd(nullptr), initialising(false), m_column(std::move(col))
    {
    }

private:
    static BOOL CALLBACK s_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        SelfType* p_data = nullptr;
        if (msg == WM_INITDIALOG) {
            p_data = reinterpret_cast<SelfType*>(lp);
            SetWindowLongPtr(wnd, DWLP_USER, lp);
        } else
            p_data = reinterpret_cast<SelfType*>(GetWindowLongPtr(wnd, DWLP_USER));
        return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
    }

    void update_controls()
    {
        pfc::vartoggle_t<bool> initialising_toggle(initialising, true);

        if (m_column) {
            uSetWindowText(edit_control(), m_column->colour_spec);
            Button_SetCheck(custom_colour_control(), m_column->use_custom_colour ? BST_CHECKED : BST_UNCHECKED);
        } else {
            uSendMessageText(edit_control(), WM_SETTEXT, 0, "");
            Button_SetCheck(custom_colour_control(), BST_UNCHECKED);
        }
        EnableWindow(edit_control(), m_column ? TRUE : FALSE);
        EnableWindow(custom_colour_control(), m_column ? TRUE : FALSE);
    }

    BOOL CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG:
            m_wnd = wnd;
            colour_code_gen(wnd, IDC_COLOUR, true, true);
            Edit_LimitText(edit_control(), 0);
            m_edit_control_hook.attach(edit_control());
            g_editor_font_notify.set(edit_control());
            update_controls();
            break;
        case WM_DESTROY:
            m_wnd = nullptr;
            g_editor_font_notify.release();
            break;
        case WM_COMMAND:
            switch (wp) {
            case IDC_TFHELP:
                show_title_formatting_help_menu(wnd, IDC_STYLE_SCRIPT);
                break;
            case IDC_PICK_COLOUR:
                colour_code_gen(wnd, IDC_COLOUR, true, false);
                break;
            case IDC_CUSTOM_COLOUR:
                if (!initialising && m_column) {
                    m_column->use_custom_colour = Button_GetCheck(reinterpret_cast<HWND>(lp)) != BST_UNCHECKED;
                }
                break;
            case EN_CHANGE << 16 | IDC_STYLE_SCRIPT:
                if (!initialising && m_column)
                    m_column->colour_spec = string_utf8_from_window(reinterpret_cast<HWND>(lp));
                break;
            }
        }
        return 0;
    }

    HWND edit_control() const { return GetDlgItem(m_wnd, IDC_STYLE_SCRIPT); }
    HWND custom_colour_control() const { return GetDlgItem(m_wnd, IDC_CUSTOM_COLOUR); }

    cui::prefs::EditControlSelectAllHook m_edit_control_hook;
    HWND m_wnd;
    bool initialising;
    PlaylistViewColumn::ptr m_column;
};

class SortingScriptTab : public ColumnTab {
public:
    using SelfType = SortingScriptTab;

    void get_column(PlaylistViewColumn::ptr& p_out) override { p_out = m_column; };

    HWND create(HWND wnd) override
    {
        return CreateDialogParam(mmh::get_current_instance(), MAKEINTRESOURCE(IDD_COLUMN_SORTING_SCRIPT), wnd,
            s_on_message, reinterpret_cast<LPARAM>(this));
    }

    void set_column(const PlaylistViewColumn::ptr& column) override
    {
        if (m_column != column) {
            m_column = column;
            update_controls();
        }
    }

    explicit SortingScriptTab(PlaylistViewColumn::ptr col)
        : m_wnd(nullptr)
        , initialising(false)
        , m_column(std::move(col))
    {
    }

private:
    static BOOL CALLBACK s_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        SelfType* p_data = nullptr;
        if (msg == WM_INITDIALOG) {
            p_data = reinterpret_cast<SelfType*>(lp);
            SetWindowLongPtr(wnd, DWLP_USER, lp);
        } else
            p_data = reinterpret_cast<SelfType*>(GetWindowLongPtr(wnd, DWLP_USER));
        return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
    }

    void update_controls()
    {
        pfc::vartoggle_t<bool> initialising_toggle(initialising, true);

        if (m_column) {
            uSetWindowText(edit_control(), m_column->sort_spec);
            Button_SetCheck(custom_sorting_control(), m_column->use_custom_sort ? BST_CHECKED : BST_UNCHECKED);
        } else {
            uSendMessageText(edit_control(), WM_SETTEXT, 0, "");
            Button_SetCheck(custom_sorting_control(), BST_UNCHECKED);
        }
        EnableWindow(edit_control(), m_column ? TRUE : FALSE);
        EnableWindow(custom_sorting_control(), m_column ? TRUE : FALSE);
    }

    BOOL CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG:
            m_wnd = wnd;
            Edit_LimitText(edit_control(), 0);
            m_edit_control_hook.attach(edit_control());
            g_editor_font_notify.set(edit_control());
            update_controls();
            break;

        case WM_DESTROY:
            m_wnd = nullptr;
            g_editor_font_notify.release();
            break;
        case WM_COMMAND:
            switch (wp) {
            case IDC_TFHELP:
                show_title_formatting_help_menu(wnd, IDC_SORTING_SCRIPT);
                break;
            case IDC_CUSTOM_SORT:
                if (!initialising && m_column) {
                    m_column->use_custom_sort = Button_GetCheck(reinterpret_cast<HWND>(lp)) != BST_UNCHECKED;
                }
                break;
            case EN_CHANGE << 16 | IDC_SORTING_SCRIPT:
                if (!initialising && m_column)
                    m_column->sort_spec = string_utf8_from_window(reinterpret_cast<HWND>(lp));
                break;
            }
        }
        return 0;
    }

    HWND edit_control() const { return GetDlgItem(m_wnd, IDC_SORTING_SCRIPT); }
    HWND custom_sorting_control() const { return GetDlgItem(m_wnd, IDC_CUSTOM_SORT); }

    cui::prefs::EditControlSelectAllHook m_edit_control_hook;
    HWND m_wnd;
    bool initialising;
    PlaylistViewColumn::ptr m_column;
};

// {0A7A2845-06A4-4c15-B09F-A6EBEE86335D}
const GUID g_guid_cfg_child_column = {0xa7a2845, 0x6a4, 0x4c15, {0xb0, 0x9f, 0xa6, 0xeb, 0xee, 0x86, 0x33, 0x5d}};

cfg_uint cfg_child_column(g_guid_cfg_child_column, 0);

void TabColumns::make_child()
{
    // HWND wnd_destroy = child;
    if (m_wnd_child) {
        ShowWindow(m_wnd_child, SW_HIDE);
        DestroyWindow(m_wnd_child);
        m_wnd_child = nullptr;
        m_child.reset();
    }

    HWND wnd_tab = GetDlgItem(m_wnd, IDC_TAB1);

    RECT tab;

    GetWindowRect(wnd_tab, &tab);
    MapWindowPoints(HWND_DESKTOP, m_wnd, (LPPOINT)&tab, 2);

    TabCtrl_AdjustRect(wnd_tab, FALSE, &tab);

    unsigned count = 4;
    if (cfg_child_column >= count)
        cfg_child_column = 0;

    if (cfg_child_column < count && cfg_child_column >= 0) {
        int item = ListView_GetNextItem(GetDlgItem(m_wnd, IDC_COLUMNS), -1, LVNI_SELECTED);

        PlaylistViewColumn::ptr column;
        if (item != -1)
            column = m_columns[item];

        if (cfg_child_column == 0)
            m_child = std::make_unique<EditColumnWindowOptions>(column);
        else if (cfg_child_column == 1)
            m_child = std::make_unique<DisplayScriptTab>(column);
        else if (cfg_child_column == 2)
            m_child = std::make_unique<StyleScriptTab>(column);
        else if (cfg_child_column == 3)
            m_child = std::make_unique<SortingScriptTab>(column);
        m_wnd_child = m_child->create(m_wnd);
    }

    if (m_wnd_child) {
        EnableThemeDialogTexture(m_wnd_child, ETDT_ENABLETAB);
        SetWindowPos(m_wnd_child, HWND_TOP, tab.left, tab.top, tab.right - tab.left, tab.bottom - tab.top, NULL);
        ShowWindow(m_wnd_child, SW_SHOWNORMAL);
    }

    // SetWindowPos(wnd_tab,GetDlgItem(m_wnd, IDC_GROUPBOX),0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
}

void TabColumns::refresh_me(HWND wnd, bool init)
{
    initialising = true;
    HWND wnd_lv = GetDlgItem(wnd, IDC_COLUMNS);

    SendDlgItemMessage(wnd, IDC_COLUMNS, WM_SETREDRAW, false, 0);
    int idx = (init ? cfg_cur_prefs_col : ListView_GetNextItem(wnd_lv, -1, LVNI_SELECTED));
    ListView_DeleteAllItems(wnd_lv);

    pfc::string8 temp;

    int t = m_columns.get_count();
    for (int i = 0; i < t; i++) {
        uih::list_view_insert_item_text(wnd_lv, i, 0, m_columns[i]->name);
    }

    SendDlgItemMessage(wnd, IDC_COLUMNS, WM_SETREDRAW, true, 0);
    initialising = false;

    if (idx >= 0 && idx < (int)m_columns.get_count()) {
        ListView_SetItemState(wnd_lv, idx, LVIS_SELECTED, LVIS_SELECTED);
        ListView_EnsureVisible(wnd_lv, idx, FALSE);
    }

    RECT rc_lv;
    GetClientRect(wnd_lv, &rc_lv);
    ListView_SetColumnWidth(wnd_lv, 0, RECT_CX(rc_lv));
}

BOOL TabColumns::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        m_wnd = wnd;
        // if (g_main_window && !cfg_nohscroll ) playlist_view::g_save_columns();
        m_wnd_lv = GetDlgItem(wnd, IDC_COLUMNS);
        uih::list_view_set_explorer_theme(m_wnd_lv);
        ListView_SetExtendedListViewStyleEx(m_wnd_lv, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
        uih::list_view_insert_column_text(m_wnd_lv, 0, L"Column", 50);

        m_columns.set_entries_copy(g_columns, true);

        refresh_me(wnd, true);

        HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);
        uTabCtrl_InsertItemText(wnd_tab, 0, "Options");
        uTabCtrl_InsertItemText(wnd_tab, 1, "Display script");
        uTabCtrl_InsertItemText(wnd_tab, 2, "Style script");
        uTabCtrl_InsertItemText(wnd_tab, 3, "Sorting script");

        TabCtrl_SetCurSel(wnd_tab, cfg_child_column);

        make_child();
    }

    break;
    case WM_CONTEXTMENU:
        if (HWND(wp) == GetDlgItem(wnd, IDC_COLUMNS)) {
            enum { ID_REMOVE = 1, ID_UP, ID_DOWN, ID_NEW };
            POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
            int item = ListView_GetNextItem(GetDlgItem(m_wnd, IDC_COLUMNS), -1, LVNI_SELECTED);
            // if (item != -1 && item >= 0)
            {
                HMENU menu = CreatePopupMenu();
                AppendMenu(menu, MF_STRING, ID_NEW, L"&New");
                if (item != -1)
                    AppendMenu(menu, MF_STRING, ID_REMOVE, L"&Remove");
                if (item != -1 && m_columns.get_count() > 1)
                    AppendMenu(menu, MF_SEPARATOR, NULL, nullptr);
                if (item > 0)
                    AppendMenu(menu, MF_STRING, ID_UP, L"Move &up");
                if (item >= 0 && (t_size(item + 1)) < m_columns.get_count())
                    AppendMenu(menu, MF_STRING, ID_DOWN, L"Move &down");

                int cmd
                    = TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, wnd, nullptr);
                DestroyMenu(menu);

                if (cmd) {
                    int& idx = item;
                    auto wnd_lv = HWND(wp);
                    if (cmd == ID_NEW) {
                        PlaylistViewColumn::ptr temp = std::make_shared<PlaylistViewColumn>();
                        temp->name = "New Column";
                        t_size insert = m_columns.insert_item(
                            temp, idx >= 0 && (t_size)idx < m_columns.get_count() ? idx : m_columns.get_count());
                        uih::list_view_insert_item_text(wnd_lv, insert, 0, "New Column");
                        ListView_SetItemState(wnd_lv, insert, LVIS_SELECTED, LVIS_SELECTED);
                        ListView_EnsureVisible(wnd_lv, insert, FALSE);
                    } else if (idx >= 0 && (t_size)idx < m_columns.get_count()) {
                        if (cmd == ID_REMOVE) {
                            m_columns.remove_by_idx(idx);
                            t_size new_count = m_columns.get_count();
                            ListView_DeleteItem(wnd_lv, idx);

                            if (idx > 0 && (t_size)idx == new_count)
                                idx--;
                            if (idx >= 0 && (t_size)idx < new_count)
                                ListView_SetItemState(wnd_lv, idx, LVIS_SELECTED, LVIS_SELECTED);
                            if (new_count == 0)
                                SendMessage(wnd, MSG_SELECTION_CHANGED, NULL, NULL);

                        } else if (cmd == ID_UP) {
                            if (idx > 0 && m_columns.move_up(idx)) {
                                uih::list_view_insert_item_text(wnd_lv, idx, 0, m_columns[idx]->name, true);
                                uih::list_view_insert_item_text(wnd_lv, idx - 1, 0, m_columns[idx - 1]->name, true);
                                ListView_SetItemState(wnd_lv, idx - 1, LVIS_SELECTED, LVIS_SELECTED);
                                ListView_EnsureVisible(wnd_lv, idx - 1, FALSE);
                            }
                        } else if (cmd == ID_DOWN) {
                            if ((t_size)(idx + 1) < m_columns.get_count() && m_columns.move_down(idx)) {
                                uih::list_view_insert_item_text(wnd_lv, idx, 0, m_columns[idx]->name, true);
                                uih::list_view_insert_item_text(wnd_lv, idx + 1, 0, m_columns[idx + 1]->name, true);
                                ListView_SetItemState(wnd_lv, idx + 1, LVIS_SELECTED, LVIS_SELECTED);
                                ListView_EnsureVisible(wnd_lv, idx + 1, FALSE);
                            }
                        }
                    }
                }
            }

            return 0;
        }
        break;

    case WM_DESTROY: {
        int idx = ListView_GetNextItem(m_wnd_lv, -1, LVNI_SELECTED);
        if (idx >= 0 && idx < (int)m_columns.get_count()) {
            cfg_cur_prefs_col = idx;
        }

        apply();
        m_columns.remove_all();
        m_wnd = nullptr;
        m_wnd_lv = nullptr;
        if (m_wnd_child) {
            DestroyWindow(m_wnd_child);
            m_wnd_child = nullptr;
            m_child.reset();
        }
    } break;
    case MSG_SELECTION_CHANGED: {
        int item = (ListView_GetNextItem(GetDlgItem(m_wnd, IDC_COLUMNS), -1, LVNI_SELECTED));
        m_child->set_column(item != -1 && item >= 0 && (t_size)item < m_columns.get_count()
                ? m_columns[item]
                : PlaylistViewColumn::ptr());
    }
        return 0;
    case MSG_COLUMN_NAME_CHANGED: {
        HWND wnd_lv = GetDlgItem(wnd, IDC_COLUMNS);
        int item = ListView_GetNextItem(wnd_lv, -1, LVNI_SELECTED);
        if (item >= 0 && (unsigned)item < m_columns.get_count())
            uih::list_view_insert_item_text(wnd_lv, item, 0, m_columns[item]->name, true);
    }
        return 0;
    case WM_NOTIFY: {
        auto lpnm = (LPNMHDR)lp;
        switch (lpnm->idFrom) {
        case IDC_COLUMNS: {
            switch (lpnm->code) {
            case LVN_ITEMCHANGED: {
                auto lpnmlv = (LPNMLISTVIEW)lp;
                if (m_child) {
                    if (lpnmlv->iItem != -1 && lpnmlv->iItem >= 0 && (t_size)lpnmlv->iItem < m_columns.get_count()) {
                        if ((lpnmlv->uNewState & LVIS_SELECTED) != (lpnmlv->uOldState & LVIS_SELECTED))
                            PostMessage(wnd, MSG_SELECTION_CHANGED, NULL, NULL);
                    }
                }
            }
                return 0;
            }
        } break;
        case IDC_TAB1:
            switch (((LPNMHDR)lp)->code) {
            case TCN_SELCHANGE: {
                cfg_child_column = TabCtrl_GetCurSel(GetDlgItem(wnd, IDC_TAB1));
                make_child();
            } break;
            }
            break;
        }
        break;
    } break;
    case WM_PARENTNOTIFY:
        switch (wp) {
        case WM_DESTROY: {
            if (m_wnd_child && (HWND)lp == m_wnd_child) {
                m_wnd_child = nullptr;
                // m_child.release();
            }
        } break;
        }
        break;
    case WM_COMMAND:
        switch (wp) {
        case IDC_APPLY:
            apply();
            break;
        case IDC_UP: {
            HWND wnd_lv = GetDlgItem(wnd, IDC_COLUMNS);
            int idx = ListView_GetNextItem(wnd_lv, -1, LVNI_SELECTED);
            if (idx >= 0 && idx > 0 && m_columns.move_up(idx)) {
                uih::list_view_insert_item_text(wnd_lv, idx, 0, m_columns[idx]->name, true);
                uih::list_view_insert_item_text(wnd_lv, idx - 1, 0, m_columns[idx - 1]->name, true);
                ListView_SetItemState(wnd_lv, idx - 1, LVIS_SELECTED, LVIS_SELECTED);
                ListView_EnsureVisible(wnd_lv, idx - 1, FALSE);
            }
            // apply();
        } break;
        case IDC_DOWN: {
            HWND wnd_lv = GetDlgItem(wnd, IDC_COLUMNS);
            int idx = ListView_GetNextItem(wnd_lv, -1, LVNI_SELECTED);
            if (idx >= 0 && (t_size(idx + 1)) < m_columns.get_count() && m_columns.move_down(idx)) {
                uih::list_view_insert_item_text(wnd_lv, idx, 0, m_columns[idx]->name, true);
                uih::list_view_insert_item_text(wnd_lv, idx + 1, 0, m_columns[idx + 1]->name, true);
                ListView_SetItemState(wnd_lv, idx + 1, LVIS_SELECTED, LVIS_SELECTED);
                ListView_EnsureVisible(wnd_lv, idx + 1, FALSE);
            }
            // apply();
        } break;
        case IDC_NEW: {
            HWND wnd_lv = GetDlgItem(wnd, IDC_COLUMNS);
            int idx = ListView_GetNextItem(wnd_lv, -1, LVNI_SELECTED);
            // if (true)
            {
                PlaylistViewColumn::ptr temp = std::make_shared<PlaylistViewColumn>();
                temp->name = "New Column";
                t_size insert = m_columns.insert_item(
                    temp, idx >= 0 && (t_size)idx < m_columns.get_count() ? idx : m_columns.get_count());
                uih::list_view_insert_item_text(wnd_lv, insert, 0, "New Column");
                ListView_SetItemState(wnd_lv, insert, LVIS_SELECTED, LVIS_SELECTED);
                ListView_EnsureVisible(wnd_lv, insert, FALSE);
            }
            // apply();
        } break;
#if 0
                case IDC_REMOVE:
                    {
                        HWND wnd_lv = GetDlgItem(wnd, IDC_COLUMNS);
                        int idx = ListView_GetNextItem(wnd_lv, -1, LVNI_SELECTED);
                        if (idx >= 0) {
                            m_columns.remove_by_idx(idx);
                            ListView_DeleteItem(wnd_lv, idx);

                            if ((unsigned)idx == m_columns.get_count()) idx--;
                            if (idx >= 0)
                                ListView_SetItemState(wnd_lv, idx, LVIS_SELECTED, LVIS_SELECTED);
                        }
                        apply();
                    }
                    break;
                case IDC_COPY:
                    {
                        HWND wnd_lv = GetDlgItem(wnd, IDC_COLUMNS);
                        int idx = ListView_GetNextItem(wnd_lv, -1, LVNI_SELECTED);
                        if (idx >= 0 && m_columns.copy_item(idx)) {
                            int new_idx = m_columns.get_count() - 1;

                            pfc::string8 temp;
                            m_columns.get_string(new_idx, temp, STRING_NAME);

                            if (uih::list_view_insert_item_text(wnd_lv, new_idx, 0, "New Column") == -1)
                                m_columns.delete_item(new_idx);
                            else {
                                ListView_SetItemState(wnd_lv, new_idx, LVIS_SELECTED, LVIS_SELECTED);
                                ListView_EnsureVisible(wnd_lv, new_idx, FALSE);
                            }
                        }
                        apply();
                    }
                    break;
                case IDC_INSERT:
                    {
                        HWND wnd_lv = GetDlgItem(wnd, IDC_COLUMNS);
                        int idx = ListView_GetNextItem(wnd_lv, -1, LVNI_SELECTED);
                        if (idx >= 0 && m_columns.insert_item(idx, "New Column", "", false, "", false, "", 100, ALIGN_LEFT, FILTER_NONE, "", 100, true, "")) {
                            if (uih::list_view_insert_item_text(wnd_lv, idx, 0, "New Column") == -1) {
                                m_columns.delete_item(idx);
                            }
                        }
                        apply();
                    }
                    break;
                case IDC_SAVE_NEW:
                    {
                        HWND wnd_lv = GetDlgItem(wnd, IDC_COLUMNS);
                        column_info * temp = new column_info;
                        temp->set_string(STRING_NAME, "New Column");
                        uih::list_view_insert_item_text(wnd_lv, m_columns.get_count(), 0, "New Column");
                        ListView_SetItemState(wnd_lv, m_columns.get_count(), LVIS_SELECTED, LVIS_SELECTED);
                        ListView_EnsureVisible(wnd_lv, m_columns.get_count(), FALSE);
                        m_columns.add_item(temp);
                        apply();
                    }
                    break;
#endif
        }
    }
    return 0;
}

void TabColumns::apply()
{
    g_columns.set_entries_copy(m_columns);
    for (size_t i = 0, count = m_columns.get_count(); i < count; i++)
        m_columns[i]->source_item = g_columns[i];
    cui::panels::playlist_view::PlaylistView::g_on_columns_change();
}

void TabColumns::show_column(size_t index)
{
    if (m_wnd_lv) {
        const auto& column_to_activate = g_columns[index];
        for (size_t i = 0; i < m_columns.get_count(); i++) {
            if (column_to_activate == m_columns[i]->source_item) {
                ListView_SetItemState(m_wnd_lv, i, LVIS_SELECTED, LVIS_SELECTED);
                ListView_EnsureVisible(m_wnd_lv, i, FALSE);
                break;
            }
        }
    } else {
        cfg_cur_prefs_col = index;
    }
    cui::prefs::page_playlist_view.get_static_instance().show_tab("Columns");
}
