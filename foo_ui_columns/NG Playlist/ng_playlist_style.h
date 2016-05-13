#pragma once

namespace pvt 
{
	class style_data_cell_info_t
	{
	public:
		colour text_colour;
		colour selected_text_colour;
		colour background_colour;
		colour selected_background_colour;
		colour selected_text_colour_non_focus;
		colour selected_background_colour_non_focus;
		colour frame_left;
		colour frame_top;
		colour frame_right;
		colour frame_bottom;
		bool use_frame_left : 1;
		bool use_frame_top : 1;
		bool use_frame_right : 1;
		bool use_frame_bottom : 1;

		static style_data_cell_info_t g_create_default();

		inline void set (const style_data_cell_info_t * in)
		{
			text_colour = in->text_colour;
			selected_text_colour = in->selected_text_colour;
			background_colour = in->background_colour;
			selected_background_colour = in->selected_background_colour;
			selected_text_colour_non_focus = in->selected_text_colour_non_focus;
			selected_background_colour_non_focus = in->selected_background_colour_non_focus;
			frame_left = in->frame_left;
			frame_top = in->frame_top;
			frame_right = in->frame_right;
			frame_bottom = in->frame_bottom; 
			use_frame_left = in->use_frame_left;
			use_frame_top = in->use_frame_top;
			use_frame_right = in->use_frame_right;
			use_frame_bottom = in->use_frame_bottom;
		}

		style_data_cell_info_t() : use_frame_left(false), use_frame_top(false), use_frame_right(false), use_frame_bottom(false) {};

		style_data_cell_info_t(COLORREF text, COLORREF text_sel, COLORREF back, COLORREF back_sel, COLORREF text_no_focus, COLORREF sel_no_focus) 
			: use_frame_left(false), use_frame_top(false), use_frame_right(false), use_frame_bottom(false)
		{
			text_colour.set(text);
			selected_text_colour.set(text_sel);
			background_colour.set(back);
			selected_background_colour.set(back_sel);
			selected_text_colour_non_focus.set(text_no_focus);
			selected_background_colour_non_focus.set(sel_no_focus);
		}

		bool is_equal (const style_data_cell_info_t & c2)
		{
			const style_data_cell_info_t & c1 = *this;
			return (c1.text_colour == c2.text_colour && 
				c1.selected_text_colour == c2.selected_text_colour && 
				c1.background_colour == c2.background_colour && 
				c1.selected_background_colour == c2.selected_background_colour &&
				c1.selected_text_colour_non_focus == c2.selected_text_colour_non_focus &&
				c1.selected_background_colour_non_focus == c2.selected_background_colour_non_focus &&
				c1.use_frame_left == c2.use_frame_left &&
				c1.use_frame_right == c2.use_frame_right &&
				c1.use_frame_bottom == c2.use_frame_bottom &&
				c1.use_frame_top == c2.use_frame_top &&
				(c1.use_frame_left ? c1.frame_left == c2.frame_left : true) &&
				(c1.use_frame_bottom ? c1.frame_bottom == c2.frame_bottom : true) &&
				(c1.use_frame_top ? c1.frame_top == c2.frame_top : true) &&
				(c1.use_frame_right ? c1.frame_right == c2.frame_right : true)
				);
		}
	};
	class style_data_cell_t : public pfc::refcounted_object_root, public style_data_cell_info_t
	{
	public:
		typedef style_data_cell_t self_t;
		typedef pfc::refcounted_object_ptr_t<self_t> ptr;

		/*inline void set (const style_data_cell_t::ptr & in)
		{
			text_colour = in->text_colour;
			selected_text_colour = in->selected_text_colour;
			background_colour = in->background_colour;
			selected_background_colour = in->selected_background_colour;
			selected_text_colour_non_focus = in->selected_text_colour_non_focus;
			selected_background_colour_non_focus = in->selected_background_colour_non_focus;
			frame_left = in->frame_left;
			frame_top = in->frame_top;
			frame_right = in->frame_right;
			frame_bottom = in->frame_bottom; 
			use_frame_left = in->use_frame_left;
			use_frame_top = in->use_frame_top;
			use_frame_right = in->use_frame_right;
			use_frame_bottom = in->use_frame_bottom;
		}
		style_data_cell_t(COLORREF text, COLORREF text_sel, COLORREF back, COLORREF back_sel, COLORREF text_no_focus, COLORREF sel_no_focus)// : use_frame_left(false), use_frame_top(false), use_frame_bottom(false), use_frame_right(false)
		{
			text_colour.set(text);
			selected_text_colour.set(text_sel);
			background_colour.set(back);
			selected_background_colour.set(back_sel);
			selected_text_colour_non_focus.set(text_no_focus);
			selected_background_colour_non_focus.set(sel_no_focus);
		}
		style_data_cell_t(const style_data_cell_t::ptr & in)
		{
			set(in);
		}*/
		style_data_cell_t(const style_data_cell_info_t & in) : style_data_cell_info_t(in)
		{
		}
		//style_data_cell_t() : use_frame_left(false), use_frame_top(false), use_frame_bottom(false), use_frame_right(false) {};


		~style_data_cell_t();
	};

	namespace style_cache_manager
	{
		void g_add_object(const style_data_cell_info_t & p_data, style_data_cell_t::ptr & p_out);
		void g_remove_object(style_data_cell_t * p_object);
	}
	typedef pfc::array_t<style_data_cell_t::ptr> style_data_t;

	class titleformat_hook_style_v2 : public titleformat_hook
	{
		style_data_cell_info_t p_default_colours;
		pfc::array_t<char> text,selected_text,back,selected_back,selected_back_no_focus,selected_text_no_focus, m_index_text;
		style_data_cell_info_t & p_colours;
		t_size m_index;
		bool m_is_group;
	public:
		virtual bool process_field(titleformat_text_out * p_out,const char * p_name,unsigned p_name_length,bool & p_found_flag);
		virtual bool process_function(titleformat_text_out * p_out,const char * p_name,unsigned p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag);
		inline titleformat_hook_style_v2(style_data_cell_info_t & vars, t_size index, bool b_is_group = false) : p_default_colours(vars), p_colours(vars), m_index(index), m_is_group(b_is_group)
		{
		};
	};
}