#include "pch.h"
#include "item_details.h"
#include "config.h"
#include "dark_mode_dialog.h"
#include "font_picker.h"
#include "format_code_generator.h"

namespace cui::panels::item_details {

namespace {

const dark::DialogDarkModeConfig dark_mode_config{
    .button_ids = {IDC_GEN_COLOUR, IDC_FORMAT_CODE_GENERATOR, IDOK, IDCANCEL},
    .combo_box_ids = {IDC_HALIGN, IDC_VALIGN, IDC_EDGESTYLE},
    .edit_ids = {IDC_SCRIPT, IDC_COLOUR_CODE}};

} // namespace

INT_PTR CALLBACK ItemDetailsConfig::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
        /*case DM_GETDEFID:
        SetWindowLongPtr(wnd, DWLP_MSGRESULT, MAKELONG(m_modal ? IDOK : IDCANCEL, DC_HASDEFID));
        return TRUE;*/
    case WM_INITDIALOG: {
        m_wnd = wnd;

        if (!m_modal) {
            m_this->set_config_wnd(wnd);
            ShowWindow(GetDlgItem(wnd, IDOK), SW_HIDE);
            SetWindowText(GetDlgItem(wnd, IDCANCEL), L"Close");
        }

        uih::enhance_edit_control(wnd, IDC_SCRIPT);
        uSetWindowText(GetDlgItem(wnd, IDC_SCRIPT), m_script);
        HWND wnd_combo = GetDlgItem(wnd, IDC_EDGESTYLE);
        ComboBox_AddString(wnd_combo, L"None");
        ComboBox_AddString(wnd_combo, L"Sunken");
        ComboBox_AddString(wnd_combo, L"Grey");
        ComboBox_SetCurSel(wnd_combo, m_edge_style);

        wnd_combo = GetDlgItem(wnd, IDC_HALIGN);
        ComboBox_AddString(wnd_combo, L"Left");
        ComboBox_AddString(wnd_combo, L"Centre");
        ComboBox_AddString(wnd_combo, L"Right");
        ComboBox_SetCurSel(wnd_combo, m_horizontal_alignment);

        wnd_combo = GetDlgItem(wnd, IDC_VALIGN);
        ComboBox_AddString(wnd_combo, L"Top");
        ComboBox_AddString(wnd_combo, L"Centre");
        ComboBox_AddString(wnd_combo, L"Bottom");
        ComboBox_SetCurSel(wnd_combo, m_vertical_alignment);

        uih::enhance_edit_control(wnd, IDC_COLOUR_CODE);
        colour_code_gen(wnd, IDC_COLOUR_CODE, false, true);

        if (!m_modal) {
            SendMessage(wnd, DM_SETDEFID, IDCANCEL, NULL);
            SetFocus(GetDlgItem(wnd, IDCANCEL));
            return FALSE;
        }
        return FALSE;
    }
        return FALSE; // m_modal ? FALSE : TRUE;
    case WM_DESTROY:
        if (m_timer_active)
            on_timer();
        if (!m_modal)
            m_this->set_config_wnd(nullptr);
        break;
    case WM_NCDESTROY:
        m_wnd = nullptr;
        break;
    case WM_CLOSE:
        if (m_modal) {
            SendMessage(wnd, WM_COMMAND, IDCANCEL, NULL);
            return TRUE;
        }
        break;
    case WM_TIMER:
        if (wp == timer_id)
            on_timer();
        break;
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDOK:
            if (m_modal)
                EndDialog(wnd, 1);
            return TRUE;
        case IDCANCEL:
            if (m_modal)
                EndDialog(wnd, 0);
            else {
                DestroyWindow(wnd);
            }
            return TRUE;
        case IDC_GEN_COLOUR:
            colour_code_gen(wnd, IDC_COLOUR_CODE, false, false);
            break;
        case IDC_FORMAT_CODE_GENERATOR:
            utils::open_format_code_generator(m_wnd, g_guid_item_details_font_client);
            break;
        case IDC_SCRIPT:
            switch (HIWORD(wp)) {
            case EN_CHANGE:
                m_script = uGetWindowText(HWND(lp));
                if (!m_modal)
                    start_timer();
                break;
            }
            break;
        case IDC_EDGESTYLE:
            switch (HIWORD(wp)) {
            case CBN_SELCHANGE:
                m_edge_style = ComboBox_GetCurSel((HWND)lp);
                if (!m_modal) {
                    m_this->set_edge_style(m_edge_style);
                    cfg_item_details_edge_style = m_edge_style;
                }
                break;
            }
            break;
        case IDC_HALIGN:
            switch (HIWORD(wp)) {
            case CBN_SELCHANGE:
                m_horizontal_alignment = ComboBox_GetCurSel((HWND)lp);
                if (!m_modal) {
                    m_this->set_horizontal_alignment(m_horizontal_alignment);
                    cfg_item_details_horizontal_alignment = m_horizontal_alignment;
                }
                break;
            }
            break;
        case IDC_VALIGN:
            switch (HIWORD(wp)) {
            case CBN_SELCHANGE:
                m_vertical_alignment = static_cast<VerticalAlignment>(ComboBox_GetCurSel((HWND)lp));
                if (!m_modal) {
                    m_this->set_vertical_alignment(m_vertical_alignment);
                    cfg_item_details_vertical_alignment = WI_EnumValue(m_vertical_alignment);
                }
                break;
            }
            break;
        }
        break;
    }
    return FALSE;
}

void ItemDetailsConfig::on_timer()
{
    m_this->set_script(m_script);
    kill_timer();
}

void ItemDetailsConfig::start_timer()
{
    kill_timer();

    SetTimer(m_wnd, timer_id, 667, nullptr);
    m_timer_active = true;
}

void ItemDetailsConfig::kill_timer()
{
    if (m_timer_active) {
        KillTimer(m_wnd, timer_id);
        m_timer_active = false;
    }
}

void ItemDetailsConfig::run_modeless(HWND wnd, ItemDetails* p_this) &&
{
    m_modal = false;
    m_this = p_this;
    modeless_dialog_box(
        IDD_ITEM_DETAILS_OPTIONS, dark_mode_config, wnd, [config{std::move(*this)}](auto&&... args) mutable {
            return config.on_message(std::forward<decltype(args)>(args)...);
        });
}

bool ItemDetailsConfig::run_modal(HWND wnd)
{
    m_modal = true;
    const auto dialog_result = modal_dialog_box(IDD_ITEM_DETAILS_OPTIONS, dark_mode_config, wnd,
        [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); });
    return dialog_result > 0;
}

ItemDetailsConfig::ItemDetailsConfig(const char* p_text, uint32_t edge_style, uint32_t halign, VerticalAlignment valign)
    : m_script(p_text)
    , m_edge_style(edge_style)
    , m_horizontal_alignment(halign)
    , m_vertical_alignment(valign)
{
}

} // namespace cui::panels::item_details
