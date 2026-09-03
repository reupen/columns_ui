#pragma once

namespace cui::fcl {

void g_export_layout(HWND wnd, pfc::string8 path = {}, bool is_quiet = false);
void g_import_layout(HWND wnd);
void g_import_layout(HWND wnd, const char* path, bool quiet = false);

} // namespace cui::fcl
