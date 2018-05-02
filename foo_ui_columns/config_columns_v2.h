#pragma once

class column_tab : public pfc::refcounted_object_root {
public:
    using self_t = column_tab;
    using ptr = pfc::refcounted_object_ptr_t<self_t>;
    virtual HWND create(HWND wnd) = 0;
    // virtual void destroy(HWND wnd)=0;
    // virtual const char * get_name()=0;
    virtual void set_column(const column_t::ptr& column) = 0;
    virtual void get_column(column_t::ptr& p_out) = 0;
};

class tab_columns_v3 : public preferences_tab {
private:
    HWND m_wnd_child{nullptr};
    HWND m_wnd{nullptr};
    HWND m_wnd_lv{nullptr};
    column_tab::ptr m_child;
    // edit_column_window_options m_tab_options;
    // edit_column_window_scripts m_tab_scripts;
public:
    static tab_columns_v3& get_instance()
    {
        static tab_columns_v3 tab_columns_v3_;
        return tab_columns_v3_;
    }

    BOOL on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_COLUMNS,
            [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); });
    }
    void make_child();
    void refresh_me(HWND wnd, bool init = false);
    void apply();
    void show_column(size_t index);

    const char* get_name() override { return "Columns"; }
    bool get_help_url(pfc::string_base& p_out) override
    {
        p_out = "http://yuo.be/wiki/columns_ui:config:playlist_view:columns";
        return true;
    }

private:
    tab_columns_v3() = default;

    cui::prefs::PreferencesTabHelper m_helper{IDC_TITLE1};
    column_list_t m_columns;
    bool initialising{false};
};
