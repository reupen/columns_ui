#ifndef _COLUMNS_API_SPLITTER_H_
#define _COLUMNS_API_SPLITTER_H_

/**
* \file window.h
* \brief Splitter Window API
*/

namespace ui_extension
{

	/**
	* \brief Holds data about a splitter item
	*
	* Derive from here and also store your other stuff (show_caption..)
	* Functions as data container only!
	*/
	class NOVTABLE splitter_item_t
	{
	public:
		virtual const GUID & get_panel_guid()const=0;
		/** Setting GUID deletes panel config and window ptr (i.e. do it first)*/
		virtual void set_panel_guid(const GUID & p_guid) =0;

		virtual void get_panel_config (stream_writer * p_out)const=0;
		virtual void set_panel_config (stream_reader * p_reader, t_size p_size)=0;

		virtual const window_ptr & get_window_ptr()const=0;

		template <typename t_class>
		bool query(const t_class * & p_out) const
		{
			if (query(t_class::get_class_guid()))
			{
				p_out = static_cast<const t_class*>(this);
				return true;
			}
			return false;
		}

		template <typename t_class>
		bool query(t_class * & p_out)
		{
			if (query(t_class::get_class_guid()))
			{
				p_out = static_cast<t_class*>(this);
				return true;
			}
			return false;
		}

		virtual bool query(const GUID & p_guid) const {return false;}
		virtual ~splitter_item_t() {};

		void set(const splitter_item_t & p_source);
		splitter_item_t & operator = (const splitter_item_t & p_source)
		{
			set(p_source);
			return *this;
		}
	};

	typedef pfc::ptrholder_t<splitter_item_t> splitter_item_ptr;

	/** \brief Implements splitter_item_t with the standard set of data stored */
	template <class t_base>
	class splitter_item_simple : public t_base
	{
	public:
		virtual const GUID & get_panel_guid()const {return m_guid;}
		virtual void get_panel_config (stream_writer * p_out)const 
		{
			abort_callback_impl p_abort;
			p_out->write(m_data.get_ptr(), m_data.get_size(), p_abort);
		}
		virtual void set_panel_guid(const GUID & p_guid) 
		{
			//if (m_guid != p_guid)
			{
				m_guid = p_guid;
				m_data.set_size(0);
				m_ptr.release();
			}
		}
		virtual void set_panel_config (stream_reader * p_reader, t_size p_size) 
		{
			abort_callback_impl p_abort;
			m_data.set_size(p_size);
			p_reader->read(m_data.get_ptr(), m_data.get_size(), p_abort);
		}
		virtual const window_ptr & get_window_ptr()const{return m_ptr;}
		void set_window_ptr(const window_ptr & p_source){m_ptr = p_source;}

		inline splitter_item_simple(const splitter_item_t & p_source) 
		{
			set(p_source);
		}
		inline splitter_item_simple() : m_guid(pfc::guid_null)
		{
		}
	protected:
		GUID m_guid;
		pfc::array_t<t_uint8> m_data;
		window_ptr m_ptr;
	};

	typedef splitter_item_simple<splitter_item_t> splitter_item_simple_t;

	/** \brief Implements splitter_item_t with a full set of data stored */
	class splitter_item_full_t : public splitter_item_t
	{
	public:
		unsigned m_caption_orientation;
		bool m_locked;
		bool m_hidden;
		bool m_autohide;
		bool m_show_caption;
		unsigned m_size;
		bool m_show_toggle_area;
		bool m_custom_title;

		virtual void get_title(pfc::string_base & p_out)const=0;
		virtual void set_title(const char * p_title, t_size length)=0;

		static const GUID & get_class_guid()
		{
			// {57C8EEAE-0D4F-486a-8769-FFFCB4B4349B}
			static const GUID rv = 
			{ 0x57c8eeae, 0xd4f, 0x486a, { 0x87, 0x69, 0xff, 0xfc, 0xb4, 0xb4, 0x34, 0x9b } };
			return rv;
		}

		virtual bool query(const GUID & p_guid) const {return (p_guid == get_class_guid()) != 0;}
	};

	class splitter_item_full_impl_t : public splitter_item_simple<splitter_item_full_t>
	{
		pfc::string8 m_title;
	public:
		virtual void get_title(pfc::string_base & p_out)const
		{
			p_out = m_title;
		}
		virtual void set_title(const char * p_title, t_size length)
		{
			m_title.set_string(p_title, length);
		}
	};
	class stream_writer_fixedbuffer : public stream_writer {
	public:
		void write(const void * p_buffer,t_size p_bytes,abort_callback & p_abort) {
			if (p_bytes > 0) {
				if (p_bytes > m_bytes - m_bytes_read) throw pfc::exception_overflow();
				memcpy((t_uint8*)m_out,p_buffer,p_bytes);
				m_bytes_read += p_bytes;
			}
		}
		stream_writer_fixedbuffer(void * p_out,t_size p_bytes,t_size & p_bytes_read) : m_out(p_out), m_bytes(p_bytes), m_bytes_read(p_bytes_read) {m_bytes_read = 0;}
	private:
		void * m_out;
		t_size m_bytes;
		t_size & m_bytes_read;
	};

	class splitter_callback;

	/**
	* \brief Subclass of ui_extension::window, specifically for splitters.
	*
	* Splitter classes must support multiple instances
	*/
	class NOVTABLE splitter_window : public window
	{
	public:

		static const GUID bool_show_caption;
		static const GUID bool_hidden;
		static const GUID bool_autohide;
		static const GUID bool_locked;
		static const GUID uint32_orientation;
		static const GUID bool_show_toggle_area;
		static const GUID uint32_size;
		static const GUID bool_use_custom_title;
		static const GUID string_custom_title;

		/**
		* \brief Get config item supported
		*
		* \return count
		*/
		virtual bool get_config_item_supported(t_size p_index, const GUID & p_type) const
		{
			return false;
		}

		/**
		* \brief Creates non-modal child configuration dialog.
		* Since its non-modal, remember to keep a refcounted reference to yourself.
		* Use WS_EX_CONTROLPARENT
		*/
		//virtual HWND create_config_window(HWND wnd_parent, const container_window::window_position_t & p_placement) {return 0;}

		//this config system isn't great. it may be changed.
		//write in native-endianess (i.e. use write_object_t)
		virtual bool get_config_item(t_size index, const GUID & p_type, stream_writer * p_out, abort_callback & p_abort) const
		{
			return false;
		};

		bool get_config_item(t_size index, const GUID & p_type, stream_writer * p_out) const
		{
			return get_config_item(index, p_type, p_out, abort_callback_impl());
		}
		virtual bool set_config_item(t_size index, const GUID & p_type, stream_reader * p_source, abort_callback & p_abort)
		{
			return false;
		};
		template <typename class_t>
		bool set_config_item_t(t_size index, const GUID & p_type, const class_t & p_val, abort_callback & p_abort)
		{
			return set_config_item(index, p_type, &stream_reader_memblock_ref(&p_val, sizeof(class_t)), p_abort);
		};

		template <class T> bool get_config_item(t_size p_index, const GUID & p_type, T & p_out, abort_callback & p_abort) const
		{
			t_size written;
			return get_config_item(p_index, p_type, &stream_writer_fixedbuffer(&p_out, sizeof(T), written), p_abort);
		}

		template <class T> bool get_config_item(t_size p_index, const GUID & p_type, T & p_out) const
		{
			return get_config_item(p_index, p_type, p_out, abort_callback_impl());
		}
		/** This method may be called on both active and inactive (i.e. no window) instances */
		virtual void insert_panel(t_size index, const splitter_item_t * p_item)=0;
		/** This method may be called on both active and inactive (i.e. no window) instances */
		virtual void remove_panel(t_size index)=0;
		/** This method may be called on both active and inactive (i.e. no window) instances */
		virtual void replace_panel(t_size index, const splitter_item_t * p_item)=0;
		virtual t_size get_panel_count()const=0;
		virtual t_size get_maximum_panel_count()const{return pfc_infinite;};

		/** Reserved for future use */
		virtual void register_callback(splitter_callback * p_callback){};
		/** Reserved for future use */
		virtual void deregister_callback(splitter_callback * p_callback){};
	protected:
		/**
		* Return value needs deleting!! Use pfc::ptrholder_t
		* This method may be called on both active and inactive (i.e. no window) instances
		*/
		virtual splitter_item_t * get_panel(t_size index)const=0;
	public:
		void get_panel(t_size index, pfc::ptrholder_t<splitter_item_t> & p_out)const
		{
			p_out = get_panel(index);
		}

		t_size add_panel(const splitter_item_t * p_item){t_size count = get_panel_count(); insert_panel(count, p_item); return count;}

		//helpers
		inline void swap_items(t_size p_item1, t_size p_item2)
		{
			splitter_item_ptr p1, p2;
			get_panel(p_item1, p1);
			get_panel(p_item2, p2);
			replace_panel(p_item1, p2.get_ptr());
			replace_panel(p_item2, p1.get_ptr());
		}

		inline bool move_up(t_size p_index)
		{
			t_size count = get_panel_count();
			if (p_index > 0 && p_index< count)
			{
				swap_items(p_index, p_index-1);
				return true;
			}
			return false;
		}
		inline bool move_down(t_size p_index)
		{
			t_size count = get_panel_count();
			if (p_index >= 0 && p_index < (count-1))
			{
				swap_items(p_index, p_index+1);
				return true;
			}
			return false;
		}
		bool find_by_ptr (const uie::window::ptr & window, t_size & p_index)
		{
			t_size i, count = get_panel_count();
			for (i=0; i<count; i++)
			{
				splitter_item_ptr si;
				get_panel(i, si);
				if (si->get_window_ptr() == window)
				{
					p_index = i;
					return true;
				}
			}
			return false;
		}
		void remove_panel(const uie::window::ptr & window)
		{
			t_size i, count = get_panel_count();
			for (i=0; i<count; i++)
			{
				splitter_item_ptr si;
				get_panel(i, si);
				if (si->get_window_ptr() == window)
				{
					remove_panel(i);
					return;
				}
			}
		}

		FB2K_MAKE_SERVICE_INTERFACE(splitter_window, window);
	};
	class NOVTABLE splitter_window_v2 : public splitter_window
	{
	public:
		//virtual void enter_layout_editing_mode()=0;
		//virtual void exit_layout_editing_mode()=0;
		//virtual bool check_wnd_is_splitter(HWND wnd)=0;

		virtual bool is_point_ours(HWND wnd_point, const POINT & pt_screen, pfc::list_base_t<uie::window::ptr> & p_hierarchy) {return false;};
		virtual void get_supported_panels(const pfc::list_base_const_t<uie::window::ptr> & p_windows, bit_array_var & p_mask_unsupported) {};
		FB2K_MAKE_SERVICE_INTERFACE(splitter_window_v2, splitter_window);
	};
}
#endif //_COLUMNS_API_SPLITTER_H_