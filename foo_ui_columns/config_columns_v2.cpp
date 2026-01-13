#include "pch.h"
#include "ng_playlist/ng_playlist.h"
#include "config.h"
#include "config_columns_v2.h"

#include "format_code_generator.h"
#include "help.h"

namespace cui::prefs {

namespace {

enum class ColumnTabIndex : int32_t {
    General,
    DisplayScript,
    StyleScript,
    SortingScript,
};

} // namespace

cfg_int g_cur_tab(GUID{0x1f7903e5, 0x9523, 0xac7e, {0xd4, 0xea, 0x13, 0xdd, 0xe5, 0xac, 0xc8, 0x66}}, 0);
cfg_uint g_last_colour(GUID{0xd352a60a, 0x4d87, 0x07b9, {0x09, 0x07, 0x03, 0xa1, 0xe0, 0x08, 0x03, 0x2f}}, 0);
cfg_int cfg_child_column({0xa7a2845, 0x6a4, 0x4c15, {0xb0, 0x9f, 0xa6, 0xeb, 0xee, 0x86, 0x33, 0x5d}},
    WI_EnumValue(ColumnTabIndex::General));

constexpr auto MSG_COLUMN_NAME_CHANGED = WM_USER + 3;

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
    void get_column(PlaylistViewColumn::ptr& p_out) override { p_out = m_column; }
    using self_t = EditColumnWindowOptions;
    HWND create(HWND parent_window) override
    {
        auto on_message_ = [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); };

        auto [wnd, _] = fbh::auto_dark_modeless_dialog_box(IDD_COLUMN_OPTIONS, parent_window, std::move(on_message_));
        return wnd;
    }
    // virtual const char * get_name()=0;
    explicit EditColumnWindowOptions(PlaylistViewColumn::ptr column)
        : initialising(false)
        , editproc(nullptr)
        , m_wnd(nullptr)
        , m_column(std::move(column))
    {
    }

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

        EnableWindow(GetDlgItem(wnd, IDC_NAME), show);
        EnableWindow(GetDlgItem(wnd, IDC_WIDTH), show);
        EnableWindow(GetDlgItem(wnd, IDC_PARTS), show);
        EnableWindow(GetDlgItem(wnd, IDC_SHOW_COLUMN), show);
        EnableWindow(GetDlgItem(wnd, IDC_ALIGNMENT), show);
        const auto enable_filter_pattern_controls = show && m_column && m_column->filter_type != FILTER_NONE;
        EnableWindow(GetDlgItem(wnd, IDC_PLAYLIST_FILTER_STRING), enable_filter_pattern_controls);
        ShowWindow(GetDlgItem(wnd, IDC_PLAYLIST_FILTER_HINT), enable_filter_pattern_controls ? SW_SHOW : SW_HIDE);
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
            SendDlgItemMessage(wnd, IDC_ALIGNMENT, CB_SETCURSEL, (size_t)m_column->align, 0);
            SendDlgItemMessage(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_SETCURSEL, (size_t)m_column->filter_type, 0);

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

    void on_column_name_change(const PlaylistViewColumn::ptr& column) override
    {
        if (m_column != column)
            return;

        initialising = true;
        uSendDlgItemMessageText(m_wnd, IDC_NAME, WM_SETTEXT, 0, m_column->name);
        initialising = false;
    }

    INT_PTR CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            m_wnd = wnd;

            uih::enhance_edit_control(wnd, IDC_NAME);
            uih::enhance_edit_control(wnd, IDC_WIDTH);
            uih::enhance_edit_control(wnd, IDC_PARTS);
            uih::enhance_edit_control(wnd, IDC_PLAYLIST_FILTER_STRING);
            uih::enhance_edit_control(wnd, IDC_EDITFIELD);

            uSendDlgItemMessageText(wnd, IDC_ALIGNMENT, CB_ADDSTRING, 0, "Left");
            uSendDlgItemMessageText(wnd, IDC_ALIGNMENT, CB_ADDSTRING, 0, "Centre");
            uSendDlgItemMessageText(wnd, IDC_ALIGNMENT, CB_ADDSTRING, 0, "Right");

            uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_ADDSTRING, 0, "Show on all playlists");
            uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_ADDSTRING, 0, "Show only on playlists:");
            uSendDlgItemMessageText(wnd, IDC_PLAYLIST_FILTER_TYPE, CB_ADDSTRING, 0, "Hide on playlists:");

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
                    const auto enable_filter_pattern_controls = m_column->filter_type != FILTER_NONE;
                    EnableWindow(GetDlgItem(wnd, IDC_PLAYLIST_FILTER_STRING), enable_filter_pattern_controls);
                    ShowWindow(
                        GetDlgItem(wnd, IDC_PLAYLIST_FILTER_HINT), enable_filter_pattern_controls ? SW_SHOW : SW_HIDE);
                }
            } break;
            case IDC_SHOW_COLUMN: {
                if (!initialising && m_column) {
                    m_column->show = ((SendMessage((HWND)lp, BM_GETCHECK, 0, 0) != 0));
                }
            } break;
            case (EN_CHANGE << 16) | IDC_SORT: {
                if (!initialising && m_column) {
                    m_column->sort_spec = (uGetWindowText((HWND)lp));
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
                    m_column->filter = (uGetWindowText((HWND)lp));
                }
            } break;
            case (EN_CHANGE << 16) | IDC_EDITFIELD: {
                if (!initialising && m_column) {
                    m_column->edit_field = (uGetWindowText((HWND)lp));
                }
            } break;
            case (EN_CHANGE << 16) | IDC_NAME: {
                if (!initialising && m_column) {
                    m_column->name = uGetWindowText((HWND)lp);
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
    RECT rc{};
    GetWindowRect(GetDlgItem(wnd, IDC_TFHELP), &rc);

    uih::Menu menu;
    uih::MenuCommandCollector collector;

    menu.append_command(collector.add([] { standard_commands::main_titleformat_help(); }), L"Title formatting help");
    menu.append_command(collector.add([wnd] { help::open_style_script_help(GetParent(wnd)); }), L"Style script help");
    menu.append_command(
        collector.add([=] { help::open_global_variables_help(GetParent(wnd)); }), L"Global variables help");

    if (cfg_child_column == WI_EnumValue(ColumnTabIndex::DisplayScript)) {
        menu.append_separator();
        menu.append_command(
            collector.add([wnd] { help::open_text_styling_help(GetParent(wnd)); }), L"Text styling help");
        menu.append_command(collector.add([parent_wnd{GetAncestor(wnd, GA_ROOT)}] {
            utils::open_format_code_generator(parent_wnd, panels::playlist_view::items_font_id);
        }),
            L"Format code generator");
    }

    menu.append_separator();
    menu.append_command(collector.add([] { speedtest(g_columns, cfg_global != 0); }), L"Speed test");
    menu.append_command(
        collector.add([edit_ctrl_id, wnd] { preview_to_console(uGetDlgItemText(wnd, edit_ctrl_id), cfg_global != 0); }),
        L"Preview script");
    menu.append_separator();
    menu.append_command(collector.add([wnd] {
        if (auto font_description = fonts::select_font(GetParent(wnd), cfg_editor_font->log_font)) {
            cfg_editor_font = *font_description;
            g_editor_font_notify.on_change();
        };
    }),
        L"Change editor font");

    menu_helpers::win32_auto_mnemonics(menu.get());

    collector.execute(menu.run(wnd, {rc.left, rc.bottom}));
}

class DisplayScriptTab : public ColumnTab {
public:
    using SelfType = DisplayScriptTab;

    void get_column(PlaylistViewColumn::ptr& p_out) override { p_out = m_column; }

    HWND create(HWND parent_window) override
    {
        auto on_message_ = [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); };

        auto [wnd, _]
            = fbh::auto_dark_modeless_dialog_box(IDD_COLUMN_DISPLAY_SCRIPT, parent_window, std::move(on_message_));
        return wnd;
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

    INT_PTR CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG:
            m_wnd = wnd;
            Edit_LimitText(edit_control(), 0);
            uih::enhance_edit_control(edit_control());
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
                    m_column->spec = uGetWindowText(reinterpret_cast<HWND>(lp));
                break;
            }
        }
        return 0;
    }

    HWND edit_control() const { return GetDlgItem(m_wnd, IDC_DISPLAY_SCRIPT); }

    HWND m_wnd;
    bool initialising;
    PlaylistViewColumn::ptr m_column;
};

class StyleScriptTab : public ColumnTab {
public:
    using SelfType = StyleScriptTab;

    void get_column(PlaylistViewColumn::ptr& p_out) override { p_out = m_column; }

    HWND create(HWND parent_window) override
    {
        auto on_message_ = [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); };

        auto [wnd, _]
            = fbh::auto_dark_modeless_dialog_box(IDD_COLUMN_STYLE_SCRIPT, parent_window, std::move(on_message_));
        return wnd;
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

    INT_PTR CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG:
            m_wnd = wnd;
            colour_code_gen(wnd, IDC_COLOUR, true, true);
            Edit_LimitText(edit_control(), 0);
            uih::enhance_edit_control(edit_control());
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
                    m_column->colour_spec = uGetWindowText(reinterpret_cast<HWND>(lp));
                break;
            }
        }
        return 0;
    }

    HWND edit_control() const { return GetDlgItem(m_wnd, IDC_STYLE_SCRIPT); }
    HWND custom_colour_control() const { return GetDlgItem(m_wnd, IDC_CUSTOM_COLOUR); }

    HWND m_wnd;
    bool initialising;
    PlaylistViewColumn::ptr m_column;
};

class SortingScriptTab : public ColumnTab {
public:
    using SelfType = SortingScriptTab;

    void get_column(PlaylistViewColumn::ptr& p_out) override { p_out = m_column; }

    HWND create(HWND parent_window) override
    {
        auto on_message_ = [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); };

        auto [wnd, _]
            = fbh::auto_dark_modeless_dialog_box(IDD_COLUMN_SORTING_SCRIPT, parent_window, std::move(on_message_));
        return wnd;
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

    INT_PTR CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG:
            m_wnd = wnd;
            Edit_LimitText(edit_control(), 0);
            uih::enhance_edit_control(edit_control());
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
                    m_column->sort_spec = uGetWindowText(reinterpret_cast<HWND>(lp));
                break;
            }
        }
        return 0;
    }

    HWND edit_control() const { return GetDlgItem(m_wnd, IDC_SORTING_SCRIPT); }
    HWND custom_sorting_control() const { return GetDlgItem(m_wnd, IDC_CUSTOM_SORT); }

    HWND m_wnd;
    bool initialising;
    PlaylistViewColumn::ptr m_column;
};

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

    const auto count = 4;

    if (cfg_child_column >= count)
        cfg_child_column = WI_EnumValue(ColumnTabIndex::General);

    if (cfg_child_column < count && cfg_child_column >= 0) {
        const auto item = m_columns_list_view.get_selected_item_single();

        PlaylistViewColumn::ptr column;
        if (item != std::numeric_limits<size_t>::max())
            column = m_columns[item];

        if (cfg_child_column == WI_EnumValue(ColumnTabIndex::General))
            m_child = std::make_unique<EditColumnWindowOptions>(column);
        else if (cfg_child_column == WI_EnumValue(ColumnTabIndex::DisplayScript))
            m_child = std::make_unique<DisplayScriptTab>(column);
        else if (cfg_child_column == WI_EnumValue(ColumnTabIndex::StyleScript))
            m_child = std::make_unique<StyleScriptTab>(column);
        else if (cfg_child_column == WI_EnumValue(ColumnTabIndex::SortingScript))
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

    const auto idx = (init ? cfg_cur_prefs_col : gsl::narrow_cast<int>(m_columns_list_view.get_selected_item_single()));
    m_columns_list_view.remove_all_items();

    std::vector<uih::ListView::InsertItem> insert_items;
    insert_items.reserve(m_columns.get_count());

    std::ranges::transform(m_columns, std::back_inserter(insert_items),
        [](const auto& column) { return uih::ListView::InsertItem{{column->name.c_str()}, {}}; });

    m_columns_list_view.insert_items(0, insert_items.size(), insert_items.data());

    initialising = false;

    if (idx >= 0 && idx < gsl::narrow<int>(m_columns.get_count())) {
        m_columns_list_view.set_item_selected_single(gsl::narrow<size_t>(idx), false);
        m_columns_list_view.ensure_visible(gsl::narrow<size_t>(idx));
    }
}

INT_PTR TabColumns::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        m_wnd = wnd;

        LOGFONT font{};
        GetObject(GetWindowFont(wnd), sizeof(font), &font);
        m_columns_list_view.set_font_from_log_font(font);
        m_columns_list_view.create(wnd, {7, 30, 79, 215}, true);

        m_columns.set_entries_copy(g_columns, true);

        refresh_me(wnd, true);

        HWND wnd_tab = GetDlgItem(wnd, IDC_TAB1);
        uTabCtrl_InsertItemText(wnd_tab, 0, "Options");
        uTabCtrl_InsertItemText(wnd_tab, 1, "Display script");
        uTabCtrl_InsertItemText(wnd_tab, 2, "Style script");
        uTabCtrl_InsertItemText(wnd_tab, 3, "Sorting script");

        TabCtrl_SetCurSel(wnd_tab, cfg_child_column);

        ShowWindow(m_columns_list_view.get_wnd(), SW_SHOWNORMAL);
        make_child();
    }

    break;
    case WM_DESTROY: {
        const auto index = m_columns_list_view.get_selected_item_single();
        if (index != std::numeric_limits<size_t>::max()) {
            cfg_cur_prefs_col = gsl::narrow<int>(index);
        }

        apply();
        m_columns.remove_all();
        m_columns_list_view.destroy();
        m_wnd = nullptr;

        if (m_wnd_child) {
            DestroyWindow(m_wnd_child);
            m_wnd_child = nullptr;
            m_child.reset();
        }
    } break;
    case MSG_COLUMN_NAME_CHANGED: {
        const auto index = m_columns_list_view.get_selected_item_single();

        if (index != std::numeric_limits<size_t>::max()) {
            const std::vector<uih::ListView::InsertItem> items{
                uih::ListView::InsertItem{{m_columns[index]->name.c_str()}, {}}};
            m_columns_list_view.replace_items(index, items.size(), items.data());
        }
        return 0;
    }
    case WM_NOTIFY: {
        auto lpnm = (LPNMHDR)lp;
        switch (lpnm->idFrom) {
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
        case IDC_UP:
            move_column_up(m_columns_list_view.get_selected_item_single());
            break;
        case IDC_DOWN:
            move_column_down(m_columns_list_view.get_selected_item_single());
            break;
        case IDC_NEW:
            add_column(m_columns_list_view.get_selected_item_single());
            break;
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
    if (m_columns_list_view.get_wnd()) {
        const auto& column_to_activate = g_columns[index];
        for (size_t i = 0; i < m_columns.get_count(); i++) {
            if (column_to_activate == m_columns[i]->source_item) {
                m_columns_list_view.set_item_selected_single(i);
                m_columns_list_view.ensure_visible(i);
                break;
            }
        }
    } else {
        cfg_cur_prefs_col = gsl::narrow<int>(index);
    }
    cui::prefs::page_playlist_view.get_static_instance().show_tab("Columns");
}

bool TabColumns::ColumnsListView::notify_create_inline_edit(const pfc::list_base_const_t<size_t>& indices,
    size_t column, pfc::string_base& p_text, size_t& p_flags, wil::com_ptr<IUnknown>& autocomplete_entries)
{
    size_t indices_count = indices.get_count();

    if (indices_count != 1 || indices[0] >= m_tab.m_columns.size())
        return false;

    const auto edit_index = indices[0];
    m_inline_edit_column = m_tab.m_columns[edit_index];
    p_text = m_inline_edit_column->name;
    return true;
}

void TabColumns::ColumnsListView::notify_save_inline_edit(const char* value)
{
    auto _ = wil::scope_exit([this] { m_inline_edit_column.reset(); });

    if (!m_inline_edit_column)
        return;

    m_inline_edit_column->name = value;

    const auto index = fbh::as_optional(m_tab.m_columns.find_item(m_inline_edit_column));

    if (!index)
        return;

    const std::vector items{InsertItem{{value}, {}}};
    replace_items(*index, items.size(), items.data());

    if (m_tab.m_child)
        m_tab.m_child->on_column_name_change(m_inline_edit_column);
}

bool TabColumns::on_column_list_contextmenu(const POINT& pt, bool from_keyboard)
{
    const auto item = m_columns_list_view.get_selected_item_single();
    const auto is_item_selected = item != std::numeric_limits<size_t>::max();

    uih::Menu menu;
    uih::MenuCommandCollector collector;

    menu.append_command(collector.add([this, item] { add_column(item); }), L"New");

    if (is_item_selected) {
        menu.append_command(collector.add([this, item] { remove_column(item); }), L"Remove");

        if (m_columns.size() > 1)
            menu.append_separator();

        if (item > 0)
            menu.append_command(collector.add([this, item] { move_column_up(item); }), L"Move up");

        if (item + 1 < m_columns.get_count())
            menu.append_command(collector.add([this, item] { move_column_down(item); }), L"Move down");
    }

    menu_helpers::win32_auto_mnemonics(menu.get());

    collector.execute(menu.run(m_columns_list_view.get_wnd(), pt));

    return true;
}

void TabColumns::on_column_list_selection_change()
{
    if (!m_child)
        return;

    const auto index = m_columns_list_view.get_selected_item_single();
    m_child->set_column(index != std::numeric_limits<size_t>::max() ? m_columns[index] : PlaylistViewColumn::ptr());
}

void TabColumns::on_column_list_reorder(size_t old_index, size_t new_index)
{
    auto permutation = utils::create_shift_item_permutation(old_index, new_index, m_columns_list_view.get_item_count());
    m_columns.reorder(permutation.data());

    const auto first_affected = std::min(old_index, new_index);
    const auto last_affected = std::max(old_index, new_index);
    const auto affected_count = last_affected - first_affected + 1;

    std::vector<uih::ListView::InsertItem> insert_items(affected_count);

    for (const auto index : ranges::views::iota(size_t{}, affected_count)) {
        insert_items[index].m_subitems.emplace_back(m_columns[first_affected + index]->name);
    }

    m_columns_list_view.replace_items(first_affected, insert_items.size(), insert_items.data());
}

void TabColumns::add_column(size_t index)
{
    const PlaylistViewColumn::ptr column = std::make_shared<PlaylistViewColumn>();
    column->name = "New column";

    const size_t insert_index
        = m_columns.insert_item(column, index < m_columns.get_count() ? index : m_columns.get_count());

    const std::vector<uih::ListView::InsertItem> insert_items{{{column->name}, {}}};
    m_columns_list_view.insert_items(insert_index, insert_items.size(), insert_items.data());
    m_columns_list_view.set_item_selected_single(insert_index);
    m_columns_list_view.ensure_visible(insert_index);
}

void TabColumns::remove_column(size_t index)
{
    if (index >= m_columns.get_count())
        return;

    m_columns.remove_by_idx(index);
    m_columns_list_view.remove_item(index);

    if (index > 0 && index == m_columns.size()) {
        m_columns_list_view.set_item_selected_single(index - 1);
    } else if (m_columns.size() > 0) {
        m_columns_list_view.set_item_selected_single(index);
    } else {
        on_column_list_selection_change();
    }
}

void TabColumns::move_column_up(size_t index)
{
    if (!m_columns.move_up(index))
        return;

    const auto selection_index = m_columns_list_view.get_selected_item_single();
    const std::vector<uih::ListView::InsertItem> insert_items{
        {{m_columns[index - 1]->name}, {}}, {{m_columns[index]->name}, {}}};

    m_columns_list_view.replace_items(index - 1, insert_items.size(), insert_items.data());

    if (selection_index == index - 1 || selection_index == index) {
        const auto new_selection_index = selection_index == index ? index - 1 : index;
        m_columns_list_view.set_item_selected_single(new_selection_index);
        m_columns_list_view.ensure_visible(new_selection_index);
    }
}

void TabColumns::move_column_down(size_t index)
{
    if (!m_columns.move_down(index))
        return;

    const auto selection_index = m_columns_list_view.get_selected_item_single();
    std::vector<uih::ListView::InsertItem> insert_items{
        {{m_columns[index]->name}, {}}, {{m_columns[index + 1]->name}, {}}};

    m_columns_list_view.replace_items(index, insert_items.size(), insert_items.data());

    if (selection_index == index || selection_index == index + 1) {
        const auto new_selection_index = selection_index == index ? index + 1 : index;
        m_columns_list_view.set_item_selected_single(new_selection_index);
        m_columns_list_view.ensure_visible(new_selection_index);
    }
}

} // namespace cui::prefs
