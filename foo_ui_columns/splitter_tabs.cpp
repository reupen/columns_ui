#include "pch.h"
#include "splitter_tabs.h"

#include "dark_mode.h"
#include "dark_mode_tabs.h"

namespace cui::panels::tab_stack {

// {6F000FC4-3F86-4fc5-80EA-F7AA4D9551E6}
const GUID g_guid_splitter_tabs = {0x6f000fc4, 0x3f86, 0x4fc5, {0x80, 0xea, 0xf7, 0xaa, 0x4d, 0x95, 0x51, 0xe6}};

class TabStackPanel::TabStackSplitterHost : public ui_extension::window_host {
    service_ptr_t<TabStackPanel> m_this;

public:
    const GUID& get_host_guid() const override
    {
        // {B5C88724-EDCD-46a1-90B9-C298309FDFB7}
        static const GUID rv = {0xb5c88724, 0xedcd, 0x46a1, {0x90, 0xb9, 0xc2, 0x98, 0x30, 0x9f, 0xdf, 0xb7}};
        return rv;
    }

    bool get_keyboard_shortcuts_enabled() const override
    {
        return m_this->get_host()->get_keyboard_shortcuts_enabled();
    }

    void on_size_limit_change(HWND wnd, unsigned flags) override
    {
        const auto iter = m_this->find_active_panel_by_wnd(wnd);

        if (iter == m_this->m_active_panels.end())
            return;

        const auto p_ext = *iter;
        MINMAXINFO mmi{};
        mmi.ptMaxTrackSize.x = MAXLONG;
        mmi.ptMaxTrackSize.y = MAXLONG;
        SendMessage(wnd, WM_GETMINMAXINFO, 0, (LPARAM)&mmi);
        p_ext->m_size_limits.min_width = std::min(mmi.ptMinTrackSize.x, static_cast<long>(MAXSHORT));
        p_ext->m_size_limits.min_height = std::min(mmi.ptMinTrackSize.y, static_cast<long>(MAXSHORT));
        p_ext->m_size_limits.max_height = std::min(mmi.ptMaxTrackSize.y, static_cast<long>(MAXSHORT));
        p_ext->m_size_limits.max_width = std::min(mmi.ptMaxTrackSize.x, static_cast<long>(MAXSHORT));

        m_this->update_size_limits();

        m_this->get_host()->on_size_limit_change(m_this->get_wnd(), flags);

        m_this->on_size_changed();
    }

    unsigned is_resize_supported(HWND wnd) const override { return false; }

    bool request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height) override { return false; }

    bool override_status_text_create(service_ptr_t<ui_status_text_override>& p_out) override
    {
        return m_this->get_host()->override_status_text_create(p_out);
    }

    bool is_visible(HWND wnd) const override
    {
        bool rv = false;

        if (!m_this->get_host()->is_visible(m_this->get_wnd())) {
            rv = false;
        } else {
            rv = IsWindowVisible(wnd) != 0;
        }
        return rv;
    }

    bool is_visibility_modifiable(HWND wnd, bool desired_visibility) const override
    {
        if (!desired_visibility)
            return false;

        const auto iter = m_this->find_active_panel_by_wnd(wnd);

        if (iter == m_this->m_active_panels.end())
            return false;

        if (m_this->get_host()->is_visible(m_this->get_wnd()))
            return true;

        return m_this->get_host()->is_visibility_modifiable(m_this->get_wnd(), desired_visibility);
    }

    bool set_window_visibility(HWND wnd, bool visibility) override
    {
        bool self_is_visible = true;

        if (!m_this->get_host()->is_visible(m_this->get_wnd()))
            self_is_visible = m_this->get_host()->set_window_visibility(m_this->get_wnd(), visibility);

        if (!(self_is_visible && visibility))
            return false;

        const auto iter = m_this->find_active_panel_by_wnd(wnd);

        if (iter == m_this->m_active_panels.end())
            return false;

        TabCtrl_SetCurSel(m_this->m_wnd_tabs, std::distance(m_this->m_active_panels.begin(), iter));
        m_this->on_active_tab_changed(TabCtrl_GetCurSel(m_this->m_wnd_tabs));
        return true;
    }

    void set_window_ptr(TabStackPanel* p_ptr) { m_this = p_ptr; }

    void relinquish_ownership(HWND wnd) override
    {
        const auto iter = m_this->find_active_panel_by_wnd(wnd);

        if (iter == m_this->m_active_panels.end())
            return;

        const auto p_ext = *iter;

        p_ext->m_wnd = nullptr;
        p_ext->m_child.release();
        TabCtrl_DeleteItem(m_this->m_wnd_tabs, std::distance(m_this->m_active_panels.begin(), iter));
        m_this->m_active_panels.erase(iter);
        std::erase(m_this->m_panels, p_ext);
        m_this->update_size_limits();
        m_this->get_host()->on_size_limit_change(m_this->get_wnd(), uie::size_limit_all);

        m_this->on_size_changed();
    }
};

ui_extension::window_host_factory<TabStackPanel::TabStackSplitterHost> g_splitter_tabs_host;

void TabStackPanel::get_supported_panels(
    const pfc::list_base_const_t<window::ptr>& p_windows, bit_array_var& p_mask_unsupported)
{
    service_ptr_t<service_base> temp;
    g_splitter_tabs_host.instance_create(temp);
    uie::window_host_ptr ptr;
    if (temp->service_query_t(ptr))
        (static_cast<TabStackSplitterHost*>(ptr.get_ptr()))->set_window_ptr(this);
    size_t count = p_windows.get_count();
    for (size_t i = 0; i < count; i++)
        p_mask_unsupported.set(i, !p_windows[i]->is_available(ptr));
}

uie::splitter_item_full_v2_t* TabStackPanel::Panel::create_splitter_item()
{
    auto ret = new uie::splitter_item_full_v2_impl_t;
    ret->set_panel_guid(m_guid);
    ret->set_panel_config_from_ptr(m_child_data.get_ptr(), m_child_data.get_size());
    ret->set_window_ptr(m_child);
    ret->m_custom_title = m_use_custom_title;
    ret->set_title(m_custom_title, m_custom_title.length());

    ret->m_autohide = false;
    ret->m_caption_orientation = 0;
    ret->m_locked = false;
    ret->m_hidden = false;
    ret->m_show_toggle_area = false;
    ret->m_size = 0;
    ret->m_show_caption = true;
    ret->m_size_v2 = 0;
    ret->m_size_v2_dpi = USER_DEFAULT_SCREEN_DPI;
    return ret;
}

void TabStackPanel::Panel::set_from_splitter_item(const uie::splitter_item_t* p_source)
{
    if (m_wnd)
        destroy();
    const uie::splitter_item_full_t* ptr = nullptr;
    if (p_source->query(ptr)) {
        m_use_custom_title = ptr->m_custom_title;
        ptr->get_title(m_custom_title);
    }
    m_child = p_source->get_window_ptr();
    m_guid = p_source->get_panel_guid();
    p_source->get_panel_config_to_array(m_child_data, true);
}

void TabStackPanel::Panel::destroy()
{
    if (m_child.is_valid()) {
        m_child->destroy_window();
        m_wnd = nullptr;
        m_child.release();
    }
    m_this.release();
    m_interface.release();
}

void TabStackPanel::Panel::refresh_child_data(abort_callback& p_abort)
{
    if (m_child.is_valid())
        m_child_data = m_child->get_config_as_array(p_abort);
}

void TabStackPanel::Panel::read(stream_reader* t, abort_callback& p_abort)
{
    t->read_lendian_t(m_guid, p_abort);
    unsigned size;
    t->read_lendian_t(size, p_abort);
    m_child_data.set_size(size);
    t->read(m_child_data.get_ptr(), size, p_abort);
    t->read_lendian_t(m_use_custom_title, p_abort);
    t->read_string(m_custom_title, p_abort);
}

void TabStackPanel::Panel::write(stream_writer* out, abort_callback& p_abort)
{
    refresh_child_data(p_abort);
    out->write_lendian_t(m_guid, p_abort);
    out->write_lendian_t(gsl::narrow<uint32_t>(m_child_data.get_size()), p_abort);
    out->write(m_child_data.get_ptr(), m_child_data.get_size(), p_abort);
    out->write_lendian_t(m_use_custom_title, p_abort);
    out->write_string(m_custom_title, p_abort);
}
void TabStackPanel::Panel::_export(stream_writer* out, abort_callback& p_abort)
{
    stream_writer_memblock child_exported_data;
    uie::window_ptr ptr = m_child;
    if (!ptr.is_valid()) {
        if (!create_by_guid(m_guid, ptr))
            throw fcl::exception_missing_panel();
        try {
            ptr->set_config_from_ptr(m_child_data.get_ptr(), m_child_data.get_size(), p_abort);
        } catch (const exception_io&) {
        }
    }
    {
        ptr->export_config(&child_exported_data, p_abort);
    }
    out->write_lendian_t(m_guid, p_abort);
    out->write_lendian_t(gsl::narrow<uint32_t>(child_exported_data.m_data.get_size()), p_abort);
    out->write(child_exported_data.m_data.get_ptr(), child_exported_data.m_data.get_size(), p_abort);
    out->write_lendian_t(m_use_custom_title, p_abort);
    out->write_string(m_custom_title, p_abort);
}
void TabStackPanel::Panel::import(stream_reader* t, abort_callback& p_abort)
{
    t->read_lendian_t(m_guid, p_abort);
    unsigned size;
    t->read_lendian_t(size, p_abort);
    pfc::array_t<uint8_t> data;
    data.set_size(size);
    t->read(data.get_ptr(), size, p_abort);
    t->read_lendian_t(m_use_custom_title, p_abort);
    t->read_string(m_custom_title, p_abort);

    if (create_by_guid(m_guid, m_child)) {
        try {
            m_child->import_config_from_ptr(data.get_ptr(), data.get_size(), p_abort);
        } catch (const exception_io&) {
        }
        m_child_data.set_size(0);
        m_child->get_config_to_array(m_child_data, p_abort);
    }
    // else
    //    throw pfc::exception_not_implemented();
}

void TabStackPanel::get_name(pfc::string_base& p_out) const
{
    p_out = "Tab stack";
}
const GUID& TabStackPanel::get_extension_guid() const
{
    // {5CB67C98-B77F-4926-A79F-49D9B21B9705}
    static const GUID rv = {0x5cb67c98, 0xb77f, 0x4926, {0xa7, 0x9f, 0x49, 0xd9, 0xb2, 0x1b, 0x97, 0x5}};
    return rv;
}
void TabStackPanel::get_category(pfc::string_base& p_out) const
{
    p_out = "Splitters";
}
unsigned TabStackPanel::get_type() const
{
    return ui_extension::type_layout | uie::type_splitter;
}

size_t TabStackPanel::get_panel_count() const
{
    return m_panels.size();
}
uie::splitter_item_t* TabStackPanel::get_panel(size_t index) const
{
    if (index < m_panels.size()) {
        return m_panels[index]->create_splitter_item();
    }
    return nullptr;
}

bool TabStackPanel::get_config_item_supported(size_t index, const GUID& p_type) const
{
    return p_type == bool_use_custom_title || p_type == string_custom_title;
}

bool TabStackPanel::get_config_item(
    size_t index, const GUID& p_type, stream_writer* p_out, abort_callback& p_abort) const
{
    if (index < m_panels.size()) {
        if (p_type == bool_use_custom_title) {
            p_out->write_lendian_t(m_panels[index]->m_use_custom_title, p_abort);
            return true;
        }
        if (p_type == string_custom_title) {
            p_out->write_string(m_panels[index]->m_custom_title, p_abort);
            return true;
        }
    }
    return false;
}

bool TabStackPanel::set_config_item(size_t index, const GUID& p_type, stream_reader* p_source, abort_callback& p_abort)
{
    if (index < m_panels.size()) {
        if (p_type == bool_use_custom_title) {
            p_source->read_lendian_t(m_panels[index]->m_use_custom_title, p_abort);
            return true;
        }
        if (p_type == string_custom_title) {
            p_source->read_string(m_panels[index]->m_custom_title, p_abort);
            return true;
        }
    }
    return false;
}

void TabStackPanel::set_config(stream_reader* config, size_t p_size, abort_callback& p_abort)
{
    if (p_size) {
        uint32_t version;
        config->read_lendian_t(version, p_abort);
        if (version <= stream_version_current) {
            m_panels.clear();

            const auto raw_active_tab_index = config->read_lendian_t<uint32_t>(p_abort);

            m_active_tab = raw_active_tab_index == std::numeric_limits<uint32_t>::max()
                ? std::nullopt
                : std::make_optional(size_t{raw_active_tab_index});

            unsigned count;
            config->read_lendian_t(count, p_abort);

            for (unsigned n = 0; n < count; n++) {
                auto temp = std::make_shared<Panel>();
                temp->read(config, p_abort);
                m_panels.emplace_back(std::move(temp));
            }
        }
    }
}
void TabStackPanel::get_config(stream_writer* out, abort_callback& p_abort) const
{
    out->write_lendian_t((uint32_t)stream_version_current, p_abort);
    const auto count = m_panels.size();
    const auto raw_active_tab_index = m_active_tab.value_or(std::numeric_limits<uint32_t>::max());
    out->write_lendian_t(gsl::narrow<uint32_t>(raw_active_tab_index), p_abort);
    out->write_lendian_t(gsl::narrow<uint32_t>(count), p_abort);
    for (size_t n = 0; n < count; n++) {
        m_panels[n]->write(out, p_abort);
    }
}

void TabStackPanel::export_config(stream_writer* p_writer, abort_callback& p_abort) const
{
    p_writer->write_lendian_t((uint32_t)stream_version_current, p_abort);
    const auto count = m_panels.size();
    const auto raw_active_tab_index = m_active_tab.value_or(std::numeric_limits<uint32_t>::max());
    p_writer->write_lendian_t(gsl::narrow<uint32_t>(raw_active_tab_index), p_abort);
    p_writer->write_lendian_t(gsl::narrow<uint32_t>(count), p_abort);
    for (size_t n = 0; n < count; n++) {
        m_panels[n]->_export(p_writer, p_abort);
    }
}

void TabStackPanel::import_config(stream_reader* p_reader, size_t p_size, abort_callback& p_abort)
{
    uint32_t version;
    p_reader->read_lendian_t(version, p_abort);
    if (version <= stream_version_current) {
        m_panels.clear();

        const auto raw_active_tab_index = p_reader->read_lendian_t<uint32_t>(p_abort);

        m_active_tab = raw_active_tab_index == std::numeric_limits<uint32_t>::max()
            ? std::nullopt
            : std::make_optional(size_t{raw_active_tab_index});

        const auto count = p_reader->read_lendian_t<uint32_t>(p_abort);

        for (unsigned n = 0; n < count; n++) {
            auto temp = std::make_shared<Panel>();
            temp->import(p_reader, p_abort);
            m_panels.emplace_back(std::move(temp));
        }
    }
}

void clip_sizelimit(uie::size_limit_t& mmi)
{
    mmi.max_height = std::min(mmi.max_height, static_cast<unsigned>(MAXSHORT));
    mmi.min_height = std::min(mmi.min_height, static_cast<unsigned>(MAXSHORT));
    mmi.max_width = std::min(mmi.max_width, static_cast<unsigned>(MAXSHORT));
    mmi.min_width = std::min(mmi.min_width, static_cast<unsigned>(MAXSHORT));
}

void TabStackPanel::update_size_limits()
{
    m_size_limits = uie::size_limit_t();

    const auto count = m_active_panels.size();

    for (size_t n = 0; n < count; n++) {
        MINMAXINFO mmi{};
        mmi.ptMaxTrackSize.x = MAXLONG;
        mmi.ptMaxTrackSize.y = MAXLONG;

        if (m_active_panels[n]->m_wnd) {
            SendMessage(m_active_panels[n]->m_wnd, WM_GETMINMAXINFO, 0, (LPARAM)&mmi);

            m_size_limits.min_height = std::max((unsigned)mmi.ptMinTrackSize.y, m_size_limits.min_height);
            m_size_limits.min_width = std::max((unsigned)mmi.ptMinTrackSize.x, m_size_limits.min_width);
            m_size_limits.max_height = std::min((unsigned)mmi.ptMaxTrackSize.y, m_size_limits.max_height);
            m_size_limits.max_width = std::min((unsigned)mmi.ptMaxTrackSize.x, m_size_limits.max_width);
        }
        if (m_size_limits.max_width < m_size_limits.min_width)
            m_size_limits.max_width = m_size_limits.min_width;
        if (m_size_limits.max_height < m_size_limits.min_height)
            m_size_limits.max_height = m_size_limits.min_height;
    }
    clip_sizelimit(m_size_limits);
    RECT rcmin = {0, 0, (LONG)m_size_limits.min_width, (LONG)m_size_limits.min_height};
    RECT rcmax = {0, 0, (LONG)m_size_limits.max_width, (LONG)m_size_limits.max_height};
    adjust_rect(TRUE, &rcmin);
    adjust_rect(TRUE, &rcmax);
    m_size_limits.min_width = wil::rect_width(rcmin);
    m_size_limits.max_width = wil::rect_width(rcmax);
    m_size_limits.min_height = wil::rect_height(rcmin);
    m_size_limits.max_height = wil::rect_height(rcmax);
}

void TabStackPanel::adjust_rect(bool b_larger, RECT* rc)
{
    // TabCtrl_AdjustRect(m_wnd_tabs, b_larger, rc);

    if (b_larger) {
        RECT rc_child = *rc;
        TabCtrl_AdjustRect(m_wnd_tabs, FALSE, &rc_child);
        rc_child.top = rc->top + 2;
        TabCtrl_AdjustRect(m_wnd_tabs, TRUE, &rc_child);
        *rc = rc_child;
    } else {
        RECT rc_tabs = *rc;
        TabCtrl_AdjustRect(m_wnd_tabs, FALSE, &rc_tabs);
        rc->top = rc_tabs.top - 2;
    }
}

void TabStackPanel::set_styles(bool visible)
{
    if (m_wnd_tabs) {
        long flags = WS_CHILD | TCS_HOTTRACK | TCS_TABS | (false ? TCS_MULTILINE | TCS_RIGHTJUSTIFY : TCS_SINGLELINE)
            | (visible ? WS_VISIBLE : 0) | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP;

        if (GetWindowLongPtr(m_wnd_tabs, GWL_STYLE) != flags)
            SetWindowLongPtr(m_wnd_tabs, GWL_STYLE, flags);
    }
}

void TabStackPanel::set_up_down_window_theme() const
{
    if (!m_up_down_control_wnd)
        return;

    const auto is_dark = colours::is_dark_mode_active();

    if (is_dark)
        dark::spin::add_window(m_up_down_control_wnd);
    else
        dark::spin::remove_window(m_up_down_control_wnd);

    SetWindowTheme(m_up_down_control_wnd, is_dark ? L"DarkMode_Explorer" : nullptr, nullptr);
}

LRESULT TabStackPanel::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        create_tabs();
        SetWindowPos(m_wnd_tabs, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        refresh_children();

        const size_t panel_index = m_active_tab.value_or(0);
        std::optional<size_t> active_panel_index;

        if (panel_index < m_panels.size()) {
            const auto iter = std::ranges::find(m_active_panels, m_panels[panel_index]);

            if (iter != m_active_panels.end())
                active_panel_index = std::distance(m_active_panels.begin(), iter);
        }

        if (!active_panel_index && !m_active_panels.empty())
            active_panel_index = 0;

        if (active_panel_index)
            TabCtrl_SetCurSel(m_wnd_tabs, *active_panel_index);
        set_styles();

        // on_active_tab_changed(activeindex);
        m_active_tab.reset();

        if (active_panel_index) {
            const auto iter = std::ranges::find(m_panels, m_active_panels[*active_panel_index]);
            m_active_tab = std::distance(m_panels.begin(), iter);
        }

        update_size_limits();
        on_size_changed();
        // ShowWindow(m_wnd_tabs, SW_SHOWNORMAL);
        g_windows.emplace_back(this);
        m_dark_mode_notifier
            = std::make_unique<colours::dark_mode_notifier>([this, self = ptr{this}, wnd, wnd_tabs = m_wnd_tabs] {
                  set_up_down_window_theme();
                  RedrawWindow(wnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE);
                  RedrawWindow(wnd_tabs, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE);
              });

        m_get_message_hook_token = uih::register_message_hook(
            uih::MessageHookType::type_get_message, [this, wnd](int code, WPARAM wp, LPARAM lp) -> bool {
                helpers::handle_tabs_ctrl_tab(reinterpret_cast<LPMSG>(lp), wnd, m_wnd_tabs);
                return false;
            });
        break;
    }
    case WM_KEYDOWN: {
        if (wp != VK_LEFT && wp != VK_RIGHT && get_host()->get_keyboard_shortcuts_enabled()
            && g_process_keydown_keyboard_shortcuts(wp))
            return 0;
        if (wp == VK_TAB) {
            g_on_tab(wnd);
            return 0;
        }
        SendMessage(wnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), NULL);
    } break;
    case WM_SYSKEYDOWN:
        if (get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp))
            return 0;
        break;
    case WM_DESTROY:
        m_get_message_hook_token.reset();
        m_dark_mode_notifier.reset();
        std::erase(g_windows, this);
        destroy_children();
        destroy_tabs();
        break;
    case WM_WINDOWPOSCHANGED: {
        auto lpwp = (LPWINDOWPOS)lp;
        if (!(lpwp->flags & SWP_NOSIZE)) {
            on_size_changed(lpwp->cx, lpwp->cy);
        }
    } break;
    /*case WM_SIZE:
        on_size_changed(LOWORD(lp), HIWORD(lp));
        break;*/
    case WM_GETMINMAXINFO: {
        auto lpmmi = (LPMINMAXINFO)lp;

        lpmmi->ptMinTrackSize.y = m_size_limits.min_height;
        lpmmi->ptMinTrackSize.x = m_size_limits.min_width;
        lpmmi->ptMaxTrackSize.y = m_size_limits.max_height;
        lpmmi->ptMaxTrackSize.x = m_size_limits.max_width;

        /*console::formatter() << "min height: " << m_size_limits.min_height
            << " max height: " << m_size_limits.max_height
            << " min width: " << m_size_limits.min_width
            << " max width: " << m_size_limits.max_width;*/
    }
        return 0;
    case WM_SHOWWINDOW: {
        if (wp == TRUE && lp == NULL && m_active_tab && *m_active_tab < m_panels.size()
            && m_panels[*m_active_tab]->m_wnd && !IsWindowVisible(m_panels[*m_active_tab]->m_wnd)) {
            show_tab_window(m_panels[*m_active_tab]->m_wnd);
        }
    } break;
    case WM_CONTEXTMENU: {
        enum {
            IDM_BASE = 1
        };

        POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        if (pt.x == -1 && pt.y == -1)
            GetMessagePos(&pt);

        POINT pt_client = pt;
        POINT pt_client_tabs = pt;

        ScreenToClient(wnd, &pt_client);

        HMENU menu = CreatePopupMenu();

        unsigned IDM_EXT_BASE = IDM_BASE;

        HWND child = ChildWindowFromPointEx(wnd, pt_client, CWP_SKIPTRANSPARENT | CWP_SKIPINVISIBLE);
        Panel::Ptr click_panel;

        if (HWND(wp) == m_wnd_tabs) {
            ScreenToClient(m_wnd_tabs, &pt_client_tabs);
            TCHITTESTINFO info;
            // memset(&info, 0, sizeof(TCHITTESTINFO));
            info.pt = pt_client_tabs;
            const auto index = TabCtrl_HitTest(m_wnd_tabs, &info);
            if ((info.flags == TCHT_ONITEM || info.flags == TCHT_ONITEMLABEL || info.flags == TCHT_ONITEMICON)
                && index >= 0 && gsl::narrow_cast<size_t>(index) < m_active_panels.size()) {
                click_panel = m_active_panels[index];
            }
        } else if (child != nullptr) {
            if (const auto iter = find_active_panel_by_wnd(child); iter != m_active_panels.end())
                click_panel = *iter;
        }

        if (!click_panel)
            return 0;

        pfc::refcounted_object_ptr_t<ui_extension::menu_hook_impl> extension_menu_nodes
            = new ui_extension::menu_hook_impl;

        if (click_panel->m_child.is_valid()) {
            click_panel->m_child->get_menu_items(*extension_menu_nodes.get_ptr());
            extension_menu_nodes->win32_build_menu(menu, IDM_EXT_BASE, pfc_infinite - IDM_EXT_BASE);
        }
        menu_helpers::win32_auto_mnemonics(menu);

        unsigned cmd
            = TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, wnd, nullptr);

        if (cmd >= IDM_EXT_BASE) {
            extension_menu_nodes->execute_by_id(cmd);
        }

        DestroyMenu(menu);
        return 0;
    }
    case WM_NOTIFY: {
        switch (((LPNMHDR)lp)->idFrom) {
        case 2345:
            switch (((LPNMHDR)lp)->code) {
            case TCN_SELCHANGE:
                on_active_tab_changed(TabCtrl_GetCurSel(m_wnd_tabs), true);
                break;
            }
            break;
        }
    } break;
    }
    return DefWindowProc(wnd, msg, wp, lp);
}

void TabStackPanel::show_tab_window(HWND wnd)
{
    assert(!m_active_child_wnd || wnd == m_active_child_wnd);
    m_active_child_wnd = wnd;
    ShowWindow(wnd, SW_SHOWNORMAL);
}

void TabStackPanel::hide_tab_window()
{
    if (m_active_child_wnd) {
        ShowWindow(m_active_child_wnd, SW_HIDE);
        m_active_child_wnd = nullptr;
    }
}

void TabStackPanel::refresh_children()
{
    const auto count = m_panels.size();
    for (size_t n = 0; n < count; n++) {
        if (!m_panels[n]->m_wnd) {
            m_panels[n]->set_splitter_window_ptr(this);
            uie::window_ptr p_ext = m_panels[n]->m_child;

            bool b_new = false;
            if (!p_ext.is_valid()) {
                create_by_guid(m_panels[n]->m_guid, p_ext);
                b_new = true;
            }

            if (!m_panels[n]->m_interface.is_valid()) {
                service_ptr_t<service_base> temp;
                g_splitter_tabs_host.instance_create(temp);
                uie::window_host_ptr ptr;
                if (temp->service_query_t(ptr)) {
                    m_panels[n]->m_interface = static_cast<TabStackSplitterHost*>(ptr.get_ptr());
                    m_panels[n]->m_interface->set_window_ptr(this);
                }
            }

            if (p_ext.is_valid()
                && p_ext->is_available(
                    uie::window_host_ptr(static_cast<uie::window_host*>(m_panels[n]->m_interface.get_ptr())))) {
                pfc::string8 name;
                if (m_panels[n]->m_use_custom_title) {
                    name = m_panels[n]->m_custom_title;
                } else {
                    if (!p_ext->get_short_name(name))
                        p_ext->get_name(name);
                }

                {
                    if (b_new) {
                        try {
                            abort_callback_dummy abortCallback;
                            p_ext->set_config_from_ptr(m_panels[n]->m_child_data.get_ptr(),
                                m_panels[n]->m_child_data.get_size(), abortCallback);
                        } catch (const exception_io& e) {
                            console::formatter formatter;
                            formatter << "Error setting panel config: " << e.what();
                        }
                    }

                    const auto iter = std::ranges::find(m_active_panels, m_panels[n]);

                    size_t index = std::distance(m_active_panels.begin(), iter);
                    const auto is_new_tab = iter == m_active_panels.end();

                    if (is_new_tab)
                        m_active_panels.emplace_back(m_panels[n]);

                    HWND wnd_panel = p_ext->create_or_transfer_window(
                        get_wnd(), uie::window_host_ptr(m_panels[n]->m_interface.get_ptr()));
                    if (wnd_panel) {
                        if (GetWindowLongPtr(wnd_panel, GWL_STYLE) & WS_VISIBLE) {
                            pfc::string8 name;
                            p_ext->get_name(name);
                            console::formatter formatter;
                            formatter << "Columns UI/Tab stack: Warning: " << name
                                      << " panel was visible on creation! This usually indicates a bug in this panel.";
                            ShowWindow(wnd_panel, SW_HIDE);
                        }
                        SetWindowLongPtr(
                            wnd_panel, GWL_STYLE, GetWindowLongPtr(wnd_panel, GWL_STYLE) | WS_CLIPSIBLINGS);
                        SetWindowPos(wnd_panel, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                        // uxtheme_api_ptr p_uxtheme;
                        // if (p_uxtheme.load())
                        //    p_uxtheme->EnableThemeDialogTexture(wnd_panel, ETDT_ENABLETAB);

                        uTabCtrl_InsertItemText(m_wnd_tabs, gsl::narrow<int>(index), name, is_new_tab);

                        MINMAXINFO mmi{};
                        mmi.ptMaxTrackSize.x = MAXLONG;
                        mmi.ptMaxTrackSize.y = MAXLONG;
                        SendMessage(wnd_panel, WM_GETMINMAXINFO, 0, (LPARAM)&mmi);
                        helpers::clip_minmaxinfo(mmi);

                        m_panels[n]->m_wnd = wnd_panel;
                        m_panels[n]->m_child = p_ext;
                        m_panels[n]->m_size_limits.min_height = mmi.ptMinTrackSize.y;
                        m_panels[n]->m_size_limits.min_width = mmi.ptMinTrackSize.x;
                        m_panels[n]->m_size_limits.max_width = mmi.ptMaxTrackSize.x;
                        m_panels[n]->m_size_limits.max_height = mmi.ptMaxTrackSize.y;
                    } else
                        m_active_panels.erase(m_active_panels.begin() + index);
                }
            }
        }
    }
    update_size_limits();
    on_size_changed();

    if (!m_active_tab) {
        const auto tab_sel_index = TabCtrl_GetCurSel(m_wnd_tabs);
        m_active_tab = tab_sel_index == -1 ? std::nullopt : std::make_optional(gsl::narrow<size_t>(tab_sel_index));
    }

    if (IsWindowVisible(get_wnd()) && m_active_tab && *m_active_tab < m_panels.size() && m_panels[*m_active_tab]->m_wnd
        && !IsWindowVisible(m_panels[*m_active_tab]->m_wnd)) {
        get_host()->on_size_limit_change(get_wnd(), uie::size_limit_all);
        show_tab_window(m_panels[*m_active_tab]->m_wnd);
    }
}

void TabStackPanel::destroy_children()
{
    const auto count = m_panels.size();
    for (size_t n = 0; n < count; n++) {
        std::shared_ptr<Panel> pal = m_panels[n];
        pal->destroy();
    }
    m_active_panels.clear();
    m_active_child_wnd = nullptr;
}

void TabStackPanel::insert_panel(size_t index, const uie::splitter_item_t* p_item)
{
    if (index > m_panels.size())
        return;

    auto temp = std::make_shared<Panel>();
    temp->set_from_splitter_item(p_item);
    m_panels.insert(m_panels.begin() + index, std::move(temp));

    if (get_wnd())
        refresh_children();
}

void TabStackPanel::replace_panel(size_t index, const uie::splitter_item_t* p_item)
{
    if (index >= m_panels.size())
        return;

    if (get_wnd()) {
        if (m_active_child_wnd == m_panels[index]->m_wnd)
            m_active_child_wnd = nullptr;

        m_panels[index]->destroy();
    }

    const auto iter = std::ranges::find(m_active_panels, m_panels[index]);
    if (iter != m_active_panels.end()) {
        TabCtrl_DeleteItem(m_wnd_tabs, std::distance(m_active_panels.begin(), iter));
        m_active_panels.erase(iter);
    }

    auto temp = std::make_shared<Panel>();
    temp->set_from_splitter_item(p_item);
    m_panels[index] = std::move(temp);

    if (get_wnd())
        refresh_children();
}

void TabStackPanel::remove_panel(size_t index)
{
    if (index >= m_panels.size())
        return;

    const auto iter = std::ranges::find(m_active_panels, m_panels[index]);

    if (iter != m_active_panels.end()) {
        TabCtrl_DeleteItem(m_wnd_tabs, std::distance(m_active_panels.begin(), iter));
        m_active_panels.erase(iter);
    }

    if (m_active_child_wnd == m_panels[index]->m_wnd)
        m_active_child_wnd = nullptr;

    m_panels[index]->destroy();
    m_panels.erase(m_panels.begin() + index);
}

void TabStackPanel::create_tabs()
{
    g_font.reset(fb2k::std_api_get<fonts::manager>()->get_font(g_guid_splitter_tabs));
    RECT rc;
    GetClientRect(get_wnd(), &rc);
    DWORD flags = WS_CHILD | WS_TABSTOP | TCS_HOTTRACK | TCS_TABS | TCS_MULTILINE | (true ? WS_VISIBLE : 0)
        | WS_CLIPCHILDREN; // TCS_MULTILINE hack to prevent BS.
    m_wnd_tabs = CreateWindowEx(0, WC_TABCONTROL, _T("Tab stack"), flags, 0, 0, rc.right, rc.bottom, get_wnd(),
        HMENU(2345), core_api::get_my_instance(), nullptr);
    SetWindowLongPtr(m_wnd_tabs, GWLP_USERDATA, (LPARAM)(this));
    m_tab_proc = (WNDPROC)SetWindowLongPtr(m_wnd_tabs, GWLP_WNDPROC, (LPARAM)g_hook_proc);
    // SetWindowTheme(m_wnd_tabs, L"BrowserTab", NULL);
    SendMessage(m_wnd_tabs, WM_SETFONT, (WPARAM)g_font.get(), MAKELPARAM(0, 0));
}
void TabStackPanel::destroy_tabs()
{
    DestroyWindow(m_wnd_tabs);
    m_wnd_tabs = nullptr;
    g_font.reset();
}

uie::window_factory<TabStackPanel> g_splitter_window_tabs;
std::vector<service_ptr_t<TabStackPanel::t_self>> TabStackPanel::g_windows;

void TabStackPanel::g_on_font_change()
{
    for (auto& window : g_windows) {
        window->on_font_change();
    }
}

void TabStackPanel::on_font_change()
{
    if (m_wnd_tabs) {
        if (g_font) {
            SendMessage(m_wnd_tabs, WM_SETFONT, (WPARAM)0, MAKELPARAM(0, 0));
        }

        g_font.reset(fb2k::std_api_get<fonts::manager>()->get_font(g_guid_splitter_tabs));

        if (m_wnd_tabs) {
            SendMessage(m_wnd_tabs, WM_SETFONT, (WPARAM)g_font.get(), MAKELPARAM(1, 0));
            update_size_limits();
            get_host()->on_size_limit_change(get_wnd(), uie::size_limit_all);
            on_size_changed();
        }
    }
}
void TabStackPanel::on_size_changed(unsigned width, unsigned height)
{
    HDWP dwp = BeginDeferWindowPos(gsl::narrow<int>(m_active_panels.size() + 1));
    if (m_wnd_tabs)
        dwp = DeferWindowPos(dwp, m_wnd_tabs, nullptr, 0, 0, width, height, SWP_NOZORDER);
    // SetWindowPos(m_wnd_tabs, NULL, 0, 0, width, height, SWP_NOZORDER);

    size_t count = m_active_panels.size();
    RECT rc = {0, 0, (LONG)width, (LONG)height};
    adjust_rect(FALSE, &rc);
    for (size_t i = 0; i < count; i++) {
        if (m_active_panels[i]->m_wnd)
            dwp = DeferWindowPos(dwp, m_active_panels[i]->m_wnd, nullptr, rc.left, rc.top, rc.right - rc.left,
                rc.bottom - rc.top, SWP_NOZORDER);
        // SetWindowPos(m_active_panels[i]->m_wnd, NULL, rc.left, rc.top, RECT_CX(rc), RECT_CY(rc), SWP_NOZORDER);
    }
    EndDeferWindowPos(dwp);
}
void TabStackPanel::on_size_changed()
{
    RECT rc;
    GetClientRect(get_wnd(), &rc);
    on_size_changed(wil::rect_width(rc), wil::rect_height(rc));
}

void TabStackPanel::on_active_tab_changed(int signed_index_to, bool from_interaction)
{
    const auto wnd_focus = GetFocus();
    const auto was_child_focused
        = wnd_focus && (wnd_focus == m_active_child_wnd || IsChild(m_active_child_wnd, wnd_focus));

    hide_tab_window();
    m_active_tab.reset();

    if (signed_index_to < 0)
        return;

    const auto index_to = gsl::narrow<size_t>(signed_index_to);

    if (index_to < m_active_panels.size() && m_active_panels[index_to]->m_wnd) {
        show_tab_window(m_active_panels[index_to]->m_wnd);

        const auto iter = std::ranges::find(m_active_panels, m_panels[index_to]);

        if (iter != m_active_panels.end())
            m_active_tab = std::distance(m_active_panels.begin(), iter);
    }

    if (!m_active_tab || wnd_focus == m_wnd_tabs || (!from_interaction && !was_child_focused))
        return;

    const HWND wnd_root = GetAncestor(m_wnd_tabs, GA_ROOT);

    if (!wnd_root)
        return;

    const auto wnd_child = m_panels[*m_active_tab]->m_wnd;

    HWND wnd_new_focus = wnd_child;

    if (!(GetWindowLongPtr(wnd_new_focus, GWL_STYLE) & WS_TABSTOP))
        wnd_new_focus = GetNextDlgTabItem(wnd_new_focus, wnd_new_focus, FALSE);

    const auto should_focus = [=] {
        if (!wnd_new_focus)
            return false;

        if (wnd_child != wnd_new_focus && !IsChild(wnd_child, wnd_new_focus))
            return false;

        return (GetWindowLongPtr(wnd_new_focus, GWL_STYLE) & WS_TABSTOP) != 0;
    }();

    if (should_focus)
        SetFocus(wnd_new_focus);
    else
        SetFocus(m_wnd_tabs);
}

std::vector<TabStackPanel::Panel::Ptr>::iterator TabStackPanel::find_active_panel_by_wnd(HWND wnd)
{
    return std::ranges::find_if(m_active_panels, [wnd](const Panel::Ptr& panel) { return panel->m_wnd == wnd; });
}

LRESULT WINAPI TabStackPanel::g_hook_proc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) noexcept
{
    auto p_this = reinterpret_cast<TabStackPanel*>(GetWindowLongPtr(wnd, GWLP_USERDATA));
    return p_this ? p_this->on_hooked_message(wnd, msg, wp, lp) : DefWindowProc(wnd, msg, wp, lp);
}

LRESULT WINAPI TabStackPanel::on_hooked_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_PARENTNOTIFY:
        switch (LOWORD(wp)) {
        case WM_CREATE: {
            const auto child_window = reinterpret_cast<HWND>(lp);
            std::array<wchar_t, 128> class_name{};
            GetClassName(child_window, class_name.data(), gsl::narrow<int>(class_name.size()));

            if (!wcsncmp(UPDOWN_CLASSW, class_name.data(), class_name.size())) {
                m_up_down_control_wnd = child_window;
                uih::subclass_window_and_paint_with_buffering(child_window);
                set_up_down_window_theme();
            }
            break;
        }
        case WM_DESTROY:
            if (m_up_down_control_wnd == reinterpret_cast<HWND>(lp))
                m_up_down_control_wnd = nullptr;
            break;
        }
        break;
    case WM_ERASEBKGND:
        return FALSE;
    case WM_PAINT:
        if (colours::is_dark_mode_active())
            dark::handle_tab_control_paint(wnd);
        else
            uih::paint_subclassed_window_with_buffering(wnd, m_tab_proc);
        return 0;
    case WM_GETDLGCODE:
        return DLGC_WANTALLKEYS;
    case WM_KEYDOWN: {
        if (wp != VK_LEFT && wp != VK_RIGHT && get_host()->get_keyboard_shortcuts_enabled()
            && g_process_keydown_keyboard_shortcuts(wp))
            return 0;
        if (wp == VK_TAB) {
            g_on_tab(wnd);
            return 0;
        }
        SendMessage(wnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), NULL);
    } break;
    case WM_SYSKEYDOWN:
        if (get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp))
            return 0;
        break;
    case WM_MOUSEWHEEL: {
        if ((GetWindowLongPtr(wnd, GWL_STYLE) & TCS_MULTILINE) != 0)
            return 0;

        if (!m_up_down_control_wnd || !IsWindowVisible(m_up_down_control_wnd))
            return 0;

        int min{};
        int max{};
        SendMessage(
            m_up_down_control_wnd, UDM_GETRANGE32, reinterpret_cast<WPARAM>(&min), reinterpret_cast<LPARAM>(&max));

        const auto index = gsl::narrow<int>(SendMessage(m_up_down_control_wnd, UDM_GETPOS32, NULL, NULL));

        if (max == 0)
            return 0;

        const int wheel_delta = GET_WHEEL_DELTA_WPARAM(wp);

        m_mousewheel_delta += wheel_delta;

        POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        ScreenToClient(wnd, &pt);

        if (abs(m_mousewheel_delta) < WHEEL_DELTA)
            return 0;

        if (m_mousewheel_delta > 0 && index > min) {
            SendMessage(wnd, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, index - 1), NULL);
            SendMessage(wnd, WM_HSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), NULL);
            SendMessage(m_up_down_control_wnd, UDM_SETPOS32, NULL, index - 1);
            SendMessage(wnd, WM_MOUSEMOVE, GET_KEYSTATE_WPARAM(wp), POINTTOPOINTS(pt));
        } else if (m_mousewheel_delta < 0 && index + 1 <= max) {
            SendMessage(wnd, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, index + 1), NULL);
            SendMessage(wnd, WM_HSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), NULL);
            SendMessage(m_up_down_control_wnd, UDM_SETPOS32, NULL, index + 1);
            SendMessage(wnd, WM_MOUSEMOVE, GET_KEYSTATE_WPARAM(wp), POINTTOPOINTS(pt));
        }

        m_mousewheel_delta = 0;
        return 0;
    }
    }
    return CallWindowProc(m_tab_proc, wnd, msg, wp, lp);
}

class TabStackFontClient : public fonts::client {
public:
    const GUID& get_client_guid() const override { return g_guid_splitter_tabs; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Tab stack"; }

    fonts::font_type_t get_default_font_type() const override { return fonts::font_type_labels; }

    void on_font_changed() const override { TabStackPanel::g_on_font_change(); }
};

TabStackFontClient::factory<TabStackFontClient> g_font_client_splitter_tabs;

} // namespace cui::panels::tab_stack
