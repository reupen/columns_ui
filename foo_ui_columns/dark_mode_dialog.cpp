#include "pch.h"

#include "dark_mode_dialog.h"

#include "dark_mode_spin.h"

namespace cui::dark {

namespace {

class DialogDarkModeHelper {
public:
    explicit DialogDarkModeHelper(DialogDarkModeConfig config) : m_config(std::move(config)) {}

    [[nodiscard]] std::optional<INT_PTR> handle_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

private:
    [[nodiscard]] std::optional<INT_PTR> handle_wm_notify(HWND wnd, LPNMHDR lpnm);
    void on_dark_mode_change();
    void apply_dark_mode_attributes();
    void set_combobox_theme(auto&& ids, bool is_dark) const;
    void set_combobox_theme(bool is_dark) const;
    void set_edit_theme(auto&& ids, bool is_dark) const;
    void set_edit_theme(bool is_dark) const;
    void set_spin_theme(auto&& ids, bool is_dark) const;
    void set_spin_theme(bool is_dark) const;
    void set_tree_view_theme(auto&& ids, bool is_dark) const;
    void set_tree_view_theme(bool is_dark) const;
    void set_window_theme(auto&& ids, const wchar_t* dark_class, bool is_dark);

    HWND m_wnd{};
    wil::unique_hbrush m_main_background_brush;
    wil::unique_hbrush m_edit_background_brush;
    wil::unique_htheme m_button_theme;
    DialogDarkModeConfig m_config;
    mmh::EventToken::Ptr m_dark_mode_status_callback;
};

void DialogDarkModeHelper::set_tree_view_theme(auto&& ids, bool is_dark) const
{
    if (!m_wnd)
        return;

    for (const auto id : ids) {
        const auto tree_view_wnd = GetDlgItem(m_wnd, id);
        SetWindowTheme(tree_view_wnd, is_dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
        TreeView_SetBkColor(tree_view_wnd, is_dark ? get_dark_colour(ColourID::TreeViewBackground) : -1);
        TreeView_SetTextColor(tree_view_wnd, is_dark ? get_dark_colour(ColourID::TreeViewText) : -1);
    }
}

void DialogDarkModeHelper::set_tree_view_theme(bool is_dark) const
{
    set_tree_view_theme(m_config.tree_view_ids, is_dark);
}

void DialogDarkModeHelper::set_combobox_theme(auto&& ids, bool is_dark) const
{
    if (!m_wnd)
        return;

    for (const auto id : ids) {
        const auto combobox_wnd = GetDlgItem(m_wnd, id);
        SetWindowTheme(combobox_wnd, is_dark ? L"DarkMode_CFD" : nullptr, nullptr);

        COMBOBOXINFO cbi{};
        cbi.cbSize = sizeof cbi;

        if (GetComboBoxInfo(combobox_wnd, &cbi) && cbi.hwndList)
            SetWindowTheme(cbi.hwndList, is_dark ? L"DarkMode_Explorer" : nullptr, nullptr);
    }
}

void DialogDarkModeHelper::set_combobox_theme(bool is_dark) const
{
    set_combobox_theme(m_config.combo_box_ids, is_dark);
}

void DialogDarkModeHelper::set_edit_theme(auto&& ids, bool is_dark) const
{
    if (!m_wnd)
        return;

    for (const auto id : ids) {
        const auto edit_wnd = GetDlgItem(m_wnd, id);
        const auto is_scrollable = (GetWindowLongPtr(edit_wnd, GWL_STYLE) & (WS_VSCROLL | WS_HSCROLL)) != 0;
        const auto dark_class = is_scrollable ? L"DarkMode_Explorer" : L"DarkMode_CFD";
        SetWindowTheme(edit_wnd, is_dark ? dark_class : L"Explorer", nullptr);
    }
}

void DialogDarkModeHelper::set_edit_theme(bool is_dark) const
{
    set_edit_theme(m_config.edit_ids, is_dark);
}

void DialogDarkModeHelper::set_spin_theme(auto&& ids, bool is_dark) const
{
    if (!m_wnd)
        return;

    for (const auto id : ids) {
        const auto spin_wnd = GetDlgItem(m_wnd, id);
        SetWindowTheme(spin_wnd, is_dark ? L"DarkMode_Explorer" : nullptr, nullptr);

        if (is_dark)
            spin::add_window(spin_wnd);
        else
            spin::remove_window(spin_wnd);
    }
}

void DialogDarkModeHelper::set_spin_theme(bool is_dark) const
{
    set_spin_theme(m_config.spin_ids, is_dark);
}

void DialogDarkModeHelper::set_window_theme(auto&& ids, const wchar_t* dark_class, bool is_dark)
{
    if (!m_wnd)
        return;

    for (const auto id : ids)
        if (const auto control_wnd = GetDlgItem(m_wnd, id))
            SetWindowTheme(control_wnd, is_dark ? dark_class : nullptr, nullptr);
}

void DialogDarkModeHelper::apply_dark_mode_attributes()
{
    const auto is_dark = is_active_ui_dark();
    set_titlebar_mode(m_wnd, is_dark);
    set_window_theme(m_config.button_ids, L"DarkMode_Explorer", is_dark);
    set_window_theme(m_config.checkbox_ids, L"DarkMode_Explorer", is_dark);
    set_combobox_theme(is_dark);
    set_edit_theme(is_dark);
    set_window_theme(m_config.list_box_ids, L"DarkMode_Explorer", is_dark);
    set_spin_theme(is_dark);
    set_tree_view_theme(is_dark);
}

std::optional<INT_PTR> DialogDarkModeHelper::handle_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG:
        m_wnd = wnd;
        apply_dark_mode_attributes();
        m_dark_mode_status_callback = add_status_callback([this] { on_dark_mode_change(); });
        break;
    case WM_NCDESTROY:
        m_dark_mode_status_callback.reset();
        m_main_background_brush.reset();
        m_edit_background_brush.reset();
        m_wnd = nullptr;
        break;
    case WM_PARENTNOTIFY:
        if (LOWORD(wp) == WM_CREATE) {
            const auto id = HIWORD(wp);
            const auto ids = {id};
            const auto is_dark = is_active_ui_dark();

            if (m_config.button_ids.contains(id) || m_config.checkbox_ids.contains(id)
                || m_config.list_box_ids.contains(id))
                set_window_theme(ids, L"DarkMode_Explorer", is_dark);
            else if (m_config.combo_box_ids.contains(id))
                set_combobox_theme(ids, is_dark);
            else if (m_config.edit_ids.contains(id))
                set_edit_theme(ids, is_dark);
            else if (m_config.spin_ids.contains(id))
                set_spin_theme(ids, is_dark);
            else if (m_config.tree_view_ids.contains(id))
                set_tree_view_theme(ids, is_dark);
        }
        break;
    case WM_THEMECHANGED:
        m_button_theme.reset();
        break;
    case WM_ERASEBKGND:
        SetWindowLongPtr(wnd, DWLP_MSGRESULT, TRUE);
        return TRUE;
    case WM_PAINT:
        handle_modern_background_paint(wnd, GetDlgItem(wnd, m_config.last_button_id), is_active_ui_dark());
        return TRUE;
    case WM_NOTIFY:
        if (const auto result = handle_wm_notify(wnd, reinterpret_cast<LPNMHDR>(lp)))
            return result;
        break;
    case WM_CTLCOLOREDIT: {
        const auto is_dark = is_active_ui_dark();

        if (!is_dark)
            break;

        if (!m_edit_background_brush)
            m_edit_background_brush = get_colour_brush(ColourID::EditBackground, true);

        const auto dc = reinterpret_cast<HDC>(wp);
        SetBkColor(dc, get_colour(ColourID::EditBackground, true));
        SetTextColor(dc, get_system_colour(COLOR_WINDOWTEXT, true));
        return reinterpret_cast<INT_PTR>(m_edit_background_brush.get());
    }
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORSTATIC: {
        const auto is_dark = is_active_ui_dark();

        if (!m_main_background_brush)
            m_main_background_brush = get_system_colour_brush(COLOR_WINDOW, is_dark);

        const auto dc = reinterpret_cast<HDC>(wp);
        SetBkColor(dc, get_system_colour(COLOR_WINDOW, is_dark));
        SetTextColor(dc, get_system_colour(COLOR_WINDOWTEXT, is_dark));
        return reinterpret_cast<INT_PTR>(m_main_background_brush.get());
    }
    }

    return {};
}

std::optional<INT_PTR> DialogDarkModeHelper::handle_wm_notify(HWND wnd, LPNMHDR lpnm)
{
    switch (lpnm->code) {
    case NM_CUSTOMDRAW: {
        if (lpnm->idFrom > gsl::narrow_cast<UINT_PTR>(std::numeric_limits<int>::max()))
            break;

        if (!m_config.checkbox_ids.contains(gsl::narrow_cast<int>(lpnm->idFrom)) || !is_active_ui_dark())
            break;

        const auto lpnmcd = reinterpret_cast<LPNMCUSTOMDRAW>(lpnm);

        switch (lpnmcd->dwDrawStage) {
        case CDDS_PREPAINT:
            const auto dc = lpnmcd->hdc;

            if (!m_button_theme)
                m_button_theme.reset(OpenThemeData(wnd, L"Button"));

            SIZE box_size{};
            if (FAILED(GetThemePartSize(m_button_theme.get(), dc, BP_CHECKBOX, 0, nullptr, TS_DRAW, &box_size)))
                break;

            const auto text_length = GetWindowTextLength(lpnm->hwndFrom);
            std::vector<wchar_t> text(text_length + 1);
            GetWindowText(lpnm->hwndFrom, text.data(), text_length + 1);

            SIZE zero_digit_size{};
            GetTextExtentPoint32(dc, L"0", 1, &zero_digit_size);

            RECT rect_text = lpnmcd->rc;
            rect_text.left += box_size.cx + zero_digit_size.cx / 2;

            const auto is_disabled = (lpnmcd->uItemState & CDIS_DISABLED) != 0;
            const auto text_colour
                = get_dark_colour(is_disabled ? ColourID::CheckboxDisabledText : ColourID::CheckboxText);
            SetTextColor(dc, text_colour);

            // Multi-line text not currently handled
            DrawTextEx(dc, text.data(), gsl::narrow<int>(text.size()), &rect_text,
                DT_HIDEPREFIX | DT_SINGLELINE | DT_VCENTER, nullptr);

            SetWindowLongPtr(wnd, DWLP_MSGRESULT, CDRF_SKIPDEFAULT);
            return TRUE;
        }
        break;
    }
    }

    return {};
}

void DialogDarkModeHelper::on_dark_mode_change()
{
    if (!m_wnd)
        return;

    m_main_background_brush.reset();

    SetWindowRedraw(m_wnd, FALSE);
    apply_dark_mode_attributes();
    SetWindowRedraw(m_wnd, TRUE);
    RedrawWindow(m_wnd, nullptr, nullptr,
        RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_FRAME | RDW_UPDATENOW | RDW_ERASENOW);

    force_titlebar_redraw(m_wnd);
}

} // namespace

INT_PTR modal_dialog_box(UINT resource_id, DialogDarkModeConfig dark_mode_config, HWND parent_window,
    std::function<INT_PTR(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)> on_message, bool poke)
{
    return fbh::modal_dialog_box(
        resource_id, parent_window,
        [helper = std::make_shared<DialogDarkModeHelper>(std::move(dark_mode_config)),
            on_message{std::move(on_message)}](HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
            if (const auto result = helper->handle_message(wnd, msg, wp, lp))
                return *result;

            return on_message(wnd, msg, wp, lp);
        },
        poke);
}

HWND modeless_dialog_box(UINT resource_id, DialogDarkModeConfig dark_mode_config, HWND parent_window,
    std::function<INT_PTR(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)> on_message)
{
    const auto wnd = fbh::modeless_dialog_box(resource_id, parent_window,
        [helper = std::make_shared<DialogDarkModeHelper>(std::move(dark_mode_config)),
            on_message{std::move(on_message)}](HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
            if (const auto result = helper->handle_message(wnd, msg, wp, lp))
                return *result;

            return on_message(wnd, msg, wp, lp);
        });

    return wnd;
}

void modeless_info_box(HWND wnd_parent, const char* title, const char* message, uih::InfoBoxType type, bool no_wrap)
{
    const auto handle_before_message
        = [helper = std::make_shared<DialogDarkModeHelper>(
               DialogDarkModeConfig{.button_ids{IDCANCEL}, .last_button_id = IDCANCEL})](HWND wnd, UINT msg, WPARAM wp,
              LPARAM lp) -> std::optional<INT_PTR> { return helper->handle_message(wnd, msg, wp, lp); };

    fbh::show_info_box_modeless(wnd_parent, title, message, type, no_wrap, handle_before_message);
}

INT_PTR modal_info_box(HWND wnd_parent, const char* title, const char* message, uih::InfoBoxType type,
    uih::InfoBoxModalType modal_type, bool no_wrap)
{
    const auto handle_before_message
        = [helper = std::make_shared<DialogDarkModeHelper>(
               DialogDarkModeConfig{.button_ids{IDCANCEL, IDOK}, .last_button_id = IDCANCEL})](HWND wnd, UINT msg,
              WPARAM wp, LPARAM lp) -> std::optional<INT_PTR> { return helper->handle_message(wnd, msg, wp, lp); };

    return fbh::show_info_box_modal(wnd_parent, title, message, type, modal_type, no_wrap, handle_before_message);
}

} // namespace cui::dark
