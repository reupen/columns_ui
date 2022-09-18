#pragma once

namespace cui::helpers {

class CoreDarkListView : public uih::ListView {
    class UIConfigCallback : public ui_config_callback {
    public:
        UIConfigCallback(CoreDarkListView* list_view, ui_config_manager::ptr manager)
            : m_list_view(list_view)
            , m_manager(std::move(manager))
        {
            m_manager->add_callback(this);
        }

        ~UIConfigCallback() { m_manager->remove_callback(this); }

        void ui_fonts_changed() override {}

        void ui_colors_changed() override { m_list_view->set_use_dark_mode(m_manager->is_dark_mode()); }

    private:
        CoreDarkListView* m_list_view{};
        ui_config_manager::ptr m_manager;
    };

    void notify_on_initialisation() override;
    void notify_on_destroy() override;
    void render_get_colour_data(ColourData& p_out) override;

    std::unique_ptr<UIConfigCallback> m_ui_config_callback;
};

} // namespace cui::helpers
