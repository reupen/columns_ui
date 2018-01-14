#ifndef _SPLITTER_TABS_H_
#define _SPLITTER_TABS_H_

class splitter_window_tabs_impl :
    public uie::container_ui_extension_t<ui_helpers::container_window, uie::splitter_window_v2>
{
    typedef splitter_window_tabs_impl t_self;
public:
    class_data & get_class_data()const override;
    void get_name(pfc::string_base & p_out) const override;
    const GUID & get_extension_guid() const override;
    void get_category(pfc::string_base & p_out) const override;
    unsigned get_type  () const override;

    void insert_panel(unsigned index, const uie::splitter_item_t *  p_item) override;
    void remove_panel(unsigned index) override;
    void replace_panel(unsigned index, const uie::splitter_item_t *  p_item) override;

    bool is_point_ours(HWND wnd_point, const POINT & pt_screen, pfc::list_base_t<uie::window::ptr> & p_hierarchy) override
    {
        if (wnd_point == get_wnd() || IsChild(get_wnd(), wnd_point))
        {
            if (wnd_point == get_wnd() || wnd_point == m_wnd_tabs)
            {
                p_hierarchy.add_item(this);
                return true;
            }
            else
            {
                t_size i, count = m_panels.get_count();
                for (i=0; i<count; i++)
                {
                    uie::splitter_window_v2_ptr sptr;
                    if (m_panels[i]->m_child.is_valid())
                    {
                        if (m_panels[i]->m_child->service_query_t(sptr))
                        {
                            pfc::list_t<uie::window::ptr> temp;
                            temp.add_item(this);
                            if (sptr->is_point_ours(wnd_point, pt_screen, temp))
                            {
                                p_hierarchy.add_items(temp);
                                return true;
                            }
                        }
                        else if (wnd_point == m_panels[i]->m_wnd || IsChild(m_panels[i]->m_wnd , wnd_point))
                        {
                            p_hierarchy.add_item(this);
                            p_hierarchy.add_item(m_panels[i]->m_child);
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    void get_supported_panels(const pfc::list_base_const_t<uie::window::ptr> & p_windows, pfc::bit_array_var & p_mask_unsupported) override;
    unsigned get_panel_count()const override;
    uie::splitter_item_t * get_panel(unsigned index)const override;

    bool get_config_item_supported(unsigned index, const GUID & p_type) const override;
    bool get_config_item(unsigned index, const GUID & p_type, stream_writer * p_out, abort_callback & p_abort) const override;
    bool set_config_item(unsigned index, const GUID & p_type, stream_reader * p_source, abort_callback & p_abort) override;

    enum {stream_version_current=0};

    void set_config(stream_reader * config, t_size p_size, abort_callback & p_abort) override;
    void get_config(stream_writer * out, abort_callback & p_abort) const override;

    void export_config(stream_writer * p_writer, abort_callback & p_abort) const override;

    void import_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort) override;

    class splitter_host_impl;

    struct t_size_limit
    {
        unsigned min_height;
        unsigned max_height;
        unsigned min_width;
        unsigned max_width;
        t_size_limit()
            : min_height(0), max_height(0), min_width(0), max_width(0)
        {};
    };
    class panel : 
        public pfc::refcounted_object_root
    {
    public:
        GUID m_guid;
        HWND m_wnd;
        pfc::array_t<t_uint8> m_child_data;
        t_size_limit m_size_limits;
        uie::window_ptr m_child;
        bool m_use_custom_title;
        pfc::string8 m_custom_title;
        service_ptr_t<splitter_window_tabs_impl> m_this;
        void set_splitter_window_ptr(splitter_window_tabs_impl * ptr)
        {
            m_this = ptr;
        }

        panel();
        ~panel() override;

        uie::splitter_item_full_v2_t * create_splitter_item();

        void set_from_splitter_item(const uie::splitter_item_t * p_source);

        void destroy();

        void read(stream_reader*t, abort_callback & p_abort);

        void write(stream_writer * out, abort_callback & p_abort);
        void _export(stream_writer * out, abort_callback & p_abort);
        void import(stream_reader*t, abort_callback & p_abort);

        service_ptr_t<class splitter_host_impl> m_interface;
    };

    class panel_list : public pfc::list_t<pfc::refcounted_object_ptr_t<panel> >
    {
    public:
        //bool move_up(unsigned idx);
        //bool move_down(unsigned idx);
        bool find_by_wnd(HWND wnd, unsigned & p_out);
    };

    LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp) override;
    LRESULT WINAPI on_hooked_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
    static LRESULT WINAPI g_hook_proc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
    WNDPROC m_tab_proc;

    void create_tabs();
    void destroy_tabs();
    void refresh_children();
    void destroy_children();
    void adjust_rect(bool b_larger, RECT * rc);
    void set_styles(bool visible = true);
    panel_list m_panels;
    panel_list m_active_panels;
    HWND m_wnd_tabs;
    t_size m_active_tab;
    static std::vector<service_ptr_t<t_self>> g_windows;
    uie::size_limit_t m_size_limits;
    t_int32 m_mousewheel_delta;


    void update_size_limits();
    void on_font_change();
    static void g_on_font_change();
    gdi_object_t<HFONT>::ptr_t g_font;
    void on_size_changed(unsigned width, unsigned height);
    void on_size_changed();
    void on_active_tab_changing(t_size index_from);
    void on_active_tab_changed(t_size index_to);

    splitter_window_tabs_impl();
};

#endif