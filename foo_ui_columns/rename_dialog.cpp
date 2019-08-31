#include "stdafx.h"
#include "rename_dialog.h"

namespace cui::helpers {

class RenameDialogBoxState {
public:
    pfc::string8 m_text;
    pfc::string8 m_title;
    modal_dialog_scope m_scope;
};

static BOOL CALLBACK show_rename_dialog_box_proc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG:
        SetWindowLongPtr(wnd, DWLP_USER, lp);
        {
            auto* ptr = reinterpret_cast<RenameDialogBoxState*>(lp);
            ptr->m_scope.initialize(FindOwningPopup(wnd));
            uSetWindowText(wnd, ptr->m_title);
            uSetDlgItemText(wnd, IDC_EDIT, ptr->m_text);
        }
        return TRUE;
    case WM_COMMAND:
        switch (wp) {
        case IDOK: {
            auto* ptr = reinterpret_cast<RenameDialogBoxState*>(GetWindowLong(wnd, DWLP_USER));
            uGetDlgItemText(wnd, IDC_EDIT, ptr->m_text);
            EndDialog(wnd, 1);
            return FALSE;
        }
        case IDCANCEL:
            EndDialog(wnd, 0);
            return FALSE;
        default:
            return FALSE;
        }
    case WM_CLOSE:
        EndDialog(wnd, 0);
        return FALSE;
    default:
        return FALSE;
    }
}

std::optional<pfc::string8> show_rename_dialog_box(HWND wnd_parent, const char* title, const char* initial_text)
{
    RenameDialogBoxState param;
    param.m_title = title;
    param.m_text = initial_text;
    const auto dialog_result = DialogBoxParam(mmh::get_current_instance(), MAKEINTRESOURCE(IDD_RENAME_PLAYLIST),
        wnd_parent, show_rename_dialog_box_proc, reinterpret_cast<LPARAM>(&param));

    if (dialog_result > 0)
        return {param.m_text};

    return {};
}

} // namespace cui::helpers
