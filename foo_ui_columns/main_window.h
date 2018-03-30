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

#define SWITCH_TIMER_ID 670
#define DAY_TIMER_ID 671

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

BOOL uDrawPanelTitle(HDC dc, const RECT* rc_clip, const char* text, int len, bool vert, bool world);

/** Main window UI control IDs */
#define ID_REBAR 2100
#define ID_STATUS 2002

#define ID_PLAYLIST_TOOLTIP 50

/** Main window custom message numbers */
enum {
    MSG_SET_AOT = WM_USER + 3,
    MSG_UPDATE_STATUS,
    MSG_UPDATE_TITLE,
    MSG_KILL_TIMER,
    MSG_SWITCH_PLAYLIST,
    MSG_SIZE,
    MSG_HIDE_SIDEBAR,
    MSG_SET_FOCUS,
    MSG_RUN_INITIAL_SETUP,
    MSG_NOTICATION_ICON
};

namespace status_bar {
enum t_parts : uint32_t {
    t_parts_none = 0,
    t_parts_all = 0xffffffff,
    t_part_main = 1 << 0,
    t_part_lock = 1 << 1,
    t_part_length = 1 << 2,
    t_part_volume = 1 << 3
};

void set_part_sizes(unsigned p_parts = t_parts_none);
}; // namespace status_bar

bool remember_window_pos();

void update_status();

bool process_keydown(UINT msg, LPARAM lp, WPARAM wp, bool playlist = false, bool keyb = true);

void create_rebar();
void destroy_rebar(bool save_config = true);

void status_update_main(bool is_caller_menu_desc);

void create_status();

bool g_rename_dialog(pfc::string8* param, HWND parent);
void g_rename_playlist(unsigned idx, HWND wnd_parent);

void make_ui();
void size_windows();

extern class status_pane g_status_pane;
extern class rebar_window* g_rebar_window;

using pma_action = void (*)(bool, unsigned int);

struct pma {
    const char* name;
    unsigned id;
    pma_action p_run;
};

class playlist_mclick_actions {
public:
    static pma g_pma_actions[];
    static unsigned id_to_idx(unsigned id);

    static bool run(unsigned id, bool on_item, unsigned idx)
    {
        g_pma_actions[id_to_idx(id)].p_run(on_item, idx);
        return true;
    }
    static unsigned get_count();
};

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

private:
    static LRESULT CALLBACK s_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    void on_create();
    void on_destroy();
    void set_title(const char* ptr);
    void update_taskbar_buttons(bool update);

    pfc::string8 m_window_title;
    mmh::ComPtr<ITaskbarList3> m_taskbar_list;
    user_interface::HookProc_t m_hook_proc{};
    bool m_should_handle_multimedia_keys{true};
    bool m_shell_hook_registered{};
    ULONG_PTR m_gdiplus_instance{NULL};
    bool m_gdiplus_initialised{false};
};

extern MainWindow main_window;
} // namespace cui

#endif
