#pragma once

#include "core_dark_list_view.h"

namespace cui::prefs {

class ColumnTab {
public:
    virtual ~ColumnTab() = default;

    virtual HWND create(HWND parent_window) = 0;
    virtual void set_column(const PlaylistViewColumn::ptr& column) = 0;
    virtual void get_column(PlaylistViewColumn::ptr& p_out) = 0;
};

class TabColumns : public PreferencesTab {
public:
    static TabColumns& get_instance()
    {
        static TabColumns instance;
        return instance;
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
    class ColumnsListView : public cui::helpers::CoreDarkListView {
    public:
        explicit ColumnsListView(TabColumns* tab) : m_tab(*tab) {}

        void notify_on_initialisation() override
        {
            CoreDarkListView::notify_on_initialisation();

            set_selection_mode(SelectionMode::SingleRelaxed);
            set_show_header(false);
            set_columns({{"Column", 100}});
            set_autosize(true);
        }

        void notify_on_selection_change(const pfc::bit_array& p_affected, const pfc::bit_array& p_status,
            notification_source_t p_notification_source) override
        {
            m_tab.on_column_list_selection_change();
        }

        bool notify_on_contextmenu(const POINT& pt, bool from_keyboard) override
        {
            return m_tab.on_column_list_contextmenu(pt, from_keyboard);
        }

    private:
        TabColumns& m_tab;
    };

    TabColumns() = default;

    bool on_column_list_contextmenu(const POINT& pt, bool from_keyboard);
    void on_column_list_selection_change();
    void add_column(size_t index);
    void remove_column(size_t index);
    void move_column_up(size_t index);
    void move_column_down(size_t index);

    HWND m_wnd_child{nullptr};
    HWND m_wnd{nullptr};
    std::unique_ptr<ColumnTab> m_child;
    cui::prefs::PreferencesTabHelper m_helper{{IDC_TITLE1}};
    ColumnList m_columns;
    ColumnsListView m_columns_list_view{this};
    bool initialising{false};
};

} // namespace cui::prefs
