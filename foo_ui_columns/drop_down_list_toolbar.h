#pragma once

template <class ToolbarArgs>
class DropDownListToolbar : public ui_extension::container_uie_window_v3 {
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

    static void s_set_window_theme()
    {
        for (auto&& window : s_windows) {
            window->set_window_theme();
        }
    }

    static void s_update_colours()
    {
        cui::colours::helper colour_helper(ToolbarArgs::colour_client_id);
        s_background_brush.reset(CreateSolidBrush(colour_helper.get_colour(cui::colours::colour_background)));

        for (auto&& window : s_windows) {
            const HWND wnd = window->m_wnd_combo;
            if (wnd)
                RedrawWindow(wnd, nullptr, nullptr, RDW_INVALIDATE);
        }
    }

    static void s_update_font()
    {
        const cui::fonts::helper font_helper(ToolbarArgs::font_client_id);
        s_items_font.reset(font_helper.get_font());

        for (auto&& window : s_windows) {
            const HWND wnd = window->m_wnd_combo;

            if (wnd) {
                SetWindowFont(wnd, s_items_font.get(), TRUE);
                window->get_host()->on_size_limit_change(window->get_wnd(),
                    uie::size_limit_minimum_height | uie::size_limit_maximum_height | uie::size_limit_minimum_width);
            }
        }
    }

    void refresh_all_items_safe();
    void update_active_item_safe();

    const GUID& get_extension_guid() const override { return ToolbarArgs::extension_guid; }
    unsigned get_type() const override { return ui_extension::type_toolbar; }
    void get_name(pfc::string_base& out) const override { out = ToolbarArgs::name; }
    void get_category(pfc::string_base& out) const override { out.set_string("Toolbars"); }
    bool is_available(const uie::window_host_ptr& p_host) const override { return ToolbarArgs::is_available(); }
    void get_menu_items(uie::menu_hook_t& p_hook) override { ToolbarArgs::get_menu_items(p_hook); }
    uie::container_window_v3_config get_window_config() override { return {ToolbarArgs::class_name}; }

private:
    class FontClient : public cui::fonts::client {
        const GUID& get_client_guid() const override { return ToolbarArgs::font_client_id; }
        void get_name(pfc::string_base& p_out) const override { p_out = ToolbarArgs::name; }
        cui::fonts::font_type_t get_default_font_type() const override { return cui::fonts::font_type_items; }
        void on_font_changed() const override { s_update_font(); }
    };

    class ColourClient : public cui::colours::client {
        const GUID& get_client_guid() const override { return ToolbarArgs::colour_client_id; }
        void get_name(pfc::string_base& p_out) const override { p_out = ToolbarArgs::name; }
        uint32_t get_supported_colours() const override
        {
            return cui::colours::colour_flag_text | cui::colours::colour_flag_background;
        }
        uint32_t get_supported_bools() const override { return cui::colours::bool_flag_dark_mode_enabled; }
        bool get_themes_supported() const override { return false; }
        void on_bool_changed(uint32_t mask) const override
        {
            if (mask & cui::colours::bool_flag_dark_mode_enabled)
                s_set_window_theme();
        }
        void on_colour_changed(uint32_t mask) const override { s_update_colours(); }
    };

    static LRESULT WINAPI s_on_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        auto p_this = reinterpret_cast<DropDownListToolbar*>(GetWindowLongPtr(wnd, GWLP_USERDATA));
        return p_this ? p_this->on_hook(wnd, msg, wp, lp) : DefWindowProc(wnd, msg, wp, lp);
    }

    LRESULT WINAPI on_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;

    void set_window_theme() const;
    void refresh_all_items();
    void update_active_item();
    int calculate_max_item_width();
    int calculate_height();

    static constexpr INT_PTR ID_COMBOBOX = 1001;
    static constexpr unsigned initial_height = 300;
    static constexpr unsigned initial_width = 150;
    static constexpr unsigned maximum_minimum_width = 150;
    inline static cui::fonts::client::factory<FontClient> s_font_client;
    inline static wil::unique_hfont s_items_font;
    inline static cui::colours::client::factory<ColourClient> s_colour_client;
    inline static wil::unique_hbrush s_background_brush;
    inline static std::vector<DropDownListToolbar<ToolbarArgs>*> s_windows;

    typename ToolbarArgs::ItemList m_items;
    HWND m_wnd_combo{nullptr};
    WNDPROC m_order_proc{nullptr};
    int m_max_item_width{0};
    bool m_initialised{false};
    bool m_process_next_char{true};
    bool m_processing_selection_change{false};
    int32_t m_mousewheel_delta{0};
};

template <class ToolbarArgs>
void DropDownListToolbar<ToolbarArgs>::set_window_theme() const
{
    SetWindowTheme(m_wnd_combo, cui::colours::is_dark_mode_active() ? L"DarkMode_CFD" : nullptr, nullptr);
}

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
    auto&& active_item_id = ToolbarArgs::get_active_item();
    m_items = ToolbarArgs::get_items();

    ComboBox_ResetContent(m_wnd_combo);
    ComboBox_Enable(m_wnd_combo, !ranges::empty(m_items));

    if (ranges::empty(m_items)) {
        if (!ToolbarArgs::no_items_text.empty()) {
            ComboBox_AddString(m_wnd_combo,
                pfc::stringcvt::string_wide_from_utf8(
                    ToolbarArgs::no_items_text.data(), ToolbarArgs::no_items_text.length()));
        }
    } else {
        for (auto&& [id, name] : m_items) {
            ComboBox_AddString(m_wnd_combo, pfc::stringcvt::string_wide_from_utf8(name.c_str()));
        }
    }

    update_active_item();

    const auto previous_max_item_width = m_max_item_width;
    m_max_item_width = calculate_max_item_width();

    if (m_initialised && m_max_item_width != previous_max_item_width) {
        get_host()->on_size_limit_change(get_wnd(), uie::size_limit_minimum_width | uie::size_limit_maximum_height);
    }
}

template <class ToolbarArgs>
int DropDownListToolbar<ToolbarArgs>::calculate_height()
{
    RECT rc{};
    GetWindowRect(m_wnd_combo, &rc);
    return rc.bottom - rc.top;
}

template <class ToolbarArgs>
int DropDownListToolbar<ToolbarArgs>::calculate_max_item_width()
{
    const int fallback_width = uih::scale_dpi_value(50);

    const auto dc = wil::GetDC(m_wnd_combo);
    const auto _ = wil::SelectObject(dc.get(), s_items_font.get());

    const auto item_count = ComboBox_GetCount(m_wnd_combo);

    if (item_count <= 0)
        return fallback_width;

    int max_item_width{};
    pfc::string8 text;
    for (auto index : ranges::views::iota(0, item_count)) {
        uComboBox_GetText(m_wnd_combo, index, text);
        const auto cx = uih::get_text_width(dc.get(), text, gsl::narrow<int>(text.get_length()));
        max_item_width = std::max(max_item_width, cx);
    }

    COMBOBOXINFO cbi{};
    cbi.cbSize = sizeof(cbi);
    GetComboBoxInfo(m_wnd_combo, &cbi);

    RECT rc_client{};
    GetClientRect(m_wnd_combo, &rc_client);

    const auto non_item_space = RECT_CX(rc_client) - RECT_CX(cbi.rcItem);
    return max_item_width + non_item_space;
}

template <class ToolbarArgs>
void DropDownListToolbar<ToolbarArgs>::update_active_item()
{
    if (ranges::empty(m_items) && !ToolbarArgs::no_items_text.empty()) {
        ComboBox_SetCurSel(m_wnd_combo, 0);
        return;
    }

    auto&& id = ToolbarArgs::get_active_item();
    const auto iter = std::find_if(
        m_items.begin(), m_items.end(), [id](auto&& item) { return std::get<typename ToolbarArgs::ID>(item) == id; });

    const ptrdiff_t sel_item_index = iter != m_items.end() ? iter - m_items.begin() : -1;
    ComboBox_SetCurSel(m_wnd_combo, gsl::narrow<int>(sel_item_index));
}

template <class ToolbarArgs>
LRESULT DropDownListToolbar<ToolbarArgs>::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        s_windows.emplace_back(this);
        if (s_windows.size() == 1) {
            ToolbarArgs::on_first_window_created();
        }

        if (!s_items_font)
            s_update_font();

        if (!s_background_brush)
            s_update_colours();

        m_wnd_combo = CreateWindowEx(0, WC_COMBOBOX, nullptr,
            CBS_DROPDOWNLIST | WS_CHILD | WS_TABSTOP | WS_VISIBLE | WS_VSCROLL, 0, 0,
            uih::scale_dpi_value(initial_width), uih::scale_dpi_value(initial_height), wnd,
            reinterpret_cast<HMENU>(ID_COMBOBOX), core_api::get_my_instance(), nullptr);

        if (m_wnd_combo) {
            SetWindowLongPtr(m_wnd_combo, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));

            set_window_theme();

            SetWindowFont(m_wnd_combo, s_items_font.get(), TRUE);

            refresh_all_items();

            m_order_proc = reinterpret_cast<WNDPROC>(
                SetWindowLongPtr(m_wnd_combo, GWLP_WNDPROC, reinterpret_cast<LPARAM>(s_on_hook)));
        }

        m_initialised = true;
        break;
    }
    case WM_DESTROY: {
        if (s_windows.size() == 1) {
            ToolbarArgs::on_last_window_destroyed();
        }
        std::erase(s_windows, this);
        m_initialised = false;
        break;
    }
    case WM_NCDESTROY:
        if (s_windows.empty()) {
            s_items_font.reset();
            s_background_brush.reset();
        }
        break;
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX: {
        const auto dc = reinterpret_cast<HDC>(wp);

        cui::colours::helper colour_helper(ToolbarArgs::colour_client_id);

        SetTextColor(dc, colour_helper.get_colour(cui::colours::colour_text));
        SetBkColor(dc, colour_helper.get_colour(cui::colours::colour_background));

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
            if constexpr (requires() { ToolbarArgs::on_click(); }) {
                ToolbarArgs::on_click();
            }

            if (ToolbarArgs::refresh_on_click)
                refresh_all_items();

            break;
        }
        }
        break;
    case WM_GETMINMAXINFO: {
        const auto mmi = reinterpret_cast<LPMINMAXINFO>(lp);
        const auto height = calculate_height();

        mmi->ptMinTrackSize.x = (std::min)(m_max_item_width, uih::scale_dpi_value(maximum_minimum_width));
        mmi->ptMinTrackSize.y = height;
        mmi->ptMaxTrackSize.y = height;
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
