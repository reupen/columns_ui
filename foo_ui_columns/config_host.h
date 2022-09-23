#pragma once

class PreferencesTab {
public:
    virtual HWND create(HWND wnd) = 0;
    virtual const char* get_name() = 0;
    virtual bool get_help_url(pfc::string_base& p_out) = 0;
};

namespace cui::prefs {

class PreferencesTabHelper {
public:
    explicit PreferencesTabHelper(std::initializer_list<unsigned> title_ctrl_ids) : m_title_ctrl_ids(title_ctrl_ids) {}

    HWND create(
        HWND wnd, UINT id, std::function<INT_PTR(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)> on_message_callback);
    HWND get_wnd() const { return m_wnd; }
    HWND get_control_wnd(int item_id) const { return GetDlgItem(m_wnd, item_id); }

private:
    INT_PTR on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    void on_initdialog(HWND wnd);
    void on_ncdestroy();

    HWND m_wnd{};
    HFONT m_title_font{};
    std::set<unsigned> m_title_ctrl_ids;
    std::function<INT_PTR(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)> m_on_message_callback;
};

} // namespace cui::prefs

class PreferencesInstanceTabsHost : public preferences_page_instance {
public:
    PreferencesInstanceTabsHost(std::function<void(const PreferencesInstanceTabsHost*)> destroy_callback,
        HWND parent_window, std::span<PreferencesTab*> tabs, cfg_int& active_tab)
        : m_tabs(tabs)
        , m_active_tab(active_tab)
        , m_destroy_callback(destroy_callback)
    {
        auto on_message_ = [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); };

        std::tie(m_wnd, m_has_dark_mode)
            = fbh::auto_dark_modeless_dialog_box(IDD_PREFS_TAB_HOST, parent_window, std::move(on_message_));
    }

    ~PreferencesInstanceTabsHost() { m_destroy_callback(this); }

    t_uint32 get_state() override { return m_has_dark_mode ? preferences_state::dark_mode_supported : 0; }
    fb2k::hwnd_t get_wnd() override { return m_wnd; }
    void apply() override {}
    void reset() override {}

    void on_active_tab_change();

private:
    INT_PTR CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    void destroy_child()
    {
        if (m_child) {
            ShowWindow(m_child, SW_HIDE);
            DestroyWindow(m_child);
            m_child = nullptr;
        }
    }

    void make_child();

    HWND m_wnd{};
    HWND m_child{};
    HWND m_wnd_tabs{};
    bool m_has_dark_mode{};
    std::span<PreferencesTab*> m_tabs;
    cfg_int& m_active_tab;
    std::function<void(const PreferencesInstanceTabsHost*)> m_destroy_callback;
};

class PreferencesTabsHost : public preferences_page_v3 {
public:
    PreferencesTabsHost(
        const char* p_name, std::span<PreferencesTab*> tabs, GUID p_guid, GUID p_parent_guid, cfg_int& p_active_tab)
        : m_name(p_name)
        , m_guid(p_guid)
        , m_parent_guid(p_parent_guid)
        , m_tabs(tabs)
        , m_active_tab(p_active_tab)
    {
    }

    const char* get_name() override { return m_name; }

    GUID get_guid() override { return m_guid; }

    GUID get_parent_guid() override { return m_parent_guid; }

    bool reset_query() override { return false; }

    void reset() override {}

    bool get_help_url(pfc::string_base& p_out) override
    {
        if (!(m_active_tab < (int)m_tabs.size() && m_tabs[m_active_tab]->get_help_url(p_out)))
            p_out = "http://yuo.be/wiki/columns_ui:manual";
        return true;
    }

    void show_tab(const char* tab_name);

    preferences_page_instance::ptr instantiate(fb2k::hwnd_t parent, preferences_page_callback::ptr callback) override
    {
        auto instance = fb2k::service_new<PreferencesInstanceTabsHost>(
            [this](const PreferencesInstanceTabsHost* instance) { std::erase(m_instances, instance); }, parent, m_tabs,
            m_active_tab);
        m_instances.emplace_back(instance.get_ptr());
        return instance;
    }

private:
    const char* m_name{};
    const GUID m_guid{};
    const GUID m_parent_guid{};
    std::span<PreferencesTab*> m_tabs;
    cfg_int& m_active_tab;
    std::vector<PreferencesInstanceTabsHost*> m_instances;
};
