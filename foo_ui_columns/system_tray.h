#pragma once

void create_icon_handle();
void create_systray_icon();
void update_systray(bool balloon = false, int btitle = 0, bool force_balloon = false);
void destroy_systray_icon();
void on_show_system_tray_icon_change();
