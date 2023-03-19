#pragma once

#include "config_appearance.h"
#include "core_dark_list_view.h"
#include "dark_mode_dialog.h"
#include "layout.h"

class QuickSetupDialog {
public:
    static void s_run();
    static void s_refresh();

private:
    class PresetsListView : public cui::helpers::CoreDarkListView {
    public:
        explicit PresetsListView(QuickSetupDialog* dialog) : CoreDarkListView(true), m_dialog(*dialog) {}

        void notify_on_initialisation() override
        {
            CoreDarkListView::notify_on_initialisation();

            set_selection_mode(SelectionMode::SingleRelaxed);
            set_show_header(false);
            set_columns({{"Preset", 100}});
            set_autosize(true);
        }

        void notify_on_selection_change(const pfc::bit_array& p_affected, const pfc::bit_array& p_status,
            notification_source_t p_notification_source) override
        {
            m_dialog.on_preset_list_selection_change();
        }

    private:
        QuickSetupDialog& m_dialog;
    };

    inline static std::vector<QuickSetupDialog*> s_instances;

    INT_PTR handle_dialog_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    void on_preset_list_selection_change();

    PresetsListView m_presets_list_view{this};
    HWND m_wnd{};
    bool m_preset_changed{};
    pfc::list_t<ConfigLayout::Preset> m_presets;
    uie::splitter_item_ptr m_previous_layout;
    cui::colours::DarkModeStatus m_previous_mode{};
    cui::colours::ColourScheme m_previous_light_colour_scheme{columns_ui::colours::ColourSchemeThemed};
    cui::colours::ColourScheme m_previous_dark_colour_scheme{columns_ui::colours::ColourSchemeThemed};
    bool m_previous_show_artwork{};
    bool m_previous_show_grouping{};
};
