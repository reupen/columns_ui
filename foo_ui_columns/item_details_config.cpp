#include "stdafx.h"
#include "item_details.h"
#include "config.h"

BOOL CALLBACK item_details_config_t::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
        /*case DM_GETDEFID:
        SetWindowLongPtr(wnd, DWLP_MSGRESULT, MAKELONG(m_modal ? IDOK : IDCANCEL, DC_HASDEFID));
        return TRUE;*/
    case WM_INITDIALOG: {
        m_wnd = wnd;

        if (!m_modal) {
            modeless_dialog_manager::g_add(wnd);
            m_this->set_config_wnd(wnd);
            ShowWindow(GetDlgItem(wnd, IDOK), SW_HIDE);
            SetWindowText(GetDlgItem(wnd, IDCANCEL), L"Close");
        }

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

        LOGFONT lf;
        static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_item_details_font_client, lf);
        m_font_code_generator.initialise(lf, wnd, IDC_FONT_CODE);

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
        if (!m_modal) {
            modeless_dialog_manager::g_remove(wnd);
            SetWindowLongPtr(wnd, DWLP_USER, NULL);
            delete this;
        }
        break;
    case WM_ERASEBKGND:
        SetWindowLongPtr(wnd, DWLP_MSGRESULT, TRUE);
        return TRUE;
    case WM_PAINT:
        uih::handle_modern_background_paint(wnd, GetDlgItem(wnd, IDOK));
        return TRUE;
    case WM_CTLCOLORSTATIC:
        SetBkColor((HDC)wp, GetSysColor(COLOR_WINDOW));
        SetTextColor((HDC)wp, GetSysColor(COLOR_WINDOWTEXT));
        return (BOOL)GetSysColorBrush(COLOR_WINDOW);
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
        case IDC_GEN_FONT:
            m_font_code_generator.run(wnd, IDC_FONT_CODE);
            break;
        case IDC_SCRIPT:
            switch (HIWORD(wp)) {
            case EN_CHANGE:
                m_script = string_utf8_from_window(HWND(lp));
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
                m_vertical_alignment = ComboBox_GetCurSel((HWND)lp);
                if (!m_modal) {
                    m_this->set_vertical_alignment(m_vertical_alignment);
                    cfg_item_details_vertical_alignment = m_vertical_alignment;
                }
                break;
            }
            break;
        }
        break;
    }
    return FALSE;
}

void item_details_config_t::on_timer()
{
    m_this->set_script(m_script);
    kill_timer();
}

void item_details_config_t::start_timer()
{
    kill_timer();

    SetTimer(m_wnd, timer_id, 667, nullptr);
    m_timer_active = true;
}

void item_details_config_t::kill_timer()
{
    if (m_timer_active) {
        KillTimer(m_wnd, timer_id);
        m_timer_active = false;
    }
}

void item_details_config_t::run_modeless(HWND wnd, item_details_t* p_this)
{
    m_modal = false;
    m_this = p_this;
    if (!uCreateDialog(IDD_ITEMDETAILS_CONFIG, wnd, g_DialogProc, (LPARAM)this))
        delete this;
}

bool item_details_config_t::run_modal(HWND wnd)
{
    m_modal = true;
    return uDialogBox(IDD_ITEMDETAILS_CONFIG, wnd, g_DialogProc, (LPARAM)this) != 0;
}

item_details_config_t::item_details_config_t(const char* p_text, t_size edge_style, t_size halign, t_size valign)
    : m_script(p_text)
    , m_edge_style(edge_style)
    , m_horizontal_alignment(halign)
    , m_vertical_alignment(valign)
    , m_modal(true)
    , m_timer_active(false)
    , m_wnd(nullptr)
{
}

BOOL CALLBACK item_details_config_t::g_DialogProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    item_details_config_t* p_data = nullptr;
    if (msg == WM_INITDIALOG) {
        p_data = reinterpret_cast<item_details_config_t*>(lp);
        SetWindowLongPtr(wnd, DWLP_USER, lp);
    } else
        p_data = reinterpret_cast<item_details_config_t*>(GetWindowLongPtr(wnd, DWLP_USER));
    return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
}
