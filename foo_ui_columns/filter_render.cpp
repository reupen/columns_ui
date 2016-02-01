#include "stdafx.h"

namespace filter_panel {

	void filter_panel_t::render_background(HDC dc, const RECT * rc)
	{
		cui::colours::helper p_helper(appearance_client_filter_impl::g_guid);
		gdi_object_t<HBRUSH>::ptr_t br = CreateSolidBrush(p_helper.get_colour(cui::colours::colour_background));
		FillRect(dc, rc, br);
	}

	void filter_panel_t::render_item(HDC dc, t_size index, t_size indentation, bool b_selected, bool b_window_focused, bool b_highlight, bool b_focused, const RECT * rc)
	{
		cui::colours::helper p_helper(appearance_client_filter_impl::g_guid);
		const t_item * item = get_item(index);
		int theme_state = NULL;
		if (b_selected)
			theme_state = (b_highlight ? LISS_HOTSELECTED : (b_window_focused ? LISS_SELECTED : LISS_SELECTEDNOTFOCUS));
		else if (b_highlight)
			theme_state = LISS_HOT;

		bool b_theme_enabled = p_helper.get_themed();
		//NB Third param of IsThemePartDefined "must be 0". But this works.
		bool b_themed = b_theme_enabled && get_theme() && IsThemePartDefined(get_theme(), LVP_LISTITEM, theme_state);

		COLORREF cr_text = NULL;
		if (b_themed && theme_state)
		{
			cr_text = GetThemeSysColor(get_theme(), b_selected ? COLOR_BTNTEXT : COLOR_WINDOWTEXT);;
			{
				if (IsThemeBackgroundPartiallyTransparent(get_theme(), LVP_LISTITEM, theme_state))
					DrawThemeParentBackground(get_wnd(), dc, rc);
				DrawThemeBackground(get_theme(), dc, LVP_LISTITEM, theme_state, rc, NULL);
			}
		}

		RECT rc_subitem = *rc;
		t_size k, countk = get_column_count();

		for (k = 0; k < countk; k++)
		{
			rc_subitem.right = rc_subitem.left + get_column_display_width(k);

			if (!(b_themed && theme_state))
			{
				if (b_selected)
					cr_text = !b_window_focused ? p_helper.get_colour(cui::colours::colour_inactive_selection_text) : p_helper.get_colour(cui::colours::colour_selection_text);
				else
					cr_text = p_helper.get_colour(cui::colours::colour_text);

				COLORREF cr_back;
				if (b_selected)
					cr_back = !b_window_focused ? p_helper.get_colour(cui::colours::colour_inactive_selection_background) : p_helper.get_colour(cui::colours::colour_selection_background);
				else
					cr_back = p_helper.get_colour(cui::colours::colour_background);

				FillRect(dc, &rc_subitem, gdi_object_t<HBRUSH>::ptr_t(CreateSolidBrush(cr_back)));
			}
			ui_helpers::text_out_colours_tab(dc, get_item_text(index, k), strlen(get_item_text(index, k)), 1 + (k == 0 ? indentation : 0), 3, &rc_subitem, b_selected, cr_text, true, true, (cfg_ellipsis != 0), get_column_alignment(k));

			rc_subitem.left = rc_subitem.right;
		}
		if (b_focused)
		{
			RECT rc_focus = *rc;
			if (b_theme_enabled && get_theme() && IsThemePartDefined(get_theme(), LVP_LISTITEM, LISS_SELECTED))
				InflateRect(&rc_focus, -1, -1);
			if (!p_helper.get_bool(cui::colours::bool_use_custom_active_item_frame))
			{
				DrawFocusRect(dc, &rc_focus);
			}
			else
			{
				gdi_object_t<HBRUSH>::ptr_t br = CreateSolidBrush(p_helper.get_colour(cui::colours::colour_active_item_frame));
				FrameRect(dc, &rc_focus, br);
			}
		}
	}


}