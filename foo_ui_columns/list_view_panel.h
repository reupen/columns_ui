#pragma once

template<typename t_appearance_client>
class t_list_view_panel : public t_list_view {
protected:
	virtual const char * get_drag_unit_plural() const override { return "tracks"; }
	virtual const char * get_drag_unit_singular() const override { return "track"; }
	virtual bool should_show_drag_text(t_size selection_count) override { return true; }
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
private:
};
