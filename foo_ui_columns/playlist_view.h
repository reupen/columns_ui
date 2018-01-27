#ifndef _COLUMNS_UI_PLAYLIST_VIEW_H_
#define _COLUMNS_UI_PLAYLIST_VIEW_H_

/**
 * \deprecated    Columns playlist is deprecated and has been superseded by NG playlist.
 */

#include "cache.h"
#include "columns_v2.h"
#include "main_window.h"

#define INLINE_EDIT 1
#define cfg_inline_edit 1
#define EDIT_TIMER_ID 668

extern service_ptr_t<titleformat_object> g_to_global;
extern service_ptr_t<titleformat_object> g_to_global_colour;

void refresh_all_playlist_views();

class playlist_view;
class IDropTarget_playlist;

class titleformat_hook_playlist_name : public titleformat_hook {
    bool m_initialised{false};
    pfc::string8 m_name;

public:
    void initialise();
    bool process_field(
        titleformat_text_out* p_out, const char* p_name, unsigned p_name_length, bool& p_found_flag) override;

    bool process_function(titleformat_text_out* p_out, const char* p_name, unsigned p_name_length,
        titleformat_hook_function_params* p_params, bool& p_found_flag) override
    {
        return false;
    };
    inline titleformat_hook_playlist_name() = default;
    ;
};

class IDropSource_playlist : public IDropSource {
    long refcount;
    service_ptr_t<playlist_view> p_playlist;

public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;

    HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) override;

    HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD dwEffect) override;

    IDropSource_playlist(playlist_view* playlist);
    ;
};

class IDropTarget_playlist : public IDropTarget {
    long drop_ref_count;
    bool last_rmb;
    service_ptr_t<playlist_view> p_playlist;

public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID FAR* ppvObject) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
    HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
    HRESULT STDMETHODCALLTYPE DragLeave() override;
    HRESULT STDMETHODCALLTYPE Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
    IDropTarget_playlist(playlist_view* playlist);
};

class playlist_message_window : public ui_helpers::container_window {
    long ref_count{0};

public:
    class_data& get_class_data() const override;

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;
    void add_ref();
    void release();
    playlist_message_window() = default;
    ;
};

class playlist_view
    : public ui_extension::container_ui_extension_t<ui_helpers::container_window, uie::playlist_window> {
    class_data& get_class_data() const override;
    //    static refcounted_ptr_t<titleformat_object> g_to_global;
    //    static refcounted_ptr_t<titleformat_object> g_to_global_colour;

public:
    enum { MSG_KILL_INLINE_EDIT = WM_USER + 3 };
    // IDropTarget_playlist IDT_playlist;
    //    sort g_sort;
    static GUID extension_guid;

    static pfc::ptr_list_t<playlist_view> list_playlist;
    HWND wnd_playlist{nullptr}, wnd_header{nullptr};

    // static LRESULT WINAPI window_proc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;

    void move_header(bool redraw = true, bool update = true);
    void rebuild_header(bool full_rebuild = true);
    void update_scrollbar(bool redraw_horz = false);
    bool draw_items(int start_item, int count);
    bool draw_items(HDC dc, int start_item, int count);
    bool draw_items_wrapper(int start_item, int count = 1);
    int hittest_item(int x, int y, bool check_in_column = true);
    int hittest_item_no_scroll(int x, int y, bool check_in_column = true);
    int hittest_column(int x, long& width);
    bool is_item_clipped(int idx, int col);
    void process_keydown(int offset, bool alt_down, bool prevent_redrawing, bool repeat);
    bool ensure_visible(int idx, bool check = false);
    unsigned int calculate_header_height();
    int get_header_height();
    int get_item_height();
    LRESULT CreateToolTip(const char* text);
    void create_header(bool visible = true);
    unsigned get_last_viewable_item();
    bool process_keydown(UINT msg, LPARAM lp, WPARAM wp, bool playlist = true, bool keyb = true);
    void get_playlist_rect(RECT* out);

    void set_focus() override
    {
        if (wnd_playlist)
            SetFocus(wnd_playlist);
    }

    inline bool is_visible(int idx) { return ensure_visible(idx, true); }

    static void g_on_columns_size_change(const playlist_view* p_skip = nullptr);

    static void update_all_windows(HWND wnd_header_skip = nullptr);

    static void g_on_playback_follows_cursor_change(bool b_val);

    static void on_playlist_activate(unsigned p_old, unsigned p_new);

    playlist_view();

    ~playlist_view();

    const GUID& get_extension_guid() const override;

    void get_name(pfc::string_base& out) const override;
    void get_category(pfc::string_base& out) const override;
    bool get_short_name(pfc::string_base& out) const override;

    unsigned get_type() const override { return ui_extension::type_playlist | ui_extension::type_panel; };

    inline static const pfc::list_base_const_t<column_t::ptr>& g_get_columns() { return columns; };
    /*inline static void g_get_titleformat_object(unsigned index, string_type string, service_ptr_t<titleformat_object>
    & p_out)
    {
        return columns.get_titleformat_object(index,string, p_out);
    }*/
    static void g_get_global_style_titleformat_object(service_ptr_t<titleformat_object>& p_out);

    static void g_load_columns(); // from config
    static void g_reset_columns(); // from config
    inline static void g_save_columns() { g_columns.set_entries_copy(columns); }
    inline static void g_kill_columns()
    {
        // g_save_columns();
        columns.remove_all();
    }

    static unsigned int g_columns_get_total_width();
    static unsigned int g_columns_get_total_parts();

    int get_columns_total_width() const;

    int get_column_width(unsigned column_index) const; // ACTIVE idx!

    int get_column_widths(pfc::array_t<int, pfc::alloc_fast_aggressive>& p_out) const;

    static unsigned g_columns_get_width(unsigned column); // ACTIVE idx!!
    inline static playlist_view_cache& g_get_cache() { return g_cache; }

    static void g_set_sort(unsigned column, bool descending, bool selection_only = false);
    static void g_update_sort();
    static void g_remove_sort();

    bool drawing_enabled{false};

    void on_size();
    void on_size(unsigned cx, unsigned cy);

private:
    //#ifdef INLINE_EDIT
    HWND m_wnd_edit{nullptr};
    void exit_inline_edit();
    // void create_inline_edit(t_size index, unsigned column);
    // void save_inline_edit();
    unsigned m_edit_index{(std::numeric_limits<unsigned>::max)()};
    unsigned m_edit_column{(std::numeric_limits<unsigned>::max)()};
    // long m_edit_x;
    bool m_prev_sel{false};
    bool m_no_next_edit{false};
    bool m_edit_timer{false};
    bool m_edit_save{true};
    bool m_edit_saving{false};
    metadb_handle_ptr m_edit_item;
    pfc::string_simple m_edit_field;

    bool m_edit_changed{false};
    metadb_handle_list m_edit_items;
    pfc::list_t<t_uint32> m_edit_indices;
    void create_inline_edit_v2(const pfc::list_base_const_t<t_uint32>& indices, unsigned column);
    void create_inline_edit_v2(t_uint32 index, unsigned column);
    void create_inline_edit_v2(t_uint32 index, t_uint32 count, unsigned column);
    void save_inline_edit_v2();
    static LRESULT WINAPI g_inline_edit_hook_v2(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT WINAPI on_inline_edit_message_v2(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    inline void scroll_veritcally(unsigned sb_command)
    {
        SendMessage(wnd_playlist, WM_VSCROLL, MAKEWPARAM(sb_command, 0), 0);
    }
    enum t_scroll_direction { scroll_horizontally, scroll_vertically };
    enum t_scroll_type { scroll_position_delta, scroll_sb };
    void scroll(t_scroll_direction p_direction, t_scroll_type p_type, int p_value);

    //#endif

    static LRESULT WINAPI g_inline_edit_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT WINAPI on_inline_edit_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    WNDPROC m_inline_edit_proc{nullptr};

    t_local_cache m_cache;

    bool initialised{false};
    bool dragged{true};
    int drag_type{0};
    unsigned dragitem{0}, dragstartitem{0};
    POINT drag_start;
    POINT drag_start_lmb;
    int last_idx{-1};
    int last_column{-1};
    int g_shift_item_start{0};

    bool g_drag_lmb{false}, m_rmb_is_dragging{false};

    int scroll_item_offset{0}, horizontal_offset{0};

    bool m_always_show_focus{false};
    bool m_prevent_wm_char_processing{false};

    RECT tooltip;

    service_ptr_t<mainmenu_manager> g_main_menu_a;
    unsigned MENU_A_BASE{1};
    service_ptr_t<contextmenu_manager> g_main_menu_b;
    unsigned MENU_B_BASE{0};

    service_ptr_t<ui_status_text_override> m_status_override;

    bool m_shown{false};

    HTHEME m_theme{nullptr};

    ui_selection_holder::ptr m_selection_holder;

    // quickfind_window m_searcher;

    static column_list_t columns;
    static playlist_view_cache g_cache;

    friend class IDropSource_playlist;
    friend class IDropTarget_playlist;
    friend class playlist_callback_columns;
    friend class playlist_callback_single_playlist;
};

class appearance_client_pv_impl : public cui::colours::client {
public:
    static const GUID g_guid;

    const GUID& get_client_guid() const override { return g_guid; };

    void get_name(pfc::string_base& p_out) const override { p_out = "Legacy playlist"; };

    t_size get_supported_colours() const override { return cui::colours::colour_flag_all; }; // bit-mask

    t_size get_supported_bools() const override
    {
        return cui::colours::bool_flag_use_custom_active_item_frame;
    }; // bit-mask
    bool get_themes_supported() const override { return true; };

    void on_colour_changed(t_size mask) const override { refresh_all_playlist_views(); };

    void on_bool_changed(t_size mask) const override{};
};

void set_day_timer();
void kill_day_timer();
void CALLBACK on_day_change();

class titleformat_hook_style : public titleformat_hook {
    colourinfo p_default_colours;
    pfc::array_t<char> text, selected_text, back, selected_back, selected_back_no_focus, selected_text_no_focus;
    colourinfo& p_colours;

public:
    bool process_field(
        titleformat_text_out* p_out, const char* p_name, unsigned p_name_length, bool& p_found_flag) override;
    bool process_function(titleformat_text_out* p_out, const char* p_name, unsigned p_name_length,
        titleformat_hook_function_params* p_params, bool& p_found_flag) override;
    inline titleformat_hook_style(colourinfo& vars) : p_default_colours(vars), p_colours(vars){};
};

extern playlist_message_window g_playlist_message_window;

#endif