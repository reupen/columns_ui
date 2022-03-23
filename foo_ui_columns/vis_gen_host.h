#pragma once

class VisualisationPanel : public uie::container_uie_window_v3 {
    static const wchar_t* class_name;
    bool initialised{false};
    pfc::array_t<uint8_t> m_data;
    service_ptr_t<class VisualisationPanelInterface> m_interface;
    uie::visualisation_ptr p_vis;
    unsigned m_frame;
    HBITMAP bm_display{nullptr};
    RECT rc_client{};
    HWND m_wnd{nullptr};

public:
    HBITMAP get_bitmap() const { return bm_display; }
    const RECT* get_rect_client() { return &rc_client; }
    static pfc::ptr_list_t<VisualisationPanel> list_vis;

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;

    uie::container_window_v3_config get_window_config() override
    {
        uie::container_window_v3_config config(class_name, false);

        if (m_frame == 1)
            config.extended_window_styles |= WS_EX_CLIENTEDGE;
        else if (m_frame == 2)
            config.extended_window_styles |= WS_EX_STATICEDGE;

        return config;
    }

    VisualisationPanel();

    ~VisualisationPanel();

    void flush_bitmap();
    void make_bitmap(HDC hdc = nullptr);

    void get_name(pfc::string_base& out) const override;
    void get_category(pfc::string_base& out) const override;

    void set_frame_style(unsigned p_type);
    unsigned get_frame_style() const { return m_frame; }
    void get_vis_ptr(uie::visualisation_ptr& p_out) { p_out = p_vis; }

    unsigned get_type() const override { return ui_extension::type_toolbar | ui_extension::type_panel; }

    void set_vis_data(const void* p_data, size_t p_size)
    {
        m_data.set_size(0);
        m_data.append_fromptr((uint8_t*)p_data, p_size);
    }
    void get_vis_data(pfc::array_t<uint8_t>& p_out) const
    {
        if (p_vis.is_valid()) {
            p_out.set_size(0);
            abort_callback_dummy abortCallback;
            p_vis->get_config_to_array(p_out, abortCallback);
        }
    }
    friend class VisualisationPanelInterface;

    // override me

    // (leave default definitions in place for these three funcs)
    // virtual void get_menu_items (ui_extension::menu_hook_t & p_hook);
    // virtual void set_config ( stream_reader * config);
    // virtual void get_config( stream_writer * data);
    // virtual bool have_config_popup(){return true;}
    // virtual bool show_config_popup(HWND wnd_parent);

    virtual const GUID& get_visualisation_guid() const = 0;
};
