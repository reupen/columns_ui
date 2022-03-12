#pragma once

namespace playlist_manager_utils {
void rename_playlist(size_t index, HWND wnd_parent);

bool check_clipboard();
bool cut(const pfc::list_base_const_t<size_t>& indices);
bool copy(const pfc::list_base_const_t<size_t>& indices);
bool cut(const bit_array& mask);
bool copy(const bit_array& mask);
bool paste(HWND wnd, size_t index_insert);
} // namespace playlist_manager_utils
