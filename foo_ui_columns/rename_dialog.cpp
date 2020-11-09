#include "stdafx.h"
#include "rename_dialog.h"

namespace cui::helpers {

class RenameDialogBoxState {
public:
    pfc::string8 m_text;
    pfc::string8 m_title;
    modal_dialog_scope m_scope;
};

static BOOL CALLBACK show_rename_dialog_box_proc(RenameDialogBoxState& state, HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG:
        state.m_scope.initialize(FindOwningPopup(wnd));
        uSetWindowText(wnd, state.m_title);
        uSetDlgItemText(wnd, IDC_EDIT, state.m_text);
        return TRUE;
    case WM_COMMAND:
        switch (wp) {
        case IDOK: {
            uGetDlgItemText(wnd, IDC_EDIT, state.m_text);
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

    const auto dialog_result = uih::dpi::modal_dialog_box(IDD_RENAME_PLAYLIST, wnd_parent,
        [&param](auto&&... args) { return show_rename_dialog_box_proc(param, std::forward<decltype(args)>(args)...); });

    if (dialog_result > 0)
        return {param.m_text};

    return {};
}

} // namespace cui::helpers
