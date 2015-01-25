#ifndef _CUI_FILTER_H_
#define _CUI_FILTER_H_
namespace filter_panel {

	class field_t
	{
	public:
		pfc::string8 m_name, m_field;
	};
	class cfg_fields_t : public cfg_var, public pfc::list_t<field_t>
	{
	public:
		enum {stream_version=0};
		virtual void set_data_raw(stream_reader * p_stream,t_size p_sizehint,abort_callback & p_abort)
		{
			t_uint32 version;
			p_stream->read_lendian_t(version, p_abort);
			if (version <= stream_version)
			{
				t_uint32 count,i;
				p_stream->read_lendian_t(count, p_abort);
				set_count(count);
				for (i=0; i<count; i++)
				{
					p_stream->read_string((*this)[i].m_name, p_abort);
					p_stream->read_string((*this)[i].m_field, p_abort);
				}
			}
		}
		virtual void get_data_raw(stream_writer * p_stream,abort_callback & p_abort)
		{
			p_stream->write_lendian_t((t_uint32)stream_version, p_abort);
			t_uint32 i, count=get_count();
			p_stream->write_lendian_t((t_uint32)count, p_abort);
			for (i=0; i<count; i++)
			{
				p_stream->write_string((*this)[i].m_name, p_abort);
				p_stream->write_string((*this)[i].m_field, p_abort);
			}
		}
		void reset()
		{
			set_count(3);
			t_size i=0;
			(*this)[i].m_name = ((*this)[i].m_field = "Genre");
			i++;
			(*this)[i].m_field = "Album Artist;Artist";
			(*this)[i].m_name = "Artist";
			i++;
			(*this)[i].m_name = ((*this)[i].m_field = "Album");
		}
		bool have_name(const char * p_name)
		{
			t_size i, count=get_count();
			for (i=0; i<count; i++)
				if (!stricmp_utf8(p_name, (*this)[i].m_name))
					return true;
			return false;
		}
		void fix_name (const char * p_name, pfc::string8 & p_out)
		{
			t_size i = 0;
			p_out = p_name;
			while (have_name(p_out))
			{
				p_out.reset();
				p_out << p_name << " (" << (++i) << ")"; 
			}
		}
		void fix_name (pfc::string8 & p_name)
		{
			fix_name(pfc::string8(p_name), p_name);
		}
		cfg_fields_t(const GUID & p_guid) : cfg_var(p_guid) {reset();}
	};

extern cfg_string /*cfg_fields, */cfg_sort_string;
extern cfg_bool cfg_sort, cfg_autosend, cfg_orderedbysplitters, cfg_showemptyitems;
extern cfg_int cfg_doubleclickaction, cfg_middleclickaction, cfg_edgestyle, cfg_itempadding;
extern cfg_fields_t cfg_field_list;
void g_on_fields_change();
void g_on_new_field(const field_t & field);
void g_on_field_removed(t_size index);
void g_on_fields_swapped(t_size index_1, t_size index_2);
void g_on_field_title_change(const char * p_old, const char * p_new);
void g_on_field_query_change(const field_t & field);
void g_on_edgestyle_change();
void g_on_orderedbysplitters_change();
void g_on_showemptyitems_change(bool b_val);
void g_on_vertical_item_padding_change();

};

#endif