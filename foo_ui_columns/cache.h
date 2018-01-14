#ifndef _DISPLAY_CACHE_H_
#define _DISPLAY_CACHE_H_

struct colourinfo
{
public:
    unsigned long ref;
    colour text_colour;
    colour selected_text_colour;
    colour background_colour;
    colour selected_background_colour;
    colour selected_text_colour_non_focus;
    colour selected_background_colour_non_focus;
    colour frame_left;
    colour frame_top;
    colour frame_right;
    colour frame_bottom;
    bool use_frame_left : 1;
    bool use_frame_top : 1;
    bool use_frame_right : 1;
    bool use_frame_bottom : 1;
    inline void add_ref()
    {
        ref++;
    }
    inline bool release()
    {
        if (ref) ref--;
        return (ref == 0);
    }
    inline void copy_to (colourinfo & out) const
    {
        out.text_colour = text_colour;
        out.selected_text_colour = selected_text_colour;
        out.background_colour = background_colour;
        out.selected_background_colour = selected_background_colour;
        out.selected_text_colour_non_focus = selected_text_colour_non_focus;
        out.selected_background_colour_non_focus = selected_background_colour_non_focus;
        out.frame_left = frame_left;
        out.frame_top = frame_top;
        out.frame_right = frame_right;
        out.frame_bottom = frame_bottom; 
        out.use_frame_left = use_frame_left;
        out.use_frame_top = use_frame_top;
        out.use_frame_right = use_frame_right;
        out.use_frame_bottom = use_frame_bottom;
    }
    inline colourinfo(COLORREF text, COLORREF text_sel, COLORREF back, COLORREF back_sel, COLORREF text_no_focus, COLORREF sel_no_focus) 
        : ref(1), use_frame_left(false), use_frame_top(false), use_frame_right(false), use_frame_bottom(false)
    {
        text_colour.set(text);
        selected_text_colour.set(text_sel);
        background_colour.set(back);
        selected_background_colour.set(back_sel);
        selected_text_colour_non_focus.set(text_no_focus);
        selected_background_colour_non_focus.set(sel_no_focus);
    }
    inline colourinfo(const colourinfo & in) : ref(1)
    {
        in.copy_to(*this);
    }
};

inline bool operator==(const colourinfo & c1, const colourinfo & c2)
{
    return (c1.text_colour == c2.text_colour && 
        c1.selected_text_colour == c2.selected_text_colour && 
        c1.background_colour == c2.background_colour && 
        c1.selected_background_colour == c2.selected_background_colour &&
        c1.selected_text_colour_non_focus == c2.selected_text_colour_non_focus &&
        c1.selected_background_colour_non_focus == c2.selected_background_colour_non_focus &&
        c1.use_frame_left == c2.use_frame_left &&
        c1.use_frame_right == c2.use_frame_right &&
        c1.use_frame_bottom == c2.use_frame_bottom &&
        c1.use_frame_top == c2.use_frame_top &&
        (c1.use_frame_left ? c1.frame_left == c2.frame_left : true) &&
        (c1.use_frame_bottom ? c1.frame_bottom == c2.frame_bottom : true) &&
        (c1.use_frame_top ? c1.frame_top == c2.frame_top : true) &&
        (c1.use_frame_right ? c1.frame_right == c2.frame_right : true)
        );
}

class cache_manager
{
    ptr_list_autodel_t<colourinfo> colours;
//    ptr_list_autodel_t<string_cache> strings;
//    ptr_list_autofree_t<void> strings;
public:
    colourinfo * add_colour(colourinfo & colour_add/*COLORREF text, COLORREF text_sel, COLORREF back, COLORREF back_sel, COLORREF back_sel_nofocus*/);
//    string_cache * add_string(const char * src, unsigned len = -6);
    void release_colour(colourinfo * col);
//    void release_string(string_cache * col);
//    ~cache_manager() {if (strings.get_count() || colours.get_count()) uMessageBox(0, "leak", "mmm", 0);}
};

extern cache_manager g_cache_manager;

class display_info
{
private:
    pfc::string_simple display;
    colourinfo * colours;
public:
    void set_display(const char * new_display);
    void get_display(pfc::string_base & out);
    COLORREF get_colour(colour_type type);
    void get_colour(colourinfo & out);
    void set_colour(/*COLORREF text, COLORREF text_sel, COLORREF back, COLORREF back_sel, COLORREF back_sel_nofocus*/colourinfo & colour_add);
    display_info() : colours(nullptr) {};
    ~display_info()
    {
        if (colours) 
            g_cache_manager.release_colour(colours);
/*        if (display) 
            g_cache_manager.release_string(display);*/
    }
};

class playlist_entry_ui
{
private:
//    pfc::ptr_list_t<display_info> display_data;
    display_info * display_data;
public:
    void add_display_items(unsigned count);
    void set_display_item(int column, const char * data, colourinfo & colour_add);
    display_info * get_item(unsigned col) const;
    ~playlist_entry_ui();
    playlist_entry_ui();
};

class global_variable
{
public:
    global_variable(const char * p_name,t_size p_name_length,const char * p_value,t_size p_value_length)
        : m_name(p_name,p_name_length), m_value(p_value,p_value_length)
    {}
    inline const char * get_name() const {return m_name;}
    inline const char * get_value() const {return m_value;}
private:
    pfc::string_simple m_name,m_value;
};

class global_variable_list : public pfc::ptr_list_t<global_variable>
{
public:
    const char * find_by_name(const char * p_name, unsigned length)
    {
        unsigned n, count = get_count();
        for (n=0;n<count;n++)
        {
            const char * ptr = get_item(n)->get_name();
            if (!stricmp_utf8_ex(p_name,length,ptr,pfc_infinite))
                return get_item(n)->get_value();
        }
        return nullptr;
    }
    void add_item(const char * p_name,unsigned p_name_length,const char * p_value,unsigned p_value_length)
    {
        auto  var = new global_variable(p_name,p_name_length,p_value,p_value_length);
        pfc::ptr_list_t<global_variable>::add_item(var);
    }
    ~global_variable_list() {delete_all();}
};

template <bool set = true, bool get = true>
class titleformat_hook_set_global : public titleformat_hook
{
    global_variable_list & p_vars;
    bool b_legacy;
public:
    bool process_field(titleformat_text_out * p_out,const char * p_name,unsigned p_name_length,bool & p_found_flag) override;
    bool process_function(titleformat_text_out * p_out,const char * p_name,unsigned p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag) override;
    inline titleformat_hook_set_global(global_variable_list & vars, bool legacy = false) : p_vars(vars), b_legacy(legacy)
    {
    };
};

class titleformat_hook_date : public titleformat_hook
{
    const SYSTEMTIME * p_st;
    pfc::array_t<char> year,month,day,dayofweek,hour,julian;
public:
    bool process_field(titleformat_text_out * p_out,const char * p_name,unsigned p_name_length,bool & p_found_flag) override;
    bool process_function(titleformat_text_out * p_out,const char * p_name,unsigned p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag) override;
    inline titleformat_hook_date(const SYSTEMTIME * st = nullptr) : p_st(st)
    {
    };
};

class titleformat_hook_splitter_pt3 : public titleformat_hook
{
public:
    inline titleformat_hook_splitter_pt3(titleformat_hook * p_hook1,titleformat_hook * p_hook2,titleformat_hook * p_hook3,titleformat_hook * p_hook4 = nullptr) : m_hook1(p_hook1), m_hook2(p_hook2), m_hook3(p_hook3), m_hook4(p_hook4) {};
    bool process_field(titleformat_text_out * p_out,const char * p_name,unsigned p_name_length,bool & p_found_flag) override;
    bool process_function(titleformat_text_out * p_out,const char * p_name,unsigned p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag) override;
private:
    titleformat_hook * m_hook1, * m_hook2, * m_hook3, * m_hook4;
};

class playlist_cache : private     pfc::ptr_list_t<playlist_entry_ui>
{
public:
    inline void add_item(int idx)
    {
        insert_item((playlist_entry_ui*)nullptr, idx);
    }

    void delete_all();

    inline bool get_active_columns_valid()
    {
        return m_active_columns_valid;
    }

    playlist_cache() : m_active_columns(0), m_active_columns_valid(false),
        m_sorted(false),m_sorted_column(0),m_sorted_descending(false)
    {};

    ~playlist_cache();

private:
    bool is_valid(unsigned idx);

    pfc::bit_array_bittable m_active_columns;
    bool m_active_columns_valid;

    bool m_sorted;
    unsigned m_sorted_column;
    bool m_sorted_descending;

    friend class playlist_view_cache;
};

class playlist_view_cache : private ptr_list_autodel_t<playlist_cache>
{
public:
    void on_items_added(unsigned p_playlist,unsigned start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const pfc::bit_array & p_selection);
    void on_items_reordered(unsigned p_playlist,const unsigned * order,unsigned count);
    void on_items_removed(unsigned p_playlist,const pfc::bit_array & mask);
    void on_items_modified(unsigned p_playlist,const pfc::bit_array & p_mask);
    void on_items_modified_fromplayback(unsigned p_playlist,const pfc::bit_array & p_mask,play_control::t_display_level p_level);
    void on_items_change(unsigned p_playlist,const pfc::bit_array & p_mask);
    inline void on_items_replaced(unsigned p_playlist,const pfc::bit_array & p_mask,const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry> & p_data)
    {
        on_items_modified(p_playlist, p_mask);
    }
    inline void on_playlist_activate(unsigned p_old,unsigned p_new)
    {
        active_playlist = p_new;
    }
    inline void on_playlist_created(unsigned p_index,const char * p_name,unsigned p_name_len)
    {
        insert_item(new playlist_cache, p_index);
#if 0
        unsigned n , count=get_count();
        if (p_index != count -1)
        for (n=p_index+1;n<count;n++)
            flush(n);
#endif
        active_playlist = static_api_ptr_t<playlist_manager>()->get_active_playlist();
    }
    inline void on_playlists_reorder(const unsigned * p_order,unsigned p_count)
    {
        unsigned old_active = active_playlist;
        unsigned n;
#if 0
        pfc::string8 temp;
        for (n=0; n<p_count; n++)
        {
            temp.add_int( p_order[n] );
            temp += " ";
        }
        console::print(temp);
#endif
        for (n=0; n<p_count; n++)
        {
            if (p_order[n]==old_active) active_playlist = n;
#if 0
            if (n!= p_order[n])
                flush(n);
#endif
        }
        reorder(p_order);
    }
    void on_playlists_removing(const pfc::bit_array & p_mask,unsigned p_old_count,unsigned p_new_count);
    void on_playlists_removed(const pfc::bit_array & p_mask,unsigned p_old_count,unsigned p_new_count)
    {
#if 0
        unsigned n, first=0;
        bool blah;
        for (n=0; n<p_old_count; n++)
            if (p_mask[n])
            {
                blah = true;
                first = n;
            }
        if (blah)
        {
            for (n=p_old_count; n>first; n--)
            {
                if (!p_mask[n-1]) flush(n-1);
            }
        }
#endif
        delete_mask(p_mask);
        active_playlist = static_api_ptr_t<playlist_manager>()->get_active_playlist();
    }
    void on_playlist_renamed(unsigned p_index,const char * p_new_name,unsigned p_new_name_len);

    inline void active_make_extra(unsigned idx, global_variable_list & p_out, const SYSTEMTIME * st = nullptr, bool b_legacy = false) const
    {
        assert (active_playlist != pfc_infinite);
        return make_extra(active_playlist, idx, p_out, st, b_legacy);
    }
    void make_extra(unsigned playlist, unsigned idx, global_variable_list & p_out, const SYSTEMTIME * st = nullptr, bool b_legacy = false) const;

    void get_display_name(unsigned playlist, unsigned idx, int col, pfc::string_base & out);
    COLORREF get_colour(unsigned playlist, unsigned idx, int col, colour_type colour);
    void get_colour(unsigned playlist, unsigned idx, int col, colourinfo & out);
    
    inline void active_get_display_name(unsigned idx, int col, pfc::string_base & out)
    {
        assert (active_playlist != pfc_infinite);
        get_display_name(active_playlist, idx, col, out);
    }
    inline COLORREF active_get_colour(unsigned idx, int col, colour_type colour)
    {
        assert (active_playlist != pfc_infinite);
        return get_colour(active_playlist, idx, col, colour);
    }
    inline void active_get_colour(unsigned idx, int col, colourinfo & out)
    {
        assert (active_playlist != pfc_infinite);
        get_colour(active_playlist, idx, col, out);
    }

    inline unsigned get_active_playlist()
    {
        return active_playlist;
    }

    inline unsigned playlist_get_count(unsigned p_playlist)
    {
        return get_item(p_playlist)->get_count();
    }

    inline unsigned activeplaylist_get_count()
    {
        if (active_playlist < get_count())
            return get_item(active_playlist)->get_count();
        return 0;
    }

    const pfc::bit_array & get_columns_mask(unsigned playlist);

    inline const pfc::bit_array & active_get_columns_mask()
    {
        static pfc::bit_array_false bt_false;
        if (is_active_playlist_valid())
            return get_columns_mask(active_playlist);
        return bt_false;
    }

    unsigned column_get_active_count(unsigned playlist);

    inline unsigned active_column_get_active_count()
    {
        if (is_active_playlist_valid())
            return column_get_active_count(active_playlist);
        return 0;
    }
    unsigned column_active_to_actual(unsigned playlist, unsigned column);
    inline unsigned active_column_active_to_actual(unsigned column)
    {
        assert (active_playlist != pfc_infinite);
        return column_active_to_actual(active_playlist, column);
    }


    void disable();
    void enable();
    void rebuild();

    void flush_all(bool flush_columns = true);
    void flush(unsigned playlist);

    void flush_columns(unsigned playlist);
    void flush_sort(unsigned playlist);

    bool get_playlist_sort(unsigned playlist, unsigned & idx, bool * p_descending = nullptr);
    void set_playlist_sort(unsigned playlist, unsigned column, bool descending);

    inline bool is_active_playlist_valid()
    {
        return active_playlist != pfc_infinite;
    }

    inline void active_set_playlist_sort(unsigned column, bool descending)
    {
        assert (is_active_playlist_valid());
            set_playlist_sort(active_playlist, column, descending);
//        else console::error("No active playlist!");
    }

    inline bool active_get_playlist_sort(unsigned & idx, bool * p_descending = nullptr)
    {
        if (is_active_playlist_valid())
            return get_playlist_sort(active_playlist, idx, p_descending);
        return false;
    }


    inline playlist_view_cache() : active_playlist(0),m_active(false) {};

    inline bool is_active()
    {
        return m_active;
    }
    
private:
/*    inline bool active_is_valid(unsigned idx)
    {
        return get_item(active_playlist)->is_valid(idx);
    }
    inline bool active_is_in_playlist(unsigned idx)
    {
        return is_in_playlist(active_playlist, idx);
    }*/
    void update_active_columns(unsigned playlist);
    bool is_in_playlist(unsigned playlist, unsigned idx);
    bool update_item(unsigned playlist, unsigned idx);

    unsigned active_playlist;
    bool m_active;
};

class t_local_cache : private playlist_callback
{
public:
    class t_local_cache_entry
    {
    public:
        inline bool get_last_position(unsigned & p_out)
        {
            p_out = m_last_position;
            return m_last_position_valid;
        }

        inline void set_last_position(unsigned u_position)
        {
            m_last_position_valid = true;
            m_last_position = u_position;
        }

        t_local_cache_entry()
            : m_last_position_valid(false), m_last_position(0)
        {};

    private:
        bool m_last_position_valid;
        unsigned m_last_position;
    };

    bool get_entry(unsigned index, t_local_cache_entry * & p_out)
    {
        if (index < m_entries.get_count())
        {
            p_out = &m_entries[index];
            return true;
        }
        return false;
    }
    void initialise()
    {
        static_api_ptr_t<playlist_manager> playlist_api;
        unsigned count = playlist_api->get_playlist_count();
        for (;count;count--)
            m_entries.add_item(t_local_cache_entry());
        playlist_api->register_callback(this, playlist_callback::flag_on_playlist_created|playlist_callback::flag_on_playlists_removed|playlist_callback::flag_on_playlists_reorder);
    }
    void deinitialise()
    {
        static_api_ptr_t<playlist_manager> playlist_api;
        playlist_api->unregister_callback(this);
        m_entries.remove_all();
    }
private:
    void FB2KAPI on_items_added(unsigned p_playlist,unsigned start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const pfc::bit_array & p_selection) override {};

    void FB2KAPI on_items_reordered(unsigned p_playlist,const unsigned * order,unsigned count) override {};

    void FB2KAPI on_items_removing(unsigned p_playlist,const pfc::bit_array & p_mask,unsigned p_old_count,unsigned p_new_count) override {};//called before actually removing them
    void FB2KAPI on_items_removed(unsigned p_playlist,const pfc::bit_array & p_mask,unsigned p_old_count,unsigned p_new_count) override {};

    void FB2KAPI on_items_selection_change(unsigned p_playlist,const pfc::bit_array & affected,const pfc::bit_array & state) override {};

    void FB2KAPI on_item_focus_change(unsigned p_playlist,unsigned from,unsigned to) override {};//focus may be -1 when no item has focus; reminder: focus may also change on other callbacks
    void FB2KAPI on_items_modified(unsigned p_playlist,const pfc::bit_array & p_mask) override {};

    void FB2KAPI on_items_modified_fromplayback(unsigned p_playlist,const pfc::bit_array & p_mask,play_control::t_display_level p_level) override {};

    void FB2KAPI on_items_replaced(unsigned p_playlist,const pfc::bit_array & p_mask,const pfc::list_base_const_t<t_on_items_replaced_entry> & p_data) override {};

    void FB2KAPI on_item_ensure_visible(unsigned p_playlist,unsigned idx) override {};

    void FB2KAPI on_playlist_activate(unsigned p_old,unsigned p_new) override {};

    void on_playlist_created(unsigned p_index,const char * p_name,unsigned p_name_len) override
    {
        m_entries.insert_item(t_local_cache_entry(), p_index);
    };

    void on_playlists_reorder(const unsigned * p_order, unsigned p_count) override
    {
        if (p_count == m_entries.get_count())
            m_entries.reorder(p_order);
    };

    void on_playlists_removing(const pfc::bit_array & p_mask, unsigned p_old_count, unsigned p_new_count) override {};

    void on_playlists_removed(const pfc::bit_array & p_mask, unsigned p_old_count, unsigned p_new_count) override
    {
        m_entries.remove_mask(p_mask);
    }

    void on_playlist_renamed(unsigned p_index,const char * p_new_name,unsigned p_new_name_len) override {};

    void on_default_format_changed() override {};

    void on_playback_order_changed(unsigned p_new_index) override {};

    void on_playlist_locked(unsigned p_playlist,bool p_locked) override {};

    pfc::list_t<t_local_cache_entry> m_entries;
};

#endif