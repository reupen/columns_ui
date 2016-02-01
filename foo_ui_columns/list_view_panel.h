#pragma once

template<typename t_appearance_client>
class t_list_view_panel : public t_list_view {
protected:
	virtual const char * get_drag_unit_plural() const override { return "tracks"; }
	virtual void render_get_colour_data(colour_data_t & p_out) override
	{
		cui::colours::helper p_helper(t_appearance_client::g_guid);
		p_out.m_themed = p_helper.get_themed();
		p_out.m_use_custom_active_item_frame = p_helper.get_bool(cui::colours::bool_use_custom_active_item_frame);
		p_out.m_text = p_helper.get_colour(cui::colours::colour_text);
		p_out.m_selection_text = p_helper.get_colour(cui::colours::colour_selection_text);
		p_out.m_background = p_helper.get_colour(cui::colours::colour_background);
		p_out.m_selection_background = p_helper.get_colour(cui::colours::colour_selection_background);
		p_out.m_inactive_selection_text = p_helper.get_colour(cui::colours::colour_inactive_selection_text);
		p_out.m_inactive_selection_background = p_helper.get_colour(cui::colours::colour_inactive_selection_background);
		p_out.m_active_item_frame = p_helper.get_colour(cui::colours::colour_active_item_frame);
		if (!p_out.m_themed || !get_group_text_colour_default(p_out.m_group_text))
			p_out.m_group_text = p_out.m_text;
		p_out.m_group_background = p_out.m_background;
	}
	void render_drag_image_icon(HDC dc, const RECT & rc) override
	{
		// Perhaps cache?
		// Load 256x256 icon because otherwise Windows helpfully picks a low-res version
		icon_ptr icon((static_api_ptr_t<ui_control>()->load_main_icon(256, 256)));
		// Ideally, we would show artwork, but this will do for now.
		// We may want to use better scaling too.
		DrawIconEx(dc, 0, 0, icon, RECT_CX(rc), RECT_CY(rc), NULL, NULL, DI_NORMAL);
	}
private:
};
