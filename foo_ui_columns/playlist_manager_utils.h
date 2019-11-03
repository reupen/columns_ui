#pragma once

namespace playlist_manager_utils {
void rename_playlist(size_t index, HWND wnd_parent);

bool check_clipboard();
bool cut(const pfc::list_base_const_t<t_size>& indices);
bool copy(const pfc::list_base_const_t<t_size>& indices);
bool cut(const pfc::bit_array& mask);
bool copy(const pfc::bit_array& mask);
bool paste(HWND wnd, t_size index_insert);
} // namespace playlist_manager_utils
