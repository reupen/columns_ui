#pragma once
#include "menu_items.h"
#include "layout.h"
#include "ng_playlist/ng_playlist.h"

namespace cui::button_items {

template <class ButtonArgs>
class PushButton : public uie::button {
public:
    static void s_on_change()
    {
        for (auto&& button : m_buttons) {
            for (auto&& callback : button->m_callbacks) {
                callback->on_button_state_change(s_get_button_state());
            }
        }
    }
    static unsigned s_get_button_state()
    {
        return (ButtonArgs::state() ? uie::BUTTON_STATE_PRESSED : 0) | uie::BUTTON_STATE_DEFAULT;
    }
    PushButton() { m_buttons.emplace_back(this); }
    ~PushButton() { m_buttons.erase(std::remove(m_buttons.begin(), m_buttons.end(), this), m_buttons.end()); }

private:
    const GUID& get_item_guid() const override { return ButtonArgs::id; }
    HBITMAP get_item_bitmap(unsigned, COLORREF, uie::t_mask&, COLORREF&, HBITMAP&) const override { return nullptr; }
    unsigned get_button_state() const override { return s_get_button_state(); }
    void register_callback(uie::button_callback& p_callback) override { m_callbacks.emplace_back(&p_callback); }
    void deregister_callback(uie::button_callback& p_callback) override
    {
        m_callbacks.erase(std::remove(m_callbacks.begin(), m_callbacks.end(), &p_callback), m_callbacks.end());
    }

    static std::vector<PushButton<ButtonArgs>*> m_buttons;
    std::vector<uie::button_callback*> m_callbacks;
};

template <class ButtonArgs>
std::vector<PushButton<ButtonArgs>*> PushButton<ButtonArgs>::m_buttons;

struct LiveLayoutEditingButtonArgs {
    static bool state() { return g_layout_window.get_layout_editing_active(); }
    static constexpr GUID id = main_menu::commands::toggle_live_editing_id;
};

using LiveLayoutEditingButton = PushButton<LiveLayoutEditingButtonArgs>;

struct ShowGroupsButtonArgs {
    static bool state() { return pvt::cfg_grouping; }
    static constexpr GUID id = main_menu::commands::show_groups_id;
};

using ShowGroupsButton = PushButton<ShowGroupsButtonArgs>;

struct ShowArtworkButtonArgs {
    static bool state() { return pvt::cfg_show_artwork; }
    static constexpr GUID id = main_menu::commands::show_artwork_id;
};

using ShowArtworkButton = PushButton<ShowArtworkButtonArgs>;

} // namespace cui::button_items
