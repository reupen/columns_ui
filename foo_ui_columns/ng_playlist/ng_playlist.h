#pragma once

#include "ng_playlist_style.h"
#include "../playlist_view_tfhooks.h"
#include "../artwork.h"
#include "../config.h"
#include "../list_view_panel.h"

namespace pvt {
extern const GUID g_guid_items_font, g_guid_header_font, g_guid_group_header_font;

extern cfg_bool cfg_show_artwork, cfg_artwork_reflection, cfg_artwork_lowpriority;
extern fbh::ConfigUint32DpiAware cfg_artwork_width;
extern fbh::ConfigBool cfg_grouping;

HBITMAP g_create_hbitmap_from_image(Gdiplus::Bitmap& bm, t_size& cx, t_size& cy, COLORREF cr_back, bool b_reflection);
HBITMAP g_create_hbitmap_from_data(
    const album_art_data_ptr& data, t_size& cx, t_size& cy, COLORREF cr_back, bool b_reflection);
bool g_get_default_nocover_bitmap_data(album_art_data_ptr& p_out, abort_callback& p_abort);
HBITMAP g_get_nocover_bitmap(t_size cx, t_size cy, COLORREF cr_back, bool b_reflection, abort_callback& p_abort);
void set_font_size(bool up);

class playlist_callback_base : public playlist_callback {
public:
    void initialise_playlist_callback(t_uint32 p_flags = playlist_callback::flag_all)
    {
        static_api_ptr_t<playlist_manager>()->register_callback(this, p_flags);
    }
    void deinitialise_playlist_callback() { static_api_ptr_t<playlist_manager>()->unregister_callback(this); }
    void set_callback_flags(t_uint32 p_flags) { static_api_ptr_t<playlist_manager>()->modify_callback(this, p_flags); }
    // dummy implementations - avoid possible pure virtual function calls!
    void on_items_added(t_size p_playlist, t_size p_start, const pfc::list_base_const_t<metadb_handle_ptr>& p_data,
        const pfc::bit_array& p_selection) override
    {
    }
    void on_items_reordered(t_size p_playlist, const t_size* p_order, t_size p_count) override {}
    void on_items_removing(
        t_size p_playlist, const pfc::bit_array& p_mask, t_size p_old_count, t_size p_new_count) override
    {
    }
    void on_items_removed(
        t_size p_playlist, const pfc::bit_array& p_mask, t_size p_old_count, t_size p_new_count) override
    {
    }
    void on_items_selection_change(
        t_size p_playlist, const pfc::bit_array& p_affected, const pfc::bit_array& p_state) override
    {
    }
    void on_item_focus_change(t_size p_playlist, t_size p_from, t_size p_to) override {}

    void on_items_modified(t_size p_playlist, const pfc::bit_array& p_mask) override {}
    void on_items_modified_fromplayback(
        t_size p_playlist, const pfc::bit_array& p_mask, play_control::t_display_level p_level) override
    {
    }

    void on_items_replaced(t_size p_playlist, const pfc::bit_array& p_mask,
        const pfc::list_base_const_t<t_on_items_replaced_entry>& p_data) override
    {
    }

    void on_item_ensure_visible(t_size p_playlist, t_size p_idx) override {}

    void on_playlist_activate(t_size p_old, t_size p_new) override {}
    void on_playlist_created(t_size p_index, const char* p_name, t_size p_name_len) override {}
    void on_playlists_reorder(const t_size* p_order, t_size p_count) override {}
    void on_playlists_removing(const pfc::bit_array& p_mask, t_size p_old_count, t_size p_new_count) override {}
    void on_playlists_removed(const pfc::bit_array& p_mask, t_size p_old_count, t_size p_new_count) override {}
    void on_playlist_renamed(t_size p_index, const char* p_new_name, t_size p_new_name_len) override {}

    void on_default_format_changed() override {}
    void on_playback_order_changed(t_size p_new_index) override {}
    void on_playlist_locked(t_size p_playlist, bool p_locked) override {}
};

template <class item_t>
class playlist_cache_t
    : private playlist_callback_base
    , public pfc::list_t<item_t> {
    void on_playlist_created(t_size p_index, const char* p_name, t_size p_name_len) override
    {
        this->insert_item(item_t(), p_index);
    }
    void on_playlists_reorder(const t_size* p_order, t_size p_count) override { this->reorder(p_order); }
    void on_playlists_removed(const pfc::bit_array& p_mask, t_size p_old_count, t_size p_new_count) override
    {
        this->remove_mask(p_mask);
    }

public:
    void initialise_playlist_callback()
    {
        this->set_count(static_api_ptr_t<playlist_manager>()->get_playlist_count());
        playlist_callback_base::initialise_playlist_callback();
    }
    using playlist_callback_base::deinitialise_playlist_callback;
};

class playlist_cache_item_t {
public:
    bool m_initialised{false};
    t_size m_scroll_position{NULL};
    playlist_cache_item_t() = default;
};

class column_data_t {
public:
    titleformat_object::ptr m_display_script;
    titleformat_object::ptr m_style_script;
    titleformat_object::ptr m_sort_script;
};

class completion_notify_artwork_base_t : public pfc::refcounted_object_root {
public:
    using ptr_t = pfc::refcounted_object_ptr_t<completion_notify_artwork_base_t>;

    virtual void on_completion(const pfc::rcptr_t<class artwork_reader_ng_t>& p_reader) = 0;

private:
};

class artwork_reader_ng_t : public mmh::Thread {
public:
    bool is_aborting() { return m_abort.is_aborting(); }
    void abort() { m_abort.abort(); }

    // only called when thread closed
    bool did_succeed() { return m_succeeded; }
    bool is_ready() { return !is_thread_open(); }
    const pfc::map_t<GUID, pfc::rcptr_t<gdi_object_t<HBITMAP>::ptr_t>>& get_content() const { return m_bitmaps; }

    void initialise(const pfc::chain_list_v2_t<GUID>& p_requestIds,
        const pfc::map_t<GUID, pfc::list_t<pfc::string8>>& p_repositories, t_size native_artwork_reader_mode,
        const metadb_handle_ptr& p_handle, t_size cx, t_size cy, COLORREF cr_back, bool b_reflection,
        const completion_notify_artwork_base_t::ptr_t& p_notify, class artwork_reader_manager_ng_t* const p_manager)
    {
        m_requestIds = p_requestIds;
        m_repositories = p_repositories;
        m_handle = p_handle;
        m_notify = p_notify;
        m_cx = cx;
        m_cy = cy;
        m_reflection = b_reflection;
        m_back = cr_back;
        m_manager = p_manager;
        m_native_artwork_reader_mode = native_artwork_reader_mode;
    }
    void send_completion_notification(const pfc::rcptr_t<artwork_reader_ng_t>& p_this)
    {
        if (m_notify.is_valid()) {
            m_notify->on_completion(p_this);
        }
    }

protected:
    DWORD on_thread() override;

private:
    unsigned read_artwork(abort_callback& p_abort);

    pfc::chain_list_v2_t<GUID> m_requestIds;
    pfc::map_t<GUID, pfc::rcptr_t<gdi_object_t<HBITMAP>::ptr_t>> m_bitmaps;
    pfc::map_t<GUID, pfc::list_t<pfc::string8>> m_repositories;
    t_size m_cx{0}, m_cy{0};
    COLORREF m_back{RGB(255, 255, 255)};
    bool m_reflection{false};
    metadb_handle_ptr m_handle;
    completion_notify_artwork_base_t::ptr_t m_notify;
    bool m_succeeded{false};
    t_size m_native_artwork_reader_mode{artwork_panel::fb2k_artwork_embedded_and_external};
    abort_callback_impl m_abort;
    pfc::refcounted_object_ptr_t<class artwork_reader_manager_ng_t> m_manager;
};

class artwork_reader_manager_ng_t : public pfc::refcounted_object_root {
public:
    using ptr = pfc::refcounted_object_ptr_t<artwork_reader_manager_ng_t>;
    void add_type(const GUID& p_what) { m_requestIds.add_item(p_what); }
    void abort_task(t_size index)
    {
        {
            if (m_current_readers[index]->is_thread_open()) {
                m_current_readers[index]->abort();
                m_aborting_readers.add_item(m_current_readers[index]);
            }
            m_current_readers.remove_by_idx(index);
        }
    }
    void abort_current_tasks()
    {
        m_pending_readers.remove_all();
        t_size i = m_current_readers.get_count();
        for (; i; i--)
            abort_task(i - 1);
    }
    void set_script(const GUID& p_what, const pfc::list_t<pfc::string8>& script)
    {
        // abort_current_tasks();
        m_repositories.set(p_what, script);
    }

    void reset_repository()
    {
        abort_current_tasks();
        m_repositories.remove_all();
    }

    void reset() { abort_current_tasks(); }

    enum { max_readers = 4 };

    void request(const metadb_handle_ptr& p_handle, pfc::rcptr_t<artwork_reader_ng_t>& p_out, t_size cx, t_size cy,
        COLORREF cr_back, bool b_reflection, completion_notify_artwork_base_t::ptr_t p_notify);

    void flush_pending()
    {
        t_size count = m_current_readers.get_count(), count_pending = m_pending_readers.get_count();
        if (count < max_readers) {
            if (count_pending) {
                pfc::rcptr_t<artwork_reader_ng_t> p_reader = m_pending_readers[count_pending - 1];
                m_pending_readers.remove_by_idx(count_pending - 1);
                if (pvt::cfg_artwork_lowpriority)
                    p_reader->set_priority(THREAD_PRIORITY_BELOW_NORMAL);
                p_reader->create_thread();
                m_current_readers.add_item(p_reader);
            }
        }
    }

    void initialise() {}

    void deinitialise()
    {
        m_pending_readers.remove_all();

        t_size i = m_aborting_readers.get_count();
        for (; i; i--) {
            m_aborting_readers[i - 1]->wait_for_and_release_thread();
            m_aborting_readers.remove_by_idx(i - 1);
        }
        i = m_current_readers.get_count();
        for (; i; i--) {
            m_current_readers[i - 1]->wait_for_and_release_thread();
            m_current_readers.remove_by_idx(i - 1);
        }

        {
            insync(m_nocover_sync);
            m_nocover_bitmap.release();
        }
    }

    void on_reader_completion(const artwork_reader_ng_t* ptr);
    void on_reader_abort(const artwork_reader_ng_t* ptr);

    artwork_reader_manager_ng_t() = default;

    void request_nocover_image(pfc::rcptr_t<gdi_object_t<HBITMAP>::ptr_t>& p_out, t_size cx, t_size cy,
        COLORREF cr_back, bool b_reflection, abort_callback& p_abort);
    void flush_nocover() { m_nocover_bitmap.release(); }

private:
    bool find_aborting_reader(const artwork_reader_ng_t* ptr, t_size& index)
    {
        t_size count = m_aborting_readers.get_count();
        for (t_size i = 0; i < count; i++)
            if (&*m_aborting_readers[i] == ptr) {
                index = i;
                return true;
            }
        return false;
    }
    bool find_current_reader(const artwork_reader_ng_t* ptr, t_size& index)
    {
        t_size count = m_current_readers.get_count();
        for (t_size i = 0; i < count; i++)
            if (&*m_current_readers[i] == ptr) {
                index = i;
                return true;
            }
        return false;
    }
    pfc::list_t<pfc::rcptr_t<artwork_reader_ng_t>> m_aborting_readers;
    pfc::list_t<pfc::rcptr_t<artwork_reader_ng_t>> m_current_readers;
    pfc::list_t<pfc::rcptr_t<artwork_reader_ng_t>> m_pending_readers;

    pfc::chain_list_v2_t<GUID> m_requestIds;
    pfc::map_t<GUID, pfc::list_t<pfc::string8>> m_repositories;

    critical_section m_nocover_sync;
    pfc::rcptr_t<gdi_object_t<HBITMAP>::ptr_t> m_nocover_bitmap;
    t_size m_nocover_cx{0}, m_nocover_cy{0};
};

class appearance_client_ngpv_impl : public cui::colours::client {
public:
    static const GUID g_guid;

    const GUID& get_client_guid() const override { return g_guid; };

    void get_name(pfc::string_base& p_out) const override { p_out = "Playlist view"; };

    t_size get_supported_colours() const override { return cui::colours::colour_flag_all; }; // bit-mask
    t_size get_supported_bools() const override
    {
        return cui::colours::bool_flag_use_custom_active_item_frame;
    }; // bit-mask
    bool get_themes_supported() const override { return true; };

    void on_colour_changed(t_size mask) const override;

    void on_bool_changed(t_size mask) const override{};
};

class ng_playlist_view_t
    : public t_list_view_panel<appearance_client_ngpv_impl, uie::playlist_window>
    , playlist_callback_single
    , playlist_callback_base {
    friend class NgTfThread;

    class ng_global_mesage_window : public ui_helpers::container_window {
        class_data& get_class_data() const override
        {
            __implement_get_class_data_ex(_T("NGPV_GMW"), _T(""), false, 0, 0, 0, 0);
        }

        LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override
        {
            switch (msg) {
            case WM_CREATE:
                break;
            case WM_TIMECHANGE:
                ng_playlist_view_t::g_on_time_change();
                break;
            case WM_DESTROY:
                break;
            }
            return DefWindowProc(wnd, msg, wp, lp);
        }
    };

public:
    ng_playlist_view_t();
    ~ng_playlist_view_t();

    static std::vector<ng_playlist_view_t*> g_windows;
    static ng_global_mesage_window g_global_mesage_window;

    static void g_on_groups_change();
    static void g_on_columns_change();
    static void g_on_column_widths_change(const ng_playlist_view_t* p_skip = nullptr);
    static void g_update_all_items();
    static void g_on_autosize_change();
    static void g_on_show_artwork_change();
    static void g_on_alternate_selection_change();
    static void g_on_artwork_width_change(const ng_playlist_view_t* p_skip = nullptr);
    static void g_flush_artwork(bool b_redraw = false, const ng_playlist_view_t* p_skip = nullptr);
    static void g_on_artwork_repositories_change();
    static void g_on_vertical_item_padding_change();
    static void g_on_show_header_change();
    static void g_on_playback_follows_cursor_change(bool b_val);
    static void g_on_show_tooltips_change();
    static void g_on_font_change();
    static void g_on_header_font_change();
    static void g_on_group_header_font_change();
    static void g_on_sorting_enabled_change();
    static void g_on_show_sort_indicators_change();
    static void g_on_edge_style_change();
    static void s_redraw_all();
    static void g_on_time_change();

    const GUID& get_extension_guid() const override;
    void get_name(pfc::string_base& out) const override;
    bool get_short_name(pfc::string_base& out) const override;
    void get_category(pfc::string_base& out) const override;
    unsigned get_type() const override;

    bool m_dragging{false};
    pfc::com_ptr_t<IDataObject> m_DataObject;
    t_size m_dragging_initial_playlist;

protected:
    using InsertItemsContainer = pfc::array_t<InsertItem>;

private:
    static const GUID g_extension_guid;
    enum { timer_date_change = TIMER_BASE };

    class item_group_ng_t : public Group {
    public:
        using self_t = item_group_ng_t;
        using ptr = pfc::refcounted_object_ptr_t<self_t>;
        style_data_cell_t::ptr m_style_data;

        bool m_artwork_load_attempted{false};
        bool m_artwork_load_succeeded{false};
        // bool m_data_to_bitmap_attempted;
        // album_art_data_ptr m_artwork_data;
        pfc::rcptr_t<gdi_object_t<HBITMAP>::ptr_t> m_artwork_bitmap; // cached for display

        item_group_ng_t() = default;
        ;
    };

    class item_ng_t : public Item {
    public:
        using self_t = item_ng_t;
        using ptr = pfc::refcounted_object_ptr_t<self_t>;
        style_data_t m_style_data;
        item_group_ng_t* get_group(t_size index) { return static_cast<item_group_ng_t*>(m_groups[index].get_ptr()); }
        t_size get_group_count() { return m_groups.get_count(); }
    };
    virtual void flush_artwork_images()
    {
        if (m_artwork_manager.is_valid()) {
            m_artwork_manager->abort_current_tasks();
            m_artwork_manager->flush_nocover();
        }
        t_size count = get_item_count();
        for (t_size i = 0; i < count; i++) {
            t_size cg = get_item(i)->get_group_count();
            if (cg) {
                item_group_ng_t* ptr = get_item(i)->get_group(cg - 1);
                // ptr->m_data_to_bitmap_attempted = false;
                ptr->m_artwork_load_succeeded = false;
                ptr->m_artwork_load_attempted = false;
                ptr->m_artwork_bitmap.release();
            }
        }
    }

    void notify_on_group_info_area_size_change(t_size new_width) override
    {
        cfg_artwork_width = new_width;
        // flush_artwork_images();
        g_on_artwork_width_change(nullptr /*this*/);
    }
    void on_artwork_read_complete(
        const item_group_ng_t::ptr& p_group, const pfc::rcptr_t<artwork_reader_ng_t>& p_reader)
    {
        if (!p_reader->is_aborting()) {
            t_size count = get_item_count(), group_count = m_scripts.get_count();

            if (group_count) {
                for (t_size i = 0; i < count; i++) {
                    auto* item = static_cast<item_ng_t*>(get_item(i));
                    if (item->get_group(group_count - 1) == p_group.get_ptr()) {
                        if (p_reader->get_content().query(album_art_ids::cover_front, p_group->m_artwork_bitmap)) {
                            p_group->m_artwork_load_succeeded = true;
                            invalidate_item_group_info_area(i);
                        }
                        break;
                    }
                }
            }
        }
    }

    class completion_notify_artwork_t : public completion_notify_artwork_base_t {
    public:
        using ptr_t = pfc::refcounted_object_ptr_t<completion_notify_artwork_t>;

        void on_completion(const pfc::rcptr_t<artwork_reader_ng_t>& p_reader) override
        {
            m_window->on_artwork_read_complete(m_group, p_reader);
        }

        item_group_ng_t::ptr m_group;
        service_ptr_t<ng_playlist_view_t> m_window;

    private:
    };

    HBITMAP request_group_artwork(t_size index_item, t_size item_group_count);

    void update_all_items(bool b_update_display = true);
    void refresh_all_items_text(bool b_update_display = true);
    void update_items(t_size index, t_size count, bool b_update_display = true);

    Item* storage_create_item() override { return new item_ng_t; }

    Group* storage_create_group() override { return new item_group_ng_t; }

    item_ng_t* get_item(t_size index) { return static_cast<item_ng_t*>(uih::ListView::get_item(index)); }

    void notify_update_item_data(t_size index) override;

    const style_data_t& get_style_data(t_size index);
    void get_insert_items(t_size start, t_size count, InsertItemsContainer& items);
    void flush_items();
    void reset_items();

    void on_items_added(unsigned start, const pfc::list_base_const_t<metadb_handle_ptr>& p_data,
        const pfc::bit_array& p_selection) override;
    void on_items_reordered(const t_size* p_order, t_size p_count) override;

    void on_items_removing(const pfc::bit_array& p_mask, t_size p_old_count,
        t_size p_new_count) override{}; // called before actually removing them
    void on_items_removed(const pfc::bit_array& p_mask, t_size p_old_count, t_size p_new_count) override;
    void on_items_selection_change(const pfc::bit_array& p_affected, const pfc::bit_array& p_state) override;
    void on_item_focus_change(t_size p_from, t_size p_to) override;
    void on_items_modified(const pfc::bit_array& p_mask) override;
    void on_items_modified_fromplayback(const pfc::bit_array& p_mask, play_control::t_display_level p_level) override;
    void on_items_replaced(const pfc::bit_array& p_mask,
        const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry>& p_data) override;
    void on_item_ensure_visible(t_size p_idx) override;
    void on_playlist_switch() override;
    void on_playlist_renamed(const char* p_new_name, t_size p_new_name_len) override;

    void on_default_format_changed() override{};

    void on_playback_order_changed(t_size p_new_index) override{};

    void on_playlist_locked(bool p_locked) override{};

    // Temp fix to avoid playlist_callback virtual functions being hidden by those of
    // playlist_callback_single
    using playlist_callback_base::on_item_ensure_visible;
    using playlist_callback_base::on_item_focus_change;
    using playlist_callback_base::on_items_added;
    using playlist_callback_base::on_items_modified;
    using playlist_callback_base::on_items_modified_fromplayback;
    using playlist_callback_base::on_items_removed;
    using playlist_callback_base::on_items_removing;
    using playlist_callback_base::on_items_reordered;
    using playlist_callback_base::on_items_replaced;
    using playlist_callback_base::on_items_selection_change;
    using playlist_callback_base::on_playlist_locked;
    using playlist_callback_base::on_playlist_renamed;

    void on_playlist_activate(t_size p_old, t_size p_new) override
    {
        if (p_old != pfc_infinite) {
            m_playlist_cache[p_old].m_initialised = true;
            m_playlist_cache[p_old].m_scroll_position = _get_scroll_position();
        }
    }

    virtual void populate_list();
    virtual void refresh_groups(bool b_update_columns = false);
    virtual void refresh_columns();
    virtual void on_groups_change();
    virtual void on_columns_change();
    void on_column_widths_change();
    t_size column_index_display_to_actual(t_size display_index);
    t_size column_index_actual_to_display(t_size actual_index);

    void notify_on_initialisation() override;
    void notify_on_create() override;
    void notify_on_destroy() override;
    void set_focus() override { SetFocus(get_wnd()); }

    t_size get_highlight_item() override;
    bool notify_on_contextmenu(const POINT& pt, bool from_keyboard) override;
    bool notify_on_contextmenu_header(const POINT& pt, const HDHITTESTINFO& ht) override;
    bool notify_on_middleclick(bool on_item, t_size index) override;
    bool notify_on_doubleleftclick_nowhere() override;

    bool notify_on_keyboard_keydown_filter(UINT msg, WPARAM wp, LPARAM lp) override;
    bool notify_on_keyboard_keydown_remove() override;

    bool notify_on_keyboard_keydown_search() override;

    bool notify_on_keyboard_keydown_undo() override;
    bool notify_on_keyboard_keydown_redo() override;
    bool notify_on_keyboard_keydown_cut() override;
    bool notify_on_keyboard_keydown_copy() override;
    bool notify_on_keyboard_keydown_paste() override;

    void notify_on_column_size_change(t_size index, int new_width) override
    {
        t_size act = column_index_display_to_actual(index);
        if (act != pfc_infinite && act < g_columns.get_count()) {
            g_columns[act]->width = new_width;
            g_on_column_widths_change(this);
        }
    };

    void notify_on_header_rearrange(t_size index_from, t_size index_to) override
    {
        t_size act_from = column_index_display_to_actual(index_from);
        t_size act_to = column_index_display_to_actual(index_to);
        g_columns.move(act_from, act_to);
        pvt::ng_playlist_view_t::g_on_columns_change();
    }

    bool notify_on_timer(UINT_PTR timerid) override
    {
        if (timerid == timer_date_change) {
            update_items(0, get_item_count());
            return true;
        }
        return false;
    };

    void on_time_change()
    {
        update_items(0, get_item_count());
        set_day_timer();
    };

    bool m_day_timer_active{false};

    void kill_day_timer()
    {
        if (m_day_timer_active) {
            KillTimer(get_wnd(), timer_date_change);
            m_day_timer_active = false;
        }
    }

    void set_day_timer()
    {
        kill_day_timer();
        SYSTEMTIME st;
        GetLocalTime(&st);
        unsigned ms
            = /*24**/ 60 * 60 * 1000 - (st.wMilliseconds + ((/*st.wHour*60 + */ st.wMinute) * 60 + st.wSecond) * 1000);
        SetTimer(get_wnd(), timer_date_change, ms, nullptr);
        m_day_timer_active = true;
    }

    void notify_sort_column(t_size index, bool b_descending, bool b_selection_only) override;
    t_size storage_get_focus_item() override;
    void storage_set_focus_item(t_size index) override;
    void storage_get_selection_state(pfc::bit_array_var& out) override;
    bool storage_set_selection_state(const pfc::bit_array& p_affected, const pfc::bit_array& p_status,
        pfc::bit_array_var* p_changed = nullptr) override;
    bool storage_get_item_selected(t_size index) override;
    t_size storage_get_selection_count(t_size max) override;

    void execute_default_action(t_size index, t_size column, bool b_keyboard, bool b_ctrl) override;
    void move_selection(int delta) override;
    bool do_drag_drop(WPARAM wp) override;

    bool notify_before_create_inline_edit(
        const pfc::list_base_const_t<t_size>& indices, unsigned column, bool b_source_mouse) override;
    bool notify_create_inline_edit(const pfc::list_base_const_t<t_size>& indices, unsigned column,
        pfc::string_base& p_text, t_size& p_flags, mmh::ComPtr<IUnknown>& pAutocompleteEntries) override;
    void notify_save_inline_edit(const char* value) override;
    void notify_exit_inline_edit() override;

    void notify_on_set_focus(HWND wnd_lost) override;
    void notify_on_kill_focus(HWND wnd_receiving) override;

    void render_group_info(HDC dc, t_size index, t_size group_count, const RECT& rc2) override;
    void render_item(HDC dc, t_size index, int indentation, bool b_selected, bool b_window_focused, bool b_highlight,
        bool should_hide_focus, bool b_focused, const RECT* rc) override;
    void render_group(
        HDC dc, t_size index, t_size group, const char* text, int indentation, t_size level, const RECT& rc) override;

    void notify_on_menu_select(WPARAM wp, LPARAM lp) override;

    service_list_t<titleformat_object> m_scripts;
    pfc::list_t<column_data_t> m_column_data;
    pfc::array_t<bool> m_column_mask;
    pfc::list_t<pfc::string8> m_edit_fields;
    pfc::string8 m_edit_field;
    metadb_handle_list m_edit_handles;
    service_ptr_t<titleformat_object> m_script_global, m_script_global_style;
    service_ptr_t<playlist_manager> m_playlist_api;
    bool m_ignore_callback{false};
    ULONG_PTR m_gdiplus_token{NULL};
    bool m_gdiplus_initialised{false};

    mainmenu_manager::ptr m_mainmenu_manager;
    contextmenu_manager::ptr m_contextmenu_manager;
    ui_status_text_override::ptr m_status_text_override;

    UINT_PTR m_mainmenu_manager_base{NULL};
    UINT_PTR m_contextmenu_manager_base{NULL};

    playlist_cache_t<playlist_cache_item_t> m_playlist_cache;

    artwork_reader_manager_ng_t::ptr m_artwork_manager;
    ui_selection_holder::ptr m_selection_holder;

    // pfc::list_t<group_t> m_groups;
    // pfc::list_t<style_data_t> m_style_data;
};

class IDropTarget_playlist : public IDropTarget {
public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID FAR* ppvObject) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
    HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
    HRESULT STDMETHODCALLTYPE DragLeave() override;
    HRESULT STDMETHODCALLTYPE Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
    IDropTarget_playlist(ng_playlist_view_t* playlist);

private:
    HRESULT UpdateDropDescription(IDataObject* pDataObj, DWORD pdwEffect);

    long drop_ref_count;
    bool last_rmb;
    bool m_is_accepted_type;
    service_ptr_t<ng_playlist_view_t> p_playlist;
    pfc::com_ptr_t<IDataObject> m_DataObject;
    mmh::ComPtr<IDropTargetHelper> m_DropTargetHelper;
};

class IDropSource_playlist : public IDropSource {
    long refcount;
    service_ptr_t<ng_playlist_view_t> p_playlist;
    DWORD m_initial_key_state;

public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) override;
    HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD dwEffect) override;
    IDropSource_playlist(ng_playlist_view_t* playlist, DWORD initial_key_state);
};

class preferences_tab_impl : public PreferencesTab {
public:
    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_PVIEW_GROUPS,
            [this](auto&&... args) { return ConfigProc(std::forward<decltype(args)>(args)...); });
    }
    const char* get_name() override { return "Grouping"; }
    bool get_help_url(pfc::string_base& p_out) override
    {
        p_out = "http://yuo.be/wiki/columns_ui:config:playlist_view:grouping";
        return true;
    }

private:
    BOOL ConfigProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    cui::prefs::PreferencesTabHelper m_helper{IDC_TITLE1};
};
} // namespace pvt

namespace playlist_utils {
bool check_clipboard();
bool cut();
bool copy();
bool paste_at_focused_item(HWND wnd);
bool paste(HWND wnd, size_t index);
} // namespace playlist_utils
