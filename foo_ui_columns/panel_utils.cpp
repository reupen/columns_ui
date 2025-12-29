#include "pch.h"

#include "panel_utils.h"

namespace cui::panel_utils {

HWND create_or_transfer_window_safe(std::string_view host_name, const uie::window::ptr& window, HWND wnd_parent,
    const uie::window_host::ptr& host, const ui_helpers::window_position_t& position, std::string_view panel_noun)
{
    try {
        return window->create_or_transfer_window(wnd_parent, host, position);
    } catch (std::exception& ex) {
        pfc::string8 panel_name;
        window->get_name(panel_name);
        console::print(fmt::format("{} – {} {} threw unexpected exception during window creation: {}", host_name,
            panel_noun, panel_name.c_str(), ex.what())
                .c_str());
        return nullptr;
    }
}

} // namespace cui::panel_utils
