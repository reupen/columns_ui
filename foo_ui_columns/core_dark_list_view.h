#pragma once
#include "dark_mode_active_ui.h"

namespace cui::helpers {

class CoreDarkListView : public uih::ListView {
public:
    CoreDarkListView(bool allow_cui_fallback = false) : m_allow_cui_fallback(allow_cui_fallback) {}

protected:
    void notify_on_initialisation() override;
    void notify_on_destroy() override;
    void render_get_colour_data(ColourData& p_out) override;

private:
    mmh::EventToken::Ptr m_dark_mode_status_callback;
    bool m_allow_cui_fallback{};
};

} // namespace cui::helpers
