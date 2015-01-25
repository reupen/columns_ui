#include "stdafx.h"





namespace mmh 
{
	t_size poweroften(t_size raiseTo)
	{
		t_size ret = 1, i;
		for (i=0; i<raiseTo; i++)
			ret *= 10;
		return ret;
	}

	int g_compare_context(void* context, const void * item1, const void* item2)
	{
		return ((sort_base*)(context))->compare(item1, item2);
	}

	int g_compare_context_lt(void* context, const void * item1, const void* item2)
	{
		return ((sort_base*)(context))->compare(item1, item2)<0;
	}

	namespace fb2k
	{
		int g_sort_metadb_handle_list_by_format_gepermutation_t_partial_compare(const pfc::array_t<WCHAR> & elem1, const pfc::array_t<WCHAR> & elem2 )
		{
			return StrCmpLogicalW(elem1.get_ptr(), elem2.get_ptr());
		}

		void g_sort_metadb_handle_list_by_format_get_permutation_t_partial(const pfc::list_base_const_t<metadb_handle_ptr> & p_list,t_size base,t_size count,permutation_t & order,const service_ptr_t<titleformat_object> & p_script,titleformat_hook * p_hook, bool b_stablise)
		{
			assert(base+count<=p_list.get_count());
			t_size n;
			pfc::array_t< pfc::array_t<WCHAR> > data;
			data.set_size(count);

			pfc::string8_fastalloc temp;
			pfc::string8_fastalloc temp2;
			temp.prealloc(512);
			for(n=0;n<count;n++)
			{
				metadb_handle_ptr item;
				p_list.get_item_ex(item,base+n);
				assert(item.is_valid());

				item->format_title(p_hook,temp,p_script,0);
				data[n].set_size(pfc::stringcvt::estimate_utf8_to_wide_quick(temp, temp.length()));
				pfc::stringcvt::convert_utf8_to_wide_unchecked(data[n].get_ptr(), temp);
			}

			g_sort_get_permutation_qsort(data, order, g_sort_metadb_handle_list_by_format_gepermutation_t_partial_compare, b_stablise);
		}

		void g_sort_metadb_handle_list_by_format_get_permutation(metadb_handle_ptr * p_list, permutation_t & order,const service_ptr_t<titleformat_object> & p_script,titleformat_hook * p_hook, bool b_stablise)
		{
			g_sort_metadb_handle_list_by_format_get_permutation_t_partial(p_list, order.get_count(), 0, order.get_count(), order, p_script, p_hook, b_stablise);
		}
		void g_sort_metadb_handle_list_by_format_get_permutation_t_partial(metadb_handle_ptr * p_list, t_size p_list_count, t_size base,t_size count,permutation_t & order,const service_ptr_t<titleformat_object> & p_script,titleformat_hook * p_hook, bool b_stablise)
		{
			assert(base+count<=p_list_count);
			t_size n;
			pfc::array_t< pfc::array_t<WCHAR> > data;
			data.set_size(count);

			pfc::string8_fastalloc temp;

			temp.prealloc(512);
			for(n=0;n<count;n++)
			{
				p_list[base+n]->format_title(p_hook,temp,p_script,0);
				data[n].set_size(pfc::stringcvt::estimate_utf8_to_wide_quick(temp, temp.length()));
				pfc::stringcvt::convert_utf8_to_wide_unchecked(data[n].get_ptr(), temp);
			}

			g_sort_get_permutation_qsort(data, order, g_sort_metadb_handle_list_by_format_gepermutation_t_partial_compare, b_stablise);

		}
		void g_sort_metadb_handle_list_by_format(pfc::list_base_t<metadb_handle_ptr> & p_list,const service_ptr_t<titleformat_object> & p_script,titleformat_hook * p_hook, bool b_stablise)
		{
			permutation_t perm (p_list.get_count());
			g_sort_metadb_handle_list_by_format_get_permutation_t_partial(p_list, 0, perm.get_count(), perm, p_script, p_hook, b_stablise);
			p_list.reorder(perm.get_ptr());
		}
	}

	const char * g_convert_utf16_to_ascii(const WCHAR * str_utf16, t_size len, pfc::string_base & p_out)
	{
		char * replacement = "_";
		t_size len_max = min (wcslen(str_utf16), len);
		if (len_max)
		{
			int size_ascii = WideCharToMultiByte(20127, NULL, str_utf16, len_max, NULL, NULL, replacement, NULL);
			if (!size_ascii)
				throw exception_win32(GetLastError());

			pfc::array_t<char> str_ascii;
			str_ascii.set_size(size_ascii+1);
			str_ascii.fill_null();
			int ret = WideCharToMultiByte(20127, NULL, str_utf16, len_max, str_ascii.get_ptr(), size_ascii, replacement, NULL);
			if (!ret)
				throw exception_win32(GetLastError());
			p_out.set_string(str_ascii.get_ptr(), size_ascii);
		}
		else p_out.reset();
		return p_out.get_ptr();
	}

	const char * g_convert_utf8_to_ascii(const char * p_source, pfc::string_base & p_out)
	{
		pfc::stringcvt::string_wide_from_utf8 str_utf16(p_source);
		g_convert_utf16_to_ascii(str_utf16, pfc_infinite, p_out);
		return p_out.get_ptr();
	}

	char format_digit(unsigned p_val)
	{
		PFC_ASSERT(p_val < 16);
		return (p_val < 10) ? p_val + '0' : p_val - 10 + 'A';
	}
}
