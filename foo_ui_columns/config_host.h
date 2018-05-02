#pragma once

class preferences_tab {
public:
    virtual HWND create(HWND wnd) = 0;
    virtual const char* get_name() = 0;
    virtual bool get_help_url(pfc::string_base& p_out) = 0;
};

namespace cui::prefs {

class PreferencesTabHelper {
public:
    PreferencesTabHelper(std::initializer_list<unsigned> title_ctrl_ids) : m_title_ctrl_ids(title_ctrl_ids) {}

    HWND create(HWND wnd, UINT id, std::function<BOOL(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)> on_message_callback);

private:
    static BOOL CALLBACK s_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    BOOL on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    void on_initdialog(HWND wnd);
    void on_ncdestroy();

    HWND m_wnd{};
    HFONT m_title_font{};
    std::set<unsigned> m_title_ctrl_ids;
    std::function<BOOL(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)> m_on_message_callback;
};

} // namespace cui::prefs

class config_host_generic : public preferences_page {
public:
    config_host_generic(const char* p_name, preferences_tab* const* const p_tabs, size_t p_tab_count,
        const GUID& p_guid, const GUID& p_parent_guid, cfg_int* const p_active_tab)
        : m_child(nullptr)
        , m_name(p_name)
        , m_guid(p_guid)
        , m_parent_guid(p_parent_guid)
        , m_tabs(p_tabs)
        , m_tab_count(p_tab_count)
        , m_active_tab(*p_active_tab)
        , m_wnd(nullptr)
        , m_wnd_tabs(nullptr)
    {
    }

    HWND create(HWND parent) override
    {
        return uCreateDialog(IDD_PREFS_TAB_HOST, parent, g_on_message, reinterpret_cast<LPARAM>(this));
    }

    const char* get_name() override { return m_name; }

    GUID get_guid() override { return m_guid; }

    GUID get_parent_guid() override { return m_parent_guid; }

    bool reset_query() override { return false; }

    void reset() override{};

    bool get_help_url(pfc::string_base& p_out) override
    {
        if (!(m_active_tab < (int)m_tab_count && m_tabs[m_active_tab]->get_help_url(p_out)))
            p_out = "http://yuo.be/wiki/columns_ui:manual";
        return true;
    }

    void show_tab(const char* tab_name);

private:
    void destroy_child()
    {
        if (m_child) {
            ShowWindow(m_child, SW_HIDE);
            DestroyWindow(m_child);
            m_child = nullptr;
        }
    }

    void make_child();

    static BOOL CALLBACK g_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    BOOL CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    HWND m_child;
    const char* m_name;
    const GUID &m_guid, &m_parent_guid;
    preferences_tab* const* const m_tabs;
    const size_t m_tab_count;
    cfg_int& m_active_tab;
    HWND m_wnd, m_wnd_tabs;
};
