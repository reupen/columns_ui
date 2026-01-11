#pragma once

#include "core_dark_list_view.h"
#include "list_view_drop_target.h"

namespace cui::prefs {

class ColumnTab {
public:
    virtual ~ColumnTab() = default;

    virtual HWND create(HWND parent_window) = 0;
    virtual void set_column(const PlaylistViewColumn::ptr& column) = 0;
    virtual void get_column(PlaylistViewColumn::ptr& p_out) = 0;
    virtual void on_column_name_change(const PlaylistViewColumn::ptr& column) {}
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

        void notify_on_create() override
        {
            wil::com_ptr drop_target(new utils::SimpleListViewDropTarget(
                this, [this](mmh::Permutation& new_order, size_t old_index, size_t new_index) {
                    m_tab.on_column_list_reorder(new_order, old_index, new_index);
                }));

            RegisterDragDrop(get_wnd(), drop_target.get());
        }

        void notify_on_destroy() override { RevokeDragDrop(get_wnd()); }

        bool do_drag_drop(WPARAM wp) override
        {
            DWORD drop_effect{DROPEFFECT_NONE};
            const auto data_object = utils::create_simple_list_view_data_object(get_wnd());
            LOG_IF_FAILED(
                uih::ole::do_drag_drop(get_wnd(), wp, data_object.get(), DROPEFFECT_MOVE, NULL, &drop_effect));
            return true;
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

        void execute_default_action(size_t index, size_t column, bool b_keyboard, bool b_ctrl) override
        {
            activate_inline_editing(index, column);
        }

        bool notify_before_create_inline_edit(
            const pfc::list_base_const_t<size_t>& indices, size_t column, bool b_source_mouse) override
        {
            return indices.size() == 1;
        }

        bool notify_create_inline_edit(const pfc::list_base_const_t<size_t>& indices, size_t column,
            pfc::string_base& p_text, size_t& p_flags, wil::com_ptr<IUnknown>& autocomplete_entries) override;

        void notify_save_inline_edit(const char* value) override;

        void notify_exit_inline_edit() override { m_inline_edit_column.reset(); }

    private:
        TabColumns& m_tab;
        PlaylistViewColumn::ptr m_inline_edit_column;
    };

    TabColumns() = default;

    bool on_column_list_contextmenu(const POINT& pt, bool from_keyboard);
    void on_column_list_selection_change();
    void on_column_list_reorder(mmh::Permutation& permutation, size_t old_index, size_t new_index);
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
