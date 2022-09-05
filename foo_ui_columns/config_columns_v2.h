#pragma once

class ColumnTab {
public:
    virtual ~ColumnTab() = default;

    virtual HWND create(HWND wnd) = 0;
    // virtual void destroy(HWND wnd)=0;
    // virtual const char * get_name()=0;
    virtual void set_column(const PlaylistViewColumn::ptr& column) = 0;
    virtual void get_column(PlaylistViewColumn::ptr& p_out) = 0;
};

class TabColumns : public PreferencesTab {
private:
    HWND m_wnd_child{nullptr};
    HWND m_wnd{nullptr};
    HWND m_wnd_lv{nullptr};
    std::unique_ptr<ColumnTab> m_child;
    // edit_column_window_options m_tab_options;
    // edit_column_window_scripts m_tab_scripts;
public:
    static TabColumns& get_instance()
    {
        static TabColumns tab_columns_v3_;
        return tab_columns_v3_;
    }

    INT_PTR on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

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
    TabColumns() = default;

    cui::prefs::PreferencesTabHelper m_helper{{{IDC_TITLE1}}, false};
    ColumnList m_columns;
    bool initialising{false};
};
