/**
 * Columns UI foobar2000 component
 *
 * \author musicmusic
 */

#include "stdafx.h"

#include "status_pane.h"
#include "layout.h"

#include "main_window.h"
#include "notification_area.h"
#include "font_notify.h"
#include "status_bar.h"

rebar_window* g_rebar_window = nullptr;
layout_window g_layout_window;
cui::MainWindow cui::main_window;
status_pane g_status_pane;

HIMAGELIST g_imagelist = nullptr;

HWND g_main_window = nullptr, g_tooltip = nullptr, g_rebar = nullptr, g_status = nullptr;

bool ui_initialising = false, g_minimised = false;

bool g_playing = false;

HICON g_icon = nullptr;

pfc::string8 statusbartext;

HFONT g_font = nullptr;

bool remember_window_pos()
{
    return config_object::g_get_data_bool_simple(standard_config_objects::bool_remember_window_positions, false);
}

void action_remove_track(bool on_item, unsigned idx)
{
    if (on_item) {
        static_api_ptr_t<playlist_manager> api;
        api->activeplaylist_undo_backup();
        api->activeplaylist_remove_items(pfc::bit_array_one(idx));
    }
}

void action_add_to_queue(bool on_item, unsigned idx)
{
    if (on_item) {
        static_api_ptr_t<playlist_manager> api;
        unsigned active = api->get_active_playlist();
        if (active != -1)
            api->queue_add_item_playlist(active, idx);
    }
}

void action_none(bool on_on_item, unsigned idx) {}

pma playlist_mclick_actions::g_pma_actions[] = {
    {"(None)", 0, action_none},
    {"Remove track from playlist", 1, action_remove_track},
    {"Add to playback queue", 2, action_add_to_queue},
};

unsigned playlist_mclick_actions::get_count()
{
    return tabsize(g_pma_actions);
}

const TCHAR* main_window_class_name = _T("{E7076D1C-A7BF-4f39-B771-BCBE88F2A2A8}");

const wchar_t* unsupported_os_message
    = L"Sorry, your operating system version is not supported by this version "
      "of Columns UI. Please upgrade to Windows 7 Service Pack 1 or newer and try again.\n\n"
      "Otherwise, uninstall the Columns UI component to return to the Default User Interface.";

HWND cui::MainWindow::initialise(user_interface::HookProc_t hook)
{
    if (!IsWindows7SP1OrGreater()) {
        MessageBox(
            nullptr, unsupported_os_message, L"Columns UI - Unsupported operating system", MB_OK | MB_ICONEXCLAMATION);
        return nullptr;
    }

    if (main_window::config_get_is_first_run()) {
        if (!cfg_layout.get_presets().get_count())
            cfg_layout.reset_presets();
    }

    m_hook_proc = hook;

    create_icon_handle();

    WNDCLASS wc{};
    wc.lpfnWndProc = s_on_message;
    wc.style = CS_DBLCLKS;
    wc.hInstance = core_api::get_my_instance();
    wc.hIcon = ui_control::get()->get_main_icon();
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = main_window_class_name;

    ATOM cls = RegisterClass(&wc);

    RECT rc_work{};
    SystemParametersInfo(SPI_GETWORKAREA, NULL, &rc_work, NULL);

    const int cx = (rc_work.right - rc_work.left) * 80 / 100;
    const int cy = (rc_work.bottom - rc_work.top) * 80 / 100;

    const int left = (rc_work.right - rc_work.left - cx) / 2;
    const int top = (rc_work.bottom - rc_work.top - cy) / 2;

    if (main_window::config_get_is_first_run()) {
        cfg_plist_width = cx * 10 / 100;
    }

    const DWORD ex_styles = main_window::config_get_transparency_enabled() ? WS_EX_LAYERED : 0;
    const DWORD styles = WS_OVERLAPPED | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN
        | WS_CLIPSIBLINGS | WS_THICKFRAME;

    g_main_window = CreateWindowEx(ex_styles, main_window_class_name, _T("foobar2000"), styles, left, top, cx, cy,
        nullptr, nullptr, core_api::get_my_instance(), &cui::main_window);

    main_window::on_transparency_enabled_change();

    const bool rem_pos = remember_window_pos();

    if (rem_pos && !main_window::config_get_is_first_run()) {
        SetWindowPlacement(g_main_window, &cfg_window_placement_columns.get_value());
        resize_child_windows();
        ShowWindow(g_main_window, cfg_window_placement_columns.get_value().showCmd);

        if (g_icon_created && cfg_go_to_tray)
            ShowWindow(g_main_window, SW_HIDE);
    } else {
        resize_child_windows();
        ShowWindow(g_main_window, SW_SHOWNORMAL);
    }

    if (g_rebar)
        ShowWindow(g_rebar, SW_SHOWNORMAL);
    if (g_status)
        ShowWindow(g_status, SW_SHOWNORMAL);
    if (g_status_pane.get_wnd())
        ShowWindow(g_status_pane.get_wnd(), SW_SHOWNORMAL);
    g_layout_window.show_window();

    RedrawWindow(g_main_window, nullptr, nullptr, RDW_UPDATENOW | RDW_ALLCHILDREN);

    if (main_window::config_get_is_first_run())
        SendMessage(g_main_window, MSG_RUN_INITIAL_SETUP, NULL, NULL);

    main_window::config_set_is_first_run();

    return g_main_window;
}

void cui::MainWindow::shutdown()
{
    DestroyWindow(g_main_window);
    UnregisterClass(main_window_class_name, core_api::get_my_instance());
    status_bar::volume_popup_window.class_release();
    g_main_window = nullptr;
    g_status = nullptr;
    if (g_imagelist) {
        ImageList_Destroy(g_imagelist);
        g_imagelist = nullptr;
    }
    if (g_icon)
        DestroyIcon(g_icon);
    g_icon = nullptr;
    status_bar::destroy_icon();

    font_cleanup();
}

void cui::MainWindow::on_query_capability()
{
    if (!g_main_window)
        m_should_handle_multimedia_keys = false;
}

void cui::MainWindow::update_title()
{
    metadb_handle_ptr track;
    static_api_ptr_t<play_control> play_api;
    play_api->get_now_playing(track);
    if (track.is_valid()) {
        pfc::string8 title;
        service_ptr_t<titleformat_object> to_wtitle;
        static_api_ptr_t<titleformat_compiler>()->compile_safe(
            to_wtitle, main_window::config_main_window_title_script.get());
        play_api->playback_format_title_ex(track, nullptr, title, to_wtitle, nullptr, play_control::display_level_all);
        set_title(title);
        track.release();
    } else {
        set_title(core_version_info_v2::get()->get_name());
    }
}

void cui::MainWindow::reset_title()
{
    set_title(core_version_info_v2::get()->get_name());
}

void cui::MainWindow::set_title(const char* ptr)
{
    if (strcmp(m_window_title, ptr) != 0)
        uSetWindowText(g_main_window, ptr);
    m_window_title = ptr;
}

void cui::MainWindow::update_taskbar_buttons(bool update)
{
    if (g_main_window && m_taskbar_list.is_valid()) {
        static_api_ptr_t<playback_control> play_api;

        bool b_is_playing = play_api->is_playing();
        bool b_is_paused = play_api->is_paused();

        const WCHAR* ttips[6]
            = {L"Stop", L"Previous", (b_is_playing && !b_is_paused ? L"Pause" : L"Play"), L"Next", L"Random"};
        INT_PTR bitmap_indices[] = {0, 1, (b_is_playing && !b_is_paused ? 2 : 3), 4, 5};

        THUMBBUTTON tb[tabsize(bitmap_indices)];
        memset(&tb, 0, sizeof(tb));

        for (size_t i = 0; i < tabsize(bitmap_indices); i++) {
            tb[i].dwMask = THB_BITMAP | THB_TOOLTIP /*|THB_FLAGS*/;
            tb[i].iId = taskbar_buttons::ID_FIRST + i;
            tb[i].iBitmap = bitmap_indices[i];
            wcscpy_s(tb[i].szTip, tabsize(tb[i].szTip), ttips[i]);
            // if (tb[i].iId == ID_STOP && !b_is_playing)
            //    tb[i].dwFlags |= THBF_DISABLED;
        }

        if (update)
            m_taskbar_list->ThumbBarUpdateButtons(g_main_window, tabsize(tb), tb);
        else
            m_taskbar_list->ThumbBarAddButtons(g_main_window, tabsize(tb), tb);
    }
}

void cui::MainWindow::queue_taskbar_button_update(bool update)
{
    fb2k::inMainThread([update, this] { update_taskbar_buttons(update); });
}

void cui::MainWindow::on_create()
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    m_gdiplus_initialised
        = (Gdiplus::Ok == Gdiplus::GdiplusStartup(&m_gdiplus_instance, &gdiplusStartupInput, nullptr));
}

void cui::MainWindow::on_destroy()
{
    if (m_gdiplus_initialised)
        Gdiplus::GdiplusShutdown(m_gdiplus_instance);
    m_gdiplus_initialised = false;
}

void cui::MainWindow::create_child_windows()
{
    pfc::vartoggle_t<bool> initialising(ui_initialising, true);

    RECT rc;
    GetWindowRect(g_main_window, &rc);

    g_layout_window.create(g_main_window);

    create_rebar();
    create_status();
    if (settings::show_status_pane)
        g_status_pane.create(g_main_window);

    g_layout_window.set_focus();
}

void cui::MainWindow::resize_child_windows()
{
    if (!/*g_minimised*/ IsIconic(g_main_window) && !ui_initialising) {
        RECT rc_main_client;
        GetClientRect(g_main_window, &rc_main_client);

        HDWP dwp = BeginDeferWindowPos(7);
        if (dwp) {
            int status_height = 0;
            if (g_status) {
                // SendMessage(g_status, WM_SETREDRAW, FALSE, 0);
                SendMessage(g_status, WM_SIZE, 0, 0);
                RECT rc_status;
                GetWindowRect(g_status, &rc_status);

                status_height += rc_status.bottom - rc_status.top;

                // dwp = DeferWindowPos(dwp, g_status, 0, 0, rc_main_client.bottom-status_height,
                // rc_main_client.right-rc_main_client.left, status_height, SWP_NOZORDER|SWP_NOREDRAW);
            }
            if (g_status_pane.get_wnd()) {
                int cy = g_status_pane.get_ideal_height();
                RedrawWindow(g_status_pane.get_wnd(), nullptr, nullptr, RDW_INVALIDATE);
                dwp = DeferWindowPos(dwp, g_status_pane.get_wnd(), nullptr, 0,
                    rc_main_client.bottom - status_height - cy, rc_main_client.right - rc_main_client.left, cy,
                    SWP_NOZORDER);
                status_height += cy;
            }
            int rebar_height = 0;

            if (g_rebar) {
                RECT rc_rebar;
                GetWindowRect(g_rebar, &rc_rebar);
                rebar_height = rc_rebar.bottom - rc_rebar.top;
            }
            if (g_layout_window.get_wnd())
                dwp = DeferWindowPos(dwp, g_layout_window.get_wnd(), nullptr, 0, rebar_height,
                    rc_main_client.right - rc_main_client.left,
                    rc_main_client.bottom - rc_main_client.top - rebar_height - status_height, SWP_NOZORDER);
            if (g_rebar) {
                RedrawWindow(g_rebar, nullptr, nullptr, RDW_INVALIDATE);
                dwp = DeferWindowPos(dwp, g_rebar, nullptr, 0, 0, rc_main_client.right - rc_main_client.left,
                    rebar_height, SWP_NOZORDER);
            }

            EndDeferWindowPos(dwp);

            if (g_status) {
                status_bar::set_part_sizes(status_bar::t_parts_none);
            }
        }
    }
}

unsigned playlist_mclick_actions::id_to_idx(unsigned id)
{
    unsigned count = tabsize(g_pma_actions);
    for (unsigned n = 0; n < count; n++) {
        if (g_pma_actions[n].id == id)
            return n;
    }
    return 0;
}

bool process_keydown(UINT msg, LPARAM lp, WPARAM wp, bool playlist, bool keyb)
{
    static_api_ptr_t<keyboard_shortcut_manager_v2> keyboard_api;

    if (msg == WM_SYSKEYDOWN) {
        if (keyb && uie::window::g_process_keydown_keyboard_shortcuts(wp)) {
            return true;
        }
    } else if (msg == WM_KEYDOWN) {
        if (keyb && uie::window::g_process_keydown_keyboard_shortcuts(wp)) {
            return true;
        }
        if (wp == VK_TAB) {
            uie::window::g_on_tab(GetFocus());
        }
    }
    return false;
}

bool g_icon_created = false;

class playlist_callback_single_columns : public playlist_callback_single_static {
public:
    void on_items_added(
        unsigned start, const pfc::list_base_const_t<metadb_handle_ptr>& p_data, const pfc::bit_array& p_selection)
        override // inside any of these methods, you can call IPlaylist APIs to get exact info about what happened (but
                 // only methods that read playlist state, not those that modify it)
    {
        if (g_main_window) {
            status_bar::set_part_sizes(status_bar::t_part_length);
        }
    }
    void on_items_reordered(const unsigned* order,
        unsigned count) override{}; // changes selection too; doesnt actually change set of items that are selected or
                                    // item having focus, just changes their order
    void FB2KAPI on_items_removing(const pfc::bit_array& p_mask, unsigned p_old_count,
        unsigned p_new_count) override{}; // called before actually removing them
    void FB2KAPI on_items_removed(const pfc::bit_array& p_mask, unsigned p_old_count, unsigned p_new_count) override
    {
        if (g_main_window) {
            status_bar::set_part_sizes(status_bar::t_part_length);
        }
    };
    void on_items_selection_change(const pfc::bit_array& affected, const pfc::bit_array& state) override
    {
        if (g_main_window) {
            status_bar::set_part_sizes(status_bar::t_part_length);
        }
    }
    void on_item_focus_change(unsigned from, unsigned to)
        override{}; // focus may be -1 when no item has focus; reminder: focus may also change on other callbacks
    void FB2KAPI on_items_modified(const pfc::bit_array& p_mask) override {}
    void FB2KAPI on_items_modified_fromplayback(
        const pfc::bit_array& p_mask, play_control::t_display_level p_level) override{};
    void on_items_replaced(const pfc::bit_array& p_mask,
        const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry>& p_data) override{};
    void on_item_ensure_visible(unsigned idx) override{};

    void on_playlist_switch() override
    {
        if (g_main_window) {
            status_bar::set_part_sizes(status_bar::t_parts_all);
        }
    };
    void on_playlist_renamed(const char* p_new_name, unsigned p_new_name_len) override{};
    void on_playlist_locked(bool p_locked) override
    {
        if (g_main_window)
            if (g_status && main_window::config_get_status_show_lock())
                status_bar::set_part_sizes(status_bar::t_parts_all);
    };

    void on_default_format_changed() override{};
    void on_playback_order_changed(unsigned p_new_index) override{};

    unsigned get_flags() override { return playlist_callback_single::flag_all; }
};
static service_factory_single_t<playlist_callback_single_columns> asdf2;

void g_split_string_by_crlf(const char* text, pfc::string_list_impl& p_out)
{
    const char* ptr = text;
    while (*ptr) {
        const char* start = ptr;
        t_size counter = 0;
        while (*ptr && *ptr != '\r' && *ptr != '\n') {
            ptr++;
        }

        p_out.add_item(pfc::string8(start, ptr - start));

        if (*ptr == '\r')
            ptr++;
        if (*ptr == '\n')
            ptr++;
    }
}

class rename_param {
public:
    modal_dialog_scope m_scope;
    pfc::string8* m_text{};
};

static BOOL CALLBACK RenameProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG:
        SetWindowLongPtr(wnd, DWLP_USER, lp);
        {
            auto* ptr = (rename_param*)lp;
            ptr->m_scope.initialize(FindOwningPopup(wnd));
            pfc::string_formatter formatter;
            formatter << R"(Rename playlist: ")" << *ptr->m_text << R"(")";
            uSetWindowText(wnd, formatter);
            uSetDlgItemText(wnd, IDC_EDIT, ptr->m_text->get_ptr());
        }
        return 1;
    case WM_COMMAND:
        switch (wp) {
        case IDOK: {
            auto* ptr = (rename_param*)GetWindowLong(wnd, DWLP_USER);
            uGetDlgItemText(wnd, IDC_EDIT, *ptr->m_text);
            EndDialog(wnd, 1);
        } break;
        case IDCANCEL:
            EndDialog(wnd, 0);
            break;
        }
        break;
    case WM_CLOSE:
        EndDialog(wnd, 0);
        break;
    }
    return 0;
}

bool g_rename_dialog(pfc::string8* text, HWND parent)
{
    rename_param param;
    param.m_text = text;
    return !!uDialogBox(IDD_RENAME_PLAYLIST, parent, RenameProc, (LPARAM)(&param));
}

void g_rename_playlist(unsigned idx, HWND wnd_parent)
{
    static_api_ptr_t<playlist_manager> playlist_api;
    pfc::string8 temp;
    if (playlist_api->playlist_get_name(idx, temp)) {
        if (g_rename_dialog(&temp, wnd_parent)) { // fucko: dialogobx has a messgeloop, someone might have called
                                                  // switcher api funcs in the meanwhile
            //            idx = ((HWND)wp == g_tab) ? idx : SendMessage(g_plist,LB_GETCURSEL,0,0);
            unsigned num = playlist_api->get_playlist_count();
            if (idx < num) {
                playlist_api->playlist_rename(idx, temp, temp.length());
            }
        }
    }
}

bool g_get_resource_data(INT_PTR id, pfc::array_t<t_uint8>& p_out)
{
    bool ret = false;
    HRSRC rsrc = FindResource(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_NOCOVER), L"PNG");
    HGLOBAL handle = LoadResource(core_api::get_my_instance(), rsrc);
    DWORD size = SizeofResource(core_api::get_my_instance(), rsrc);
    LPVOID ptr = LockResource(handle);
    if (ptr && size) {
        p_out.append_fromptr((t_uint8*)ptr, size);
        ret = true;
    }
    FreeResource(handle);
    return ret;
}

class control_impl : public columns_ui::control {
public:
    bool get_string(const GUID& p_guid, pfc::string_base& p_out) const override
    {
        if (p_guid == columns_ui::strings::guid_global_variables) {
            p_out = cfg_globalstring;
            return true;
        }
        return false;
    }
};

service_factory_single_t<control_impl> g_control_impl;
