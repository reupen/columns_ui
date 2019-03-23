#ifndef _COLUMNS_UI_H_
#define _COLUMNS_UI_H_

/*!
 * \file main_window.h
 *
 * \author musicmusic
 * \date 1 March 2015
 *
 * Largely functions used for initialising and deinitialising the core parts of the UI
 */

namespace colours {
enum t_colours {
    COLOUR_TEXT,
    COLOUR_SELECTED_TEXT,
    COLOUR_BACK,
    COLOUR_SELECTED_BACK,
    COLOUR_SELECTED_BACK_NO_FOCUS,
    COLOUR_SELECTED_TEXT_NO_FOCUS,
    COLOUR_FRAME,
};
};

COLORREF get_default_colour(colours::t_colours index, bool themed = false);

/** Main window UI control IDs */
#define ID_REBAR 2100
#define ID_STATUS 2002

#define ID_PLAYLIST_TOOLTIP 50

/** Main window custom message numbers */
enum {
    MSG_SET_AOT = WM_USER + 3,
    MSG_UPDATE_STATUS,
    MSG_UPDATE_TITLE,
    MSG_RUN_INITIAL_SETUP,
    MSG_NOTICATION_ICON
};

bool remember_window_pos();

void update_status();

bool process_keydown(UINT msg, LPARAM lp, WPARAM wp, bool playlist = false, bool keyb = true);

void create_rebar();
void destroy_rebar(bool save_config = true);

void status_update_main(bool is_caller_menu_desc);

void create_status();
void on_show_status_change();
void on_show_status_pane_change();
void on_show_toolbars_change();

extern class status_pane g_status_pane;
extern class rebar_window* g_rebar_window;

namespace taskbar_buttons {
enum { ID_FIRST = 667, ID_STOP = ID_FIRST, ID_PREV, ID_PLAY_OR_PAUSE, ID_NEXT, ID_RAND };
}

namespace cui {
class MainWindow {
public:
    HWND initialise(user_interface::HookProc_t hook);
    void shutdown();
    void on_query_capability();
    void update_title();
    void reset_title();

    /*
     * ITaskbarList3::ThumbBarUpdateButtons calls SendMessageTimeout without the SMTO_BLOCK flag.
     * So we postpone updates, to avoid weird bugs (other function calls when executing a callback
     * function that aren't legal, calls to ITaskbarList3::ThumbBarUpdateButtons() during another
     * ITaskbarList3::ThumbBarUpdateButtons() call etc.)
     */
    void queue_taskbar_button_update(bool update = true);
    void resize_child_windows();

    HWND get_wnd() const { return m_wnd; }

private:
    static LRESULT CALLBACK s_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    void on_create();
    void on_destroy();
    void create_child_windows();
    void set_title(const char* ptr);
    void update_taskbar_buttons(bool update);

    pfc::string8 m_window_title;
    mmh::ComPtr<ITaskbarList3> m_taskbar_list;
    HWND m_wnd{};
    user_interface::HookProc_t m_hook_proc{};
    bool m_should_handle_multimedia_keys{true};
    bool m_shell_hook_registered{};
    ULONG_PTR m_gdiplus_instance{NULL};
    bool m_gdiplus_initialised{false};
};

extern MainWindow main_window;
} // namespace cui

#endif
