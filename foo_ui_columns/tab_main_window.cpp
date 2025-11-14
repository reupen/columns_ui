#include "pch.h"
#include "fcl.h"
#include "config.h"
#include "dark_mode_dialog.h"
#include "rebar.h"
#include "main_window.h"

namespace cui::prefs {

namespace {

class MainWindowTab : public PreferencesTab {
public:
    void refresh_transparency_enabled() const
    {
        if (!m_wnd)
            return;

        SendDlgItemMessage(m_wnd, IDC_USE_TRANSPARENCY, BM_SETCHECK,
            main_window::config_get_transparency_enabled() ? BST_CHECKED : BST_UNCHECKED, 0);
    }

    BOOL handle_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            m_wnd = wnd;
            uih::enhance_edit_control(wnd, IDC_STRING);
            SendDlgItemMessage(wnd, IDC_TRANSPARENCY_SPIN, UDM_SETRANGE32, 0, 255);
            SendDlgItemMessage(wnd, IDC_TOOLBARS, BM_SETCHECK, cfg_toolbars, 0);
            refresh_transparency_enabled();
            SendDlgItemMessage(
                wnd, IDC_TRANSPARENCY_SPIN, UDM_SETPOS32, 0, main_window::config_get_transparency_level());

            uSendDlgItemMessageText(
                wnd, IDC_STRING, WM_SETTEXT, NULL, main_window::config_main_window_title_script.get());
            m_initialised = true;
            break;
        }
        case WM_DESTROY:
            m_initialised = false;
            m_wnd = nullptr;
            break;
        case WM_COMMAND:
            switch (wp) {
            case EN_CHANGE << 16 | IDC_STRING:
                main_window::config_main_window_title_script.set(uGetWindowText((HWND)lp));
                break;
            case EN_CHANGE << 16 | IDC_TRANSPARENCY_LEVEL: {
                if (!m_initialised)
                    break;

                BOOL succeeded{};
                const unsigned new_value = GetDlgItemInt(wnd, IDC_TRANSPARENCY_LEVEL, &succeeded, FALSE);

                if (succeeded)
                    main_window::config_set_transparency_level(
                        gsl::narrow_cast<uint8_t>(std::clamp(new_value, 0u, 255u)));

                break;
            }
            case IDC_USE_TRANSPARENCY:
                main_window::config_set_transparency_enabled(
                    SendMessage(reinterpret_cast<HWND>(lp), BM_GETCHECK, 0, 0) == BST_CHECKED);
                break;
            case IDC_TOOLBARS:
                cfg_toolbars = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;
                on_show_toolbars_change();
                break;
            case IDC_RESET_TOOLBARS:
                if (!dark::modal_info_box(wnd, "Reset toolbars",
                        "Warning! This will reset the toolbars to the default state. Continue?",
                        uih::InfoBoxType::Neutral, uih::InfoBoxModalType::YesNo))
                    break;

                if (main_window.get_wnd())
                    rebar::destroy_rebar();

                rebar::g_cfg_rebar.reset();

                if (main_window.get_wnd()) {
                    rebar::create_rebar();

                    if (rebar::g_rebar) {
                        ShowWindow(rebar::g_rebar, SW_SHOWNORMAL);
                        UpdateWindow(rebar::g_rebar);
                    }

                    main_window.resize_child_windows();
                }
                break;
            }
        }
        return 0;
    }

    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_MAIN_WINDOW,
            [this](auto&&... args) { return handle_message(std::forward<decltype(args)>(args)...); });
    }

    const char* get_name() override { return "Main window"; }

private:
    bool m_initialised{};
    HWND m_wnd{};
    PreferencesTabHelper m_helper{{IDC_TITLE1, IDC_TITLE2}};
};

MainWindowTab main_window_tab;

} // namespace

PreferencesTab& get_main_window_tab()
{
    return main_window_tab;
}

void update_main_window_tab_transparency_enabled()
{
    main_window_tab.refresh_transparency_enabled();
}

} // namespace cui::prefs
