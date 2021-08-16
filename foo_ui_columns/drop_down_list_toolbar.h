#pragma once

template <class ToolbarArgs>
class DropDownListToolbar : public ui_extension::container_ui_extension {
public:
    static void s_update_active_item_safe()
    {
        for (auto&& window : s_windows) {
            window->update_active_item_safe();
        }
    }

    static void s_refresh_all_items_safe()
    {
        for (auto&& window : s_windows) {
            window->refresh_all_items_safe();
        }
    }

    static void s_update_colours()
    {
        cui::colours::helper helper(GUID{});

        s_background_brush.reset(CreateSolidBrush(helper.get_colour(cui::colours::colour_background)));

        for (auto&& window : s_windows) {
            const HWND wnd = window->m_wnd_combo;
            if (wnd)
                RedrawWindow(wnd, nullptr, nullptr, RDW_INVALIDATE);
        }
    }

    void refresh_all_items_safe();
    void update_active_item_safe();

    const GUID& get_extension_guid() const override { return ToolbarArgs::extension_guid; }
    unsigned get_type() const override { return ui_extension::type_toolbar; };
    void get_name(pfc::string_base& out) const override { out = ToolbarArgs::name; }
    void get_category(pfc::string_base& out) const override { out.set_string("Toolbars"); }
    bool is_available(const uie::window_host_ptr& p_host) const override { return ToolbarArgs::is_available(); }
    void get_menu_items(uie::menu_hook_t& p_hook) override { ToolbarArgs::get_menu_items(p_hook); }
    class_data& get_class_data() const override
    {
        __implement_get_class_data_child_ex(ToolbarArgs::class_name, false, false);
    }

private:
    class ColourCallback : public cui::colours::common_callback {
    public:
        void on_bool_changed(t_size mask) const override{};
        void on_colour_changed(t_size mask) const override { DropDownListToolbar<ToolbarArgs>::s_update_colours(); }
    };

    static LRESULT WINAPI s_on_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        auto p_this = reinterpret_cast<DropDownListToolbar*>(GetWindowLongPtr(wnd, GWLP_USERDATA));
        return p_this ? p_this->on_hook(wnd, msg, wp, lp) : DefWindowProc(wnd, msg, wp, lp);
    }

    LRESULT WINAPI on_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;

    static constexpr unsigned ID_COMBOBOX = 1001;
    static constexpr unsigned initial_height = 300;
    static constexpr unsigned initial_width = 150;
    static constexpr unsigned maximum_minimum_width = 150;
    static HFONT s_icon_font;
    static ColourCallback s_colour_callback;
    static wil::unique_hbrush s_background_brush;
    static std::vector<DropDownListToolbar<ToolbarArgs>*> s_windows;

    void refresh_all_items();
    void update_active_item();

    typename ToolbarArgs::ItemList m_items;
    HWND m_wnd_combo{nullptr};
    WNDPROC m_order_proc{nullptr};
    int m_max_item_width{0};
    int m_height{0};
    bool m_initialised{false};
    bool m_process_next_char{true};
    bool m_processing_selection_change{false};
    t_int32 m_mousewheel_delta{0};
};

template <class ToolbarArgs>
void DropDownListToolbar<ToolbarArgs>::refresh_all_items_safe()
{
    if (m_processing_selection_change) {
        fb2k::inMainThread([self = ptr{this}, this] {
            if (get_wnd())
                refresh_all_items();
        });
        return;
    }

    refresh_all_items();
}

template <class ToolbarArgs>
void DropDownListToolbar<ToolbarArgs>::update_active_item_safe()
{
    if (m_processing_selection_change) {
        fb2k::inMainThread([self = ptr{this}, this] {
            if (get_wnd())
                update_active_item();
        });
        return;
    }

    update_active_item();
}

template <class ToolbarArgs>
void DropDownListToolbar<ToolbarArgs>::refresh_all_items()
{
    const HDC dc = GetDC(m_wnd_combo);
    const HFONT prev_font = SelectFont(dc, s_icon_font);

    auto&& active_item_id = ToolbarArgs::get_active_item();
    m_items = ToolbarArgs::get_items();

    ComboBox_ResetContent(m_wnd_combo);
    ComboBox_Enable(m_wnd_combo, !ranges::empty(m_items));

    if (ranges::empty(m_items)) {
        if (ToolbarArgs::get_items_empty_text() != nullptr) {
            const auto items_empty_text = ToolbarArgs::get_items_empty_text();

            ComboBox_AddString(m_wnd_combo, pfc::stringcvt::string_wide_from_utf8(items_empty_text));
            const auto cx = uih::get_text_width(dc, items_empty_text, std::strlen(items_empty_text));
            m_max_item_width = max(m_max_item_width, cx);
        } else {
            m_max_item_width = max(m_max_item_width, initial_width);
        }
    } else {
        // auto&& crashes the VS 2017 15.6 compiler here
        for (auto& [id, name] : m_items) {
            ComboBox_AddString(m_wnd_combo, pfc::stringcvt::string_wide_from_utf8(name.c_str()));
            const auto cx = uih::get_text_width(dc, name.c_str(), name.size());
            m_max_item_width = max(m_max_item_width, cx);
        }
    }

    SelectFont(dc, prev_font);
    ReleaseDC(m_wnd_combo, dc);

    update_active_item();
}

template <class ToolbarArgs>
void DropDownListToolbar<ToolbarArgs>::update_active_item()
{
    if (ranges::empty(m_items) && ToolbarArgs::get_items_empty_text() != nullptr) {
        ComboBox_SetCurSel(m_wnd_combo, 0);
        return;
    }

    auto&& id = ToolbarArgs::get_active_item();
    const auto iter = std::find_if(
        m_items.begin(), m_items.end(), [id](auto&& item) { return std::get<typename ToolbarArgs::ID>(item) == id; });

    const int sel_item_index = iter != m_items.end() ? iter - m_items.begin() : -1;
    ComboBox_SetCurSel(m_wnd_combo, sel_item_index);
}

template <class ToolbarArgs>
LRESULT DropDownListToolbar<ToolbarArgs>::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        s_windows.emplace_back(this);
        if (s_windows.size() == 1) {
            ToolbarArgs::on_first_window_created();

            fb2k::std_api_get<cui::colours::manager>()->register_common_callback(&s_colour_callback);
        }

        if (!s_icon_font)
            s_icon_font = uCreateIconFont();

        if (!s_background_brush)
            s_update_colours();

        m_wnd_combo = CreateWindowEx(0, WC_COMBOBOX, nullptr,
            CBS_DROPDOWNLIST | WS_CHILD | WS_TABSTOP | WS_VISIBLE | WS_VSCROLL, 0, 0,
            uih::scale_dpi_value(initial_width), uih::scale_dpi_value(initial_height), wnd,
            reinterpret_cast<HMENU>(ID_COMBOBOX), core_api::get_my_instance(), nullptr);

        m_initialised = true;

        if (m_wnd_combo) {
            SetWindowLongPtr(m_wnd_combo, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));

            SendMessage(m_wnd_combo, WM_SETFONT, reinterpret_cast<WPARAM>(s_icon_font), MAKELPARAM(1, 0));

            refresh_all_items();

            m_order_proc = reinterpret_cast<WNDPROC>(
                SetWindowLongPtr(m_wnd_combo, GWLP_WNDPROC, reinterpret_cast<LPARAM>(s_on_hook)));

            COMBOBOXINFO cbi;
            memset(&cbi, 0, sizeof(cbi));
            cbi.cbSize = sizeof(cbi);

            GetComboBoxInfo(m_wnd_combo, &cbi);

            RECT rc_client;
            GetClientRect(m_wnd_combo, &rc_client);

            m_max_item_width += RECT_CX(rc_client) - RECT_CX(cbi.rcItem);
            RECT rc;
            GetWindowRect(m_wnd_combo, &rc);
            m_height = rc.bottom - rc.top;
        }
        break;
    }
    case WM_DESTROY: {
        if (s_windows.size() == 1) {
            ToolbarArgs::on_last_window_destroyed();

            fb2k::std_api_get<cui::colours::manager>()->deregister_common_callback(&s_colour_callback);
        }
        s_windows.erase(std::remove(s_windows.begin(), s_windows.end(), this), s_windows.end());

        m_initialised = false;
        const unsigned count = get_class_data().refcount;
        DestroyWindow(m_wnd_combo);
        if (count == 1) {
            DeleteFont(s_icon_font);
            s_icon_font = nullptr;
            s_background_brush.reset();
        }
        break;
    }
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX: {
        const auto dc = reinterpret_cast<HDC>(wp);

        cui::colours::helper helper(GUID{});

        SetTextColor(dc, helper.get_colour(cui::colours::colour_text));
        SetBkColor(dc, helper.get_colour(cui::colours::colour_background));

        return reinterpret_cast<LRESULT>(s_background_brush.get());
    }
    case WM_WINDOWPOSCHANGED: {
        const auto lpwp = reinterpret_cast<LPWINDOWPOS>(lp);
        if (!(lpwp->flags & SWP_NOSIZE)) {
            SetWindowPos(m_wnd_combo, nullptr, 0, 0, lpwp->cx, uih::scale_dpi_value(300), SWP_NOZORDER);
        }
        break;
    }
    case WM_COMMAND:
        switch (wp) {
        case ID_COMBOBOX | CBN_SELCHANGE << 16: {
            pfc::vartoggle_t<bool> var_toggle(m_processing_selection_change, true);
            const int sel = ComboBox_GetCurSel(reinterpret_cast<HWND>(lp));
            if (sel >= 0 && sel < gsl::narrow<int>(m_items.size()))
                ToolbarArgs::set_active_item(std::get<typename ToolbarArgs::ID>(m_items[sel]));
            break;
        }
        case ID_COMBOBOX | CBN_DROPDOWN << 16: {
            if (ToolbarArgs::refresh_on_click)
                refresh_all_items();
            break;
        }
        }
        break;
    case WM_GETMINMAXINFO: {
        const auto mmi = LPMINMAXINFO(lp);
        mmi->ptMinTrackSize.x = (std::min)(m_max_item_width, uih::scale_dpi_value(maximum_minimum_width));
        mmi->ptMinTrackSize.y = m_height;
        mmi->ptMaxTrackSize.y = m_height;
        return 0;
    }
    }

    return DefWindowProc(wnd, msg, wp, lp);
}

template <class ToolbarArgs>
LRESULT DropDownListToolbar<ToolbarArgs>::on_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_GETDLGCODE:
        return DLGC_WANTALLKEYS;
    case WM_KEYDOWN: {
        const auto processed = get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp);
        m_process_next_char = !processed;
        if (processed)
            return 0;

        if (wp == VK_TAB)
            g_on_tab(wnd);
        SendMessage(wnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), NULL);
        break;
    }
    case WM_SYSKEYDOWN: {
        const auto processed = get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp);
        m_process_next_char = !processed;
        if (processed)
            return 0;
        break;
    }
    case WM_CHAR:
        if (!m_process_next_char) {
            m_process_next_char = true;
            return 0;
        }
        break;
    case WM_UPDATEUISTATE:
        RedrawWindow(wnd, nullptr, nullptr, RDW_INVALIDATE);
        break;
    case WM_MOUSEWHEEL: {
        const int index = ComboBox_GetCurSel(wnd);
        const int count = ComboBox_GetCount(wnd);
        int new_index = index;
        const int delta = GET_WHEEL_DELTA_WPARAM(wp);

        m_mousewheel_delta += delta;
        if (m_mousewheel_delta >= WHEEL_DELTA)
            new_index = index - 1;
        else if (m_mousewheel_delta <= -WHEEL_DELTA)
            new_index = index + 1;

        if (new_index != index) {
            m_mousewheel_delta = 0;

            if (new_index >= 0 && new_index < count) {
                ComboBox_SetCurSel(wnd, new_index);
                SendMessage(get_wnd(), WM_COMMAND, CBN_SELCHANGE << 16 | ID_COMBOBOX, reinterpret_cast<LPARAM>(wnd));
            }
        }

        return 0;
    }
    }
    return CallWindowProc(m_order_proc, wnd, msg, wp, lp);
}

template <class ToolbarArgs>
HFONT DropDownListToolbar<ToolbarArgs>::s_icon_font;

template <class ToolbarArgs>
typename DropDownListToolbar<ToolbarArgs>::ColourCallback DropDownListToolbar<ToolbarArgs>::s_colour_callback;

template <class ToolbarArgs>
wil::unique_hbrush DropDownListToolbar<ToolbarArgs>::s_background_brush;

template <class ToolbarArgs>
std::vector<DropDownListToolbar<ToolbarArgs>*> DropDownListToolbar<ToolbarArgs>::s_windows;
