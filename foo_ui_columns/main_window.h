#pragma once
#include "icons.h"

namespace colours {
enum ColourID {
    COLOUR_TEXT,
    COLOUR_SELECTED_TEXT,
    COLOUR_BACK,
    COLOUR_SELECTED_BACK,
    COLOUR_SELECTED_BACK_NO_FOCUS,
    COLOUR_SELECTED_TEXT_NO_FOCUS,
    COLOUR_FRAME,
};
} // namespace colours

COLORREF get_default_colour(colours::ColourID index, bool themed = false);

/** Main window UI control IDs */
#define ID_REBAR 2100
#define ID_STATUS 2002

#define ID_PLAYLIST_TOOLTIP 50

/** Main window custom message numbers */
enum {
    MSG_SET_AOT = WM_USER + 3,
    MSG_UPDATE_TITLE,
    MSG_RUN_INITIAL_SETUP,
    MSG_NOTIFICATION_ICON,
    MSG_CREATE_TASKBAR_BUTTONS,
    MSG_UPDATE_TASKBAR_BUTTONS
};

bool remember_window_pos();

bool process_keydown(UINT msg, LPARAM lp, WPARAM wp, bool playlist = false, bool keyb = true);

void on_show_status_change();
void on_show_status_pane_change();
void on_show_toolbars_change();

namespace taskbar_buttons {
enum {
    ID_FIRST = 667,
    ID_STOP = ID_FIRST,
    ID_PREV,
    ID_PLAY_OR_PAUSE,
    ID_NEXT,
    ID_RAND
};
} // namespace taskbar_buttons

namespace cui {
class MainWindow {
public:
    HWND initialise(user_interface::HookProc_t hook);
    void shutdown();
    void on_query_capability();
    void update_title();
    void reset_title();
    void set_dark_mode_attributes(bool is_update = false) const;

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
    static constexpr std::array taskbar_icon_configs{icons::built_in::stop, icons::built_in::previous,
        icons::built_in::pause, icons::built_in::play, icons::built_in::next, icons::built_in::random};

    static LRESULT CALLBACK s_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    static void warn_if_ui_hacks_installed();

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    void on_create();
    void on_destroy();
    void create_child_windows();
    void set_title(const char* ptr);
    bool update_taskbar_button_images() const;
    void update_taskbar_buttons(bool update) const;

    pfc::string8 m_window_title;
    wil::com_ptr_t<ITaskbarList3> m_taskbar_list;
    HWND m_wnd{};
    HMONITOR m_monitor{};
    user_interface::HookProc_t m_hook_proc{};
    bool m_should_handle_multimedia_keys{true};
    bool m_shell_hook_registered{};
    ULONG_PTR m_gdiplus_instance{NULL};
    bool m_gdiplus_initialised{false};
    wil::unique_himagelist m_taskbar_button_images;
};

extern MainWindow main_window;
} // namespace cui
