#include "pch.h"
#include "fcl.h"
#include "config.h"
#include "event_token.h"
#include "main_window.h"

namespace cui::prefs {

namespace {

std::vector<std::shared_ptr<GenericEventHandler>> use_hardware_acceleration_changed_handlers;

class SetupTab : public PreferencesTab {
public:
    BOOL handle_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            uih::enhance_edit_control(wnd, IDC_STRING);
            SendDlgItemMessage(wnd, IDC_TRANSPARENCY_SPIN, UDM_SETRANGE32, 0, 255);

            if (!main_window.get_wnd())
                EnableWindow(GetDlgItem(wnd, IDC_QUICKSETUP), FALSE);

            break;
        }
        case WM_COMMAND:
            switch (wp) {
            case IDC_QUICKSETUP:
                SendMessage(main_window.get_wnd(), MSG_RUN_INITIAL_SETUP, NULL, NULL);
                break;
            case IDC_FCL_EXPORT:
                g_export_layout(wnd);
                break;
            case IDC_FCL_IMPORT:
                g_import_layout(wnd);
                break;
            case IDC_HARDWARE_ACCELERATION:
                config::use_hardware_acceleration = Button_GetCheck(reinterpret_cast<HWND>(lp)) == BST_CHECKED;

                for (auto&& handler : use_hardware_acceleration_changed_handlers)
                    (*handler)();

                break;
            }
        }
        return 0;
    }

    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_SETUP,
            [this](auto&&... args) { return handle_message(std::forward<decltype(args)>(args)...); });
    }

    const char* get_name() override { return "Setup"; }

    PreferencesTabHelper m_helper{{IDC_TITLE1, IDC_TITLE2}};
};

SetupTab setup_tab;

} // namespace

PreferencesTab& get_setup_tab()
{
    return setup_tab;
}

std::unique_ptr<EventToken> add_use_hardware_acceleration_changed_handler(std::function<void()> event_handler)
{
    return make_event_token(use_hardware_acceleration_changed_handlers, std::move(event_handler));
}

} // namespace cui::prefs
