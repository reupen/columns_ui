#pragma once
#include "dark_mode_active_ui.h"

namespace cui::helpers {

class CoreDarkListView : public uih::ListView {
protected:
    void notify_on_initialisation() override;
    void notify_on_destroy() override;
    void render_get_colour_data(ColourData& p_out) override;

private:
    std::unique_ptr<EventToken> m_dark_mode_status_callback;
};

} // namespace cui::helpers
