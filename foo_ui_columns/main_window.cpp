/**
 * Columns UI foobar2000 component
 *
 * \author musicmusic
 */

#include "stdafx.h"

#include "main_window.h"

#include "status_pane.h"
#include "layout.h"
#include "dark_mode.h"
#include "notification_area.h"
#include "status_bar.h"
#include "migrate.h"
#include "legacy_artwork_config.h"
#include "rebar.h"

cui::rebar::RebarWindow* g_rebar_window = nullptr;
LayoutWindow g_layout_window;
cui::MainWindow cui::main_window;

HIMAGELIST g_imagelist = nullptr;

HWND g_status = nullptr;

bool g_icon_created = false;
bool ui_initialising = false, g_minimised = false;

HICON g_icon = nullptr;

bool remember_window_pos()
{
    return config_object::g_get_data_bool_simple(standard_config_objects::bool_remember_window_positions, false);
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

    warn_if_ui_hacks_installed();

    try {
        THROW_IF_FAILED(OleInitialize(nullptr));
    } catch (const wil::ResultException&) {
        MessageBox(
            nullptr, unsupported_os_message, L"Columns UI - Failed to initialise COM", MB_OK | MB_ICONEXCLAMATION);
        return nullptr;
    }

    migrate::v100::migrate();

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

    m_wnd = CreateWindowEx(ex_styles, main_window_class_name, _T("foobar2000"), styles, left, top, cx, cy, nullptr,
        nullptr, core_api::get_my_instance(), &main_window);

    main_window::on_transparency_enabled_change();

    const bool rem_pos = remember_window_pos();

    if (rem_pos && !main_window::config_get_is_first_run()) {
        SetWindowPlacement(m_wnd, &cfg_window_placement_columns.get_value());
        resize_child_windows();
        ShowWindow(m_wnd, cfg_window_placement_columns.get_value().showCmd);

        if (g_icon_created && cfg_go_to_tray)
            ShowWindow(m_wnd, SW_HIDE);
    } else {
        resize_child_windows();
        ShowWindow(m_wnd, SW_SHOWNORMAL);
    }

    if (rebar::g_rebar)
        ShowWindow(rebar::g_rebar, SW_SHOWNORMAL);
    if (g_status)
        ShowWindow(g_status, SW_SHOWNORMAL);
    if (status_pane::g_status_pane.get_wnd())
        ShowWindow(status_pane::g_status_pane.get_wnd(), SW_SHOWNORMAL);
    g_layout_window.show_window();

    RedrawWindow(m_wnd, nullptr, nullptr, RDW_UPDATENOW | RDW_ALLCHILDREN);

    if (main_window::config_get_is_first_run())
        SendMessage(m_wnd, MSG_RUN_INITIAL_SETUP, NULL, NULL);

    main_window::config_set_is_first_run();

    fb2k::inMainThread(artwork::legacy::prompt_to_reconfigure);

    return m_wnd;
}

void cui::MainWindow::shutdown()
{
    DestroyWindow(m_wnd);
    UnregisterClass(main_window_class_name, core_api::get_my_instance());
    status_bar::volume_popup_window.class_release();
    m_wnd = nullptr;
    if (g_icon)
        DestroyIcon(g_icon);
    g_icon = nullptr;

    OleUninitialize();
}

void cui::MainWindow::on_query_capability()
{
    if (!m_wnd)
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
        uSetWindowText(m_wnd, ptr);
    m_window_title = ptr;
}

bool cui::MainWindow::update_taskbar_button_images() const
{
    if (!m_taskbar_list)
        return false;

    int cx{};
    int cy{};

    if (!ImageList_GetIconSize(m_taskbar_button_images.get(), &cx, &cy))
        return false;

    const auto icons = colours::is_dark_mode_active() ? dark_taskbar_icons : light_taskbar_icons;

    for (size_t i = 0; i < std::size(light_taskbar_icons); i++) {
        wil::unique_hicon icon(static_cast<HICON>(
            LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(icons[i]), IMAGE_ICON, cx, cy, NULL)));
        ImageList_ReplaceIcon(m_taskbar_button_images.get(), i, icon.get());
    }

    return SUCCEEDED(m_taskbar_list->ThumbBarSetImageList(m_wnd, m_taskbar_button_images.get()));
}

void cui::MainWindow::update_taskbar_buttons(bool update) const
{
    if (m_wnd && m_taskbar_list) {
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
            m_taskbar_list->ThumbBarUpdateButtons(m_wnd, tabsize(tb), tb);
        else
            m_taskbar_list->ThumbBarAddButtons(m_wnd, tabsize(tb), tb);
    }
}

void cui::MainWindow::queue_taskbar_button_update(bool update)
{
    fb2k::inMainThread([update, this] { update_taskbar_buttons(update); });
}

void cui::MainWindow::warn_if_ui_hacks_installed()
{
    constexpr auto ui_hacks_warning
        = "Columns UI detected that the UI Hacks (foo_ui_hacks) component is installed. UI Hacks "
          "interferes with normal Columns UI operation and should be uninstalled to avoid problems.";

    HMODULE ui_hacks_module = nullptr;
    const auto is_ui_hacks_installed
        = GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, L"foo_ui_hacks.dll", &ui_hacks_module);

    if (is_ui_hacks_installed)
        console::print(ui_hacks_warning);
}

void cui::MainWindow::on_create()
{
    INITCOMMONCONTROLSEX icex{};
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES | ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    Gdiplus::GdiplusStartupInput gdiplusStartupInput{};
    m_gdiplus_initialised = (Gdiplus::Ok == GdiplusStartup(&m_gdiplus_instance, &gdiplusStartupInput, nullptr));

    set_dark_mode_attributes();
}

void cui::MainWindow::on_destroy()
{
    if (m_gdiplus_initialised)
        Gdiplus::GdiplusShutdown(m_gdiplus_instance);
    m_gdiplus_initialised = false;
}

void cui::MainWindow::set_dark_mode_attributes(bool is_update) const
{
    if (!dark::does_os_support_dark_mode())
        return;

    const auto is_dark = colours::is_dark_mode_active();
    dark::set_titlebar_mode(m_wnd, is_dark);
    set_app_mode(is_dark ? dark::PreferredAppMode::Dark : dark::PreferredAppMode::Light);

    if (!is_update)
        return;

    update_taskbar_button_images();

    if (!IsWindowVisible(m_wnd))
        return;

    // The below is a hack to force the titlebar to redraw (nothing else works).
    RECT rc{};
    if (!GetWindowRect(m_wnd, &rc))
        return;

    const auto cx = RECT_CX(rc);
    const auto cy = RECT_CY(rc);

    if (cx <= 0)
        return;

    SetWindowPos(m_wnd, nullptr, 0, 0, cx - 1, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(m_wnd, nullptr, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void cui::MainWindow::create_child_windows()
{
    pfc::vartoggle_t<bool> initialising(ui_initialising, true);

    RECT rc;
    GetWindowRect(m_wnd, &rc);

    g_layout_window.create(m_wnd);

    rebar::create_rebar();
    status_bar::create_window();
    if (settings::show_status_pane)
        status_pane::g_status_pane.create(m_wnd);

    g_layout_window.set_focus();
}

void cui::MainWindow::resize_child_windows()
{
    if (!/*g_minimised*/ IsIconic(m_wnd) && !ui_initialising) {
        RECT rc_main_client;
        GetClientRect(m_wnd, &rc_main_client);

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
            if (status_pane::g_status_pane.get_wnd()) {
                int cy = status_pane::g_status_pane.get_ideal_height();
                RedrawWindow(status_pane::g_status_pane.get_wnd(), nullptr, nullptr, RDW_INVALIDATE);
                dwp = DeferWindowPos(dwp, status_pane::g_status_pane.get_wnd(), nullptr, 0,
                    rc_main_client.bottom - status_height - cy, rc_main_client.right - rc_main_client.left, cy,
                    SWP_NOZORDER);
                status_height += cy;
            }
            int rebar_height = 0;

            if (rebar::g_rebar) {
                RECT rc_rebar;
                GetWindowRect(rebar::g_rebar, &rc_rebar);
                rebar_height = rc_rebar.bottom - rc_rebar.top;
            }
            if (g_layout_window.get_wnd())
                dwp = DeferWindowPos(dwp, g_layout_window.get_wnd(), nullptr, 0, rebar_height,
                    rc_main_client.right - rc_main_client.left,
                    rc_main_client.bottom - rc_main_client.top - rebar_height - status_height, SWP_NOZORDER);
            if (rebar::g_rebar) {
                RedrawWindow(rebar::g_rebar, nullptr, nullptr, RDW_INVALIDATE);
                dwp = DeferWindowPos(dwp, rebar::g_rebar, nullptr, 0, 0, rc_main_client.right - rc_main_client.left,
                    rebar_height, SWP_NOZORDER);
            }

            EndDeferWindowPos(dwp);

            if (g_status) {
                set_part_sizes(status_bar::t_parts_none);
            }
        }
    }
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

class MainWindowPlaylistCallback : public playlist_callback_single_static {
public:
    void on_items_added(
        unsigned start, const pfc::list_base_const_t<metadb_handle_ptr>& p_data, const bit_array& p_selection)
        override // inside any of these methods, you can call IPlaylist APIs to get exact info about what happened (but
                 // only methods that read playlist state, not those that modify it)
    {
        if (cui::main_window.get_wnd()) {
            set_part_sizes(cui::status_bar::t_part_length);
        }
    }
    void on_items_reordered(const unsigned* order, unsigned count) override {
    } // changes selection too; doesnt actually change set of items that are selected or
      // item having focus, just changes their order
    void FB2KAPI on_items_removing(const bit_array& p_mask, unsigned p_old_count, unsigned p_new_count) override {
    } // called before actually removing them
    void FB2KAPI on_items_removed(const bit_array& p_mask, unsigned p_old_count, unsigned p_new_count) override
    {
        if (cui::main_window.get_wnd()) {
            set_part_sizes(cui::status_bar::t_part_length);
        }
    }
    void on_items_selection_change(const bit_array& affected, const bit_array& state) override
    {
        if (cui::main_window.get_wnd()) {
            set_part_sizes(cui::status_bar::t_part_length);
        }
    }
    void on_item_focus_change(unsigned from, unsigned to) override {
    } // focus may be -1 when no item has focus; reminder: focus may also change on other callbacks
    void FB2KAPI on_items_modified(const bit_array& p_mask) override {}
    void FB2KAPI on_items_modified_fromplayback(const bit_array& p_mask, play_control::t_display_level p_level) override
    {
    }
    void on_items_replaced(const bit_array& p_mask,
        const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry>& p_data) override
    {
    }
    void on_item_ensure_visible(unsigned idx) override {}

    void on_playlist_switch() override
    {
        if (cui::main_window.get_wnd()) {
            set_part_sizes(cui::status_bar::t_parts_all);
        }
    }
    void on_playlist_renamed(const char* p_new_name, unsigned p_new_name_len) override {}
    void on_playlist_locked(bool p_locked) override
    {
        if (cui::main_window.get_wnd())
            if (g_status && main_window::config_get_status_show_lock())
                set_part_sizes(cui::status_bar::t_parts_all);
    }

    void on_default_format_changed() override {}
    void on_playback_order_changed(unsigned p_new_index) override {}

    unsigned get_flags() override { return flag_all; }
};

static service_factory_single_t<MainWindowPlaylistCallback> asdf2;

void on_show_status_change()
{
    if (cui::main_window.get_wnd()) {
        cui::status_bar::create_window();
        if (g_status) {
            ShowWindow(g_status, SW_SHOWNORMAL);
            UpdateWindow(g_status);
        }
        cui::main_window.resize_child_windows();
    }
}

void on_show_status_pane_change()
{
    if (cui::main_window.get_wnd()) {
        if (settings::show_status_pane != (cui::status_pane::g_status_pane.get_wnd() != nullptr)) {
            if (settings::show_status_pane) {
                cui::status_pane::g_status_pane.create(cui::main_window.get_wnd());
                ShowWindow(cui::status_pane::g_status_pane.get_wnd(), SW_SHOWNORMAL);
            } else
                cui::status_pane::g_status_pane.destroy();
            cui::main_window.resize_child_windows();
        }
    }
}

void on_show_toolbars_change()
{
    if (cui::main_window.get_wnd()) {
        cui::rebar::create_rebar();
        if (cui::rebar::g_rebar) {
            ShowWindow(cui::rebar::g_rebar, SW_SHOWNORMAL);
            UpdateWindow(cui::rebar::g_rebar);
        }
        cui::main_window.resize_child_windows();
    }
}

class UIControl : public columns_ui::control {
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

service_factory_single_t<UIControl> g_control_impl;
