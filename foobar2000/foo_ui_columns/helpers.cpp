#include "foo_ui_columns.h"

void g_ui_selection_manager_register_callback_no_now_playing_fallback(ui_selection_callback * p_callback)
{
	if (static_api_test_t<ui_selection_manager_v2>())
		static_api_ptr_t<ui_selection_manager_v2>()->register_callback(p_callback, ui_selection_manager_v2::flag_no_now_playing);
	else
		static_api_ptr_t<ui_selection_manager>()->register_callback(p_callback);
}

bool g_ui_selection_manager_is_now_playing_fallback()
{
	if (static_api_test_t<ui_selection_manager_v2>())
		return false;
	else
		return static_api_ptr_t<ui_selection_manager>()->get_selection_type() == contextmenu_item::caller_now_playing;
}