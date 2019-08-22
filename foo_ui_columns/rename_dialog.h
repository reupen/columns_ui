#pragma once

namespace cui::helpers {

std::optional<pfc::string8> show_rename_dialog_box(HWND wnd_parent, const char* title, const char* initial_text);

} // namespace cui::helpers
