#pragma once

template <typename t_type>
class config_item_t
{
	cfg_int_t<t_type> m_value;
public:

	void reset(){set(get_default_value());}
	void set (t_type p_val){m_value = p_val; on_change();}
	t_type get () const {return m_value;};

	virtual t_type get_default_value ()=0;
	virtual void on_change()=0;
	virtual const GUID & get_guid()=0;
	config_item_t(const GUID & p_guid, t_type p_value) : m_value(p_guid,p_value)
	{};
};

template<>
class config_item_t<pfc::string8>
{
	cfg_string m_value;
public:

	void reset(){set(get_default_value());}
	void set (const char * p_val){m_value = p_val; on_change();}
	const char * get () const {return m_value;};

	virtual const char * get_default_value ()=0;
	virtual void on_change()=0;
	virtual const GUID & get_guid()=0;
	config_item_t(const GUID & p_guid, const char * p_value) : m_value(p_guid, p_value)
	{};
};

namespace fcl
{
	class writer
	{
	public:
		template <typename t_item>
		void write_raw (const t_item & item)
		{
			pfc::assert_raw_type<t_item>();
			m_output->write_lendian_t(item, m_abort);
		};
		void write_item (unsigned id, const void * item, t_size size)
		{
			m_output->write_lendian_t(id, m_abort);
			m_output->write_lendian_t(size, m_abort);
			m_output->write(item, size, m_abort);
		};
		template <typename t_item>
		void write_item (unsigned id, const t_item & item)
		{
			pfc::assert_raw_type<t_item>();
			m_output->write_lendian_t(id, m_abort);
			m_output->write_lendian_t(sizeof(item), m_abort);
			m_output->write_lendian_t(item, m_abort);
		};
		template <typename t_int_type>
		void write_item (unsigned id, const cfg_int_t<t_int_type> & item)
		{
			m_output->write_lendian_t(id, m_abort);
			m_output->write_lendian_t(sizeof(t_int_type), m_abort);
			m_output->write_lendian_t(t_int_type(item.get_value()), m_abort);
		};
		template <>
		void write_item (unsigned id, const cfg_string & item)
		{
			write_item(id, item.get_ptr());
		};
		template <>
		void write_item (unsigned id, const char * const & item)
		{
			m_output->write_lendian_t(id, m_abort);
			m_output->write_string(item, m_abort);
		};
		template <>
		void write_item (unsigned id, const pfc::string8 & item)
		{
			write_item(id, item.get_ptr());
		};
		template <typename t_int_type>
		void write_item_config (unsigned id, const config_item_t<t_int_type> & item)
		{
			m_output->write_lendian_t(id, m_abort);
			m_output->write_lendian_t(sizeof(t_int_type), m_abort);
			m_output->write_lendian_t(t_int_type(item.get()), m_abort);
		};
		template <>
		void write_item(unsigned id, const cfg_struct_t<LOGFONT> & cfg_lfc)
		{
			write_item(id, cfg_lfc.get_value());
		}
		template <>
		void write_item(unsigned id, const LOGFONT & lfc)
		{
			LOGFONT lf = lfc;
			t_size face_len = pfc::wcslen_max(lf.lfFaceName, tabsize(lf.lfFaceName));
			
			if (face_len < tabsize(lf.lfFaceName))
			{
				WCHAR * ptr = lf.lfFaceName;
				ptr += face_len;
				memset(ptr, 0, sizeof(WCHAR)*(tabsize(lf.lfFaceName)-face_len));
			}

			m_output->write_lendian_t(id, m_abort);
			m_output->write_lendian_t(sizeof(lf), m_abort);

			m_output->write_lendian_t(lf.lfHeight, m_abort);
			m_output->write_lendian_t(lf.lfWidth, m_abort);
			m_output->write_lendian_t(lf.lfEscapement, m_abort);
			m_output->write_lendian_t(lf.lfOrientation, m_abort);
			m_output->write_lendian_t(lf.lfWeight, m_abort);

			//meh endianness
			m_output->write(&lf.lfItalic,8 + sizeof(lf.lfFaceName),m_abort);
		}
		writer(stream_writer * p_out, abort_callback & p_abort)
			: m_abort(p_abort), m_output(p_out)
		{} ;
	private:
		stream_writer * m_output;
		abort_callback & m_abort;
	};

	class reader
	{
	public:
		template <typename t_item>
		void read_item (t_item & p_out)
		{
			pfc::assert_raw_type<t_item>();
			m_input->read_lendian_t(p_out, m_abort);
			m_position += sizeof (t_item);
		};
		void read (void * pout, t_size size)
		{
			t_size pread = m_input->read(pout, size, m_abort);
			m_position += pread;
			if (pread != size) throw exception_io_data_truncation();
		};
		template <typename t_int_type>
		void read_item (cfg_int_t<t_int_type> & p_out)
		{
			pfc::assert_raw_type<t_int_type>();
			t_int_type temp;
			m_input->read_lendian_t(temp, m_abort);
			p_out = temp;
			m_position += sizeof (t_int_type);
		};
		void read_item (cfg_string & p_out, t_size size)
		{
			read_item((pfc::string8&)p_out, size);
		};
		void read_item (pfc::string8 & p_out, t_size size)
		{
			pfc::array_t<char> temp;
			temp.set_size(size+1);
			temp.fill(0);

			if (size)
			{
				unsigned read = 0;
				read = m_input->read(temp.get_ptr(), size, m_abort);
				if (read != size) throw exception_io_data_truncation();
			}
			p_out = temp.get_ptr();
			m_position += size;
		};
		template <typename t_int_type>
		void read_item_config (config_item_t<t_int_type> & p_out)
		{
			pfc::assert_raw_type<t_int_type>();
			t_int_type temp;
			m_input->read_lendian_t(temp, m_abort);
			p_out.set(temp);
			m_position += sizeof (t_int_type);
		};
		template<>
		void read_item(cfg_struct_t<LOGFONT> & cfg_out)
		{
			read_item(cfg_out.get_value());
		}
		template<>
		void read_item(LOGFONT & lf_out)
		{
			LOGFONT lf;

			read_item(lf.lfHeight);
			read_item(lf.lfWidth);
			read_item(lf.lfEscapement);
			read_item(lf.lfOrientation);
			read_item(lf.lfWeight);

			//meh endianness
			read(&lf.lfItalic,8 + sizeof(lf.lfFaceName));

			lf_out = lf;
		}
		template <typename t_item>
		void read_item_force_bool (t_item & p_out)
		{
			pfc::assert_raw_type<t_item>();
			bool temp;
			m_input->read_lendian_t(temp, m_abort);
			t_item = temp;
			m_position += sizeof (bool);
		};
		template <typename t_int_type>
		void read_item_force_bool (cfg_int_t<t_int_type> & p_out)
		{
			bool temp;
			m_input->read_lendian_t(temp, m_abort);
			p_out = temp;
			m_position += sizeof (bool);
		};
		t_size get_remaining ()
		{
			return m_position > m_size ? 0 : m_size - m_position;
		}
		void skip (t_size delta)
		{
			if (m_input->skip(delta, m_abort) != delta)
				throw exception_io_data_truncation();
		}
		reader(stream_reader * p_input, t_size size, abort_callback & p_abort)
			: m_abort(p_abort), m_input(p_input), m_position(0), m_size(size)
		{} ;
		reader(reader & p_reader, t_size size, abort_callback & p_abort)
			: m_abort(p_abort), m_input(p_reader.m_input), m_position(0), m_size(size)
		{
			p_reader.m_position += size;
		};
	private:
		t_size m_size, m_position;
		stream_reader * m_input;
		abort_callback & m_abort;
	};
}
