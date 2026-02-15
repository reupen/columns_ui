#include "pch.h"
#include "fcl.h"
#include "config.h"
#include "dark_mode.h"
#include "main_window.h"

namespace cui::prefs {

namespace {

std::vector<std::shared_ptr<mmh::GenericEventHandler>> use_hardware_acceleration_changed_handlers;

class SetupTab : public PreferencesTab {
public:
    BOOL handle_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            uih::enhance_edit_control(wnd, IDC_STRING);

            const auto documentation_link_wnd = GetDlgItem(wnd, IDC_DOCUMENTATION_LINK);

            LITEM item{};
            item.mask = LIF_STATE | LIF_ITEMINDEX;
            item.iLink = 0;
            item.state = LIS_DEFAULTCOLORS;
            item.stateMask = LIS_DEFAULTCOLORS;
            SendMessage(documentation_link_wnd, LM_SETITEM, 0, reinterpret_cast<LPARAM>(&item));

            SendDlgItemMessage(wnd, IDC_TRANSPARENCY_SPIN, UDM_SETRANGE32, 0, 255);

            if (config::use_hardware_acceleration)
                Button_SetCheck(GetDlgItem(wnd, IDC_HARDWARE_ACCELERATION), BST_CHECKED);

            if (!main_window.get_wnd())
                EnableWindow(GetDlgItem(wnd, IDC_QUICKSETUP), FALSE);

            uih::subclass_window(wnd,
                [documentation_link_wnd](auto wndproc, auto wnd, auto msg, auto wp, auto lp) -> std::optional<LRESULT> {
                    if (msg == WM_CTLCOLORSTATIC && reinterpret_cast<HWND>(lp) == documentation_link_wnd) {
                        const auto result = CallWindowProc(wndproc, wnd, msg, wp, lp);
                        const auto colour
                            = dark::get_colour(dark::ColourID::HyperlinkText, ui_config_manager::g_is_dark_mode());
                        SetTextColor(reinterpret_cast<HDC>(wp), colour);
                        return result;
                    }

                    return {};
                });

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
            break;
        case WM_NOTIFY: {
            const auto nmhdr = reinterpret_cast<LPNMHDR>(lp);

            if (nmhdr->idFrom != IDC_DOCUMENTATION_LINK)
                break;

            switch (nmhdr->code) {
            case NM_CLICK:
            case NM_RETURN: {
                PNMLINK nmlink = reinterpret_cast<PNMLINK>(lp);
                helpers::open_web_page(wnd, nmlink->item.szUrl);
                break;
            }
            }
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

    PreferencesTabHelper m_helper{{IDC_TITLE1, IDC_TITLE2, IDC_TITLE3}};
};

SetupTab setup_tab;

} // namespace

PreferencesTab& get_setup_tab()
{
    return setup_tab;
}

mmh::EventToken::Ptr add_use_hardware_acceleration_changed_handler(mmh::GenericEventHandler event_handler)
{
    return mmh::make_event_token(use_hardware_acceleration_changed_handlers, std::move(event_handler));
}

} // namespace cui::prefs
