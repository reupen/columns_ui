#include "stdafx.h"

bool titleformat_hook_impl_file_info_branch::process_field(titleformat_text_out * p_out, const char * p_name, t_size p_name_length, bool & p_found_flag)
{
	if (p_name_length > 2 && p_name[0] == '<')
	{
		p_name_length = strlen_max(p_name, p_name_length);
		if (p_name[p_name_length - 1] == '>')
		{
			t_size index;
			if (remap_meta(index, p_name + 1, p_name_length - 2))
			{
				p_found_flag = process_meta_branch(p_out, index);
			}
			else p_found_flag = false;
			if (!p_found_flag) p_out->write(titleformat_inputtypes::meta, "?", 1);
			return true;
		}
	}
	return baseClass::process_field(p_out, p_name, p_name_length, p_found_flag);
}

bool titleformat_hook_impl_file_info_branch::process_function(titleformat_text_out * p_out, const char * p_name, t_size p_name_length, titleformat_hook_function_params * p_params, bool & p_found_flag)
{
	bool strip = false, remapswap = false, remapstrip = false;
	if (!stricmp_utf8_ex(p_name, p_name_length, "meta_branch", pfc_infinite))
	{
		if (p_params->get_param_count() != 1) return false;
		const char * name;
		t_size name_length;
		p_params->get_param(0, name, name_length);
		t_size index = m_info->meta_find_ex(name, name_length);
		p_found_flag = process_meta_branch(p_out, index);
		return true;
	}
	else if (!stricmp_utf8_ex(p_name, p_name_length, "meta_branch_remap", pfc_infinite))
	{
		if (p_params->get_param_count() != 1) return false;
		const char * name;
		t_size name_length;
		p_params->get_param(0, name, name_length);
		t_size index;
		if (remap_meta(index, name, name_length))
		{
			p_found_flag = process_meta_branch(p_out, index);
		}
		else p_found_flag = false;
		return true;
	}
	else if (!stricmp_utf8_ex(p_name, p_name_length, "meta_branch_swapprefix", pfc_infinite)
		|| (strip = !stricmp_utf8_ex(p_name, p_name_length, "meta_branch_stripprefix", pfc_infinite))
		|| (remapstrip = !stricmp_utf8_ex(p_name, p_name_length, "meta_branch_remap_stripprefix", pfc_infinite))
		|| (remapswap = !stricmp_utf8_ex(p_name, p_name_length, "meta_branch_remap_swapprefix", pfc_infinite))
		)
	{
		strip = strip || remapstrip;
		bool remap = remapstrip || remapswap;

		t_size param_count = p_params->get_param_count();
		if (param_count < 1) return false;

		pfc::string_list_impl articles;
		const char * value;
		t_size value_length;
		p_params->get_param(0, value, value_length);
		if (param_count == 1)
		{
			articles.add_item("The ");
			articles.add_item("A ");
		}
		else
		{
			t_size i;
			for (i = 1; i<param_count; i++)
			{
				const char * article;
				t_size article_length;
				p_params->get_param(i, article, article_length);
				pfc::string8 art(article, article_length); art.add_byte(' ');
				articles.add_item(art);
			}
		}

		pfc::string_list_impl branches;
		const char * ptr = value;

		t_size index = pfc_infinite;
		if (remap)
		{
			if (!remap_meta(index, value, value_length)) index = pfc_infinite;
		}
		else
			index = m_info->meta_find_ex(value, value_length);

		p_found_flag = false;
		if (index != pfc_infinite)
		{

			t_size i, article_count = articles.get_count();

			t_size n, m = m_info->meta_enum_value_count(index);
			//bool found = false;
			p_out->write(titleformat_inputtypes::meta, "\4", 1);
			for (n = 0; n < m; n++)
			{
				if (n > 0)
				{
					p_out->write(titleformat_inputtypes::meta, "\5", 1);
				}
				const char * value = m_info->meta_enum_value(index, n);
				if (*value != 0) p_found_flag = true;

				pfc::string8 temp;

				for (i = 0; i<article_count; i++)
				{
					t_size len = strlen(articles[i]);
					if (!stricmp_utf8_max(value, articles[i], len))
					{
						temp << (value + len);
						if (!strip) temp << ", " << articles[i];
						value = temp;
						break;
					}
				}

				p_out->write(titleformat_inputtypes::meta, value, pfc_infinite);
			}
			p_out->write(titleformat_inputtypes::meta, "\4", 1);
		}
		return true;
	}
	return baseClass::process_function(p_out, p_name, p_name_length, p_params, p_found_flag);
}

bool titleformat_hook_impl_file_info_branch::process_meta_branch(titleformat_text_out * p_out, t_size p_index)
{
	if (p_index == pfc_infinite) return false;

	t_size n, m = m_info->meta_enum_value_count(p_index);
	bool found = false;
	p_out->write(titleformat_inputtypes::meta, "\4", 1);
	for (n = 0; n < m; n++)
	{
		if (n > 0)
		{
			p_out->write(titleformat_inputtypes::meta, "\5", 1);
		}
		const char * value = m_info->meta_enum_value(p_index, n);
		if (*value != 0) found = true;
		p_out->write(titleformat_inputtypes::meta, value, pfc_infinite);
	}
	p_out->write(titleformat_inputtypes::meta, "\4", 1);
	return found;
}
