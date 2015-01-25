#ifndef _COLUMNS_API_COLUMNS_H_
#define _COLUMNS_API_COLUMNS_H_

/**
* \file columns_ui.h
* \brief Columns UI Definitions
*/

/** \brief Namespace containing all Columns UI definitions */
namespace columns_ui 
{
	/** \brief Namespace containting standard Columns UI toolbar GUIDs */
	namespace toolbars
	{
		extern const GUID
			guid_menu,
			guid_buttons,
			guid_playback_order,
			guid_spectrum_analyser,
			guid_seek_bar,
			guid_volume_control,
			guid_filter_search_bar;
	}
	/** \brief Namespace containting standard Columns UI panel GUIDs */
	namespace panels
	{
		extern const GUID
			guid_playlist_view,
			guid_playlist_switcher,
			guid_playlist_tabs,
			guid_horizontal_splitter,
			guid_vertical_splitter,
			guid_playlist_view_v2,
			guid_filter,
			guid_artwork_view,
			guid_item_properties,
			guid_item_details;
	}
	/** \brief Namespace containting standard Columns UI visualisation GUIDs */
	namespace visualisations
	{
		extern const GUID
			guid_spectrum_analyser;
	}

	/** \brief Namespace containting Columns UI string GUIDs */
	namespace strings
	{
		extern const GUID
			guid_global_variables;
	}

	/**
	* \brief Service exposing Columns UI control methods
	* \remarks
	* - One implementation in Columns UI, do not reimplement.
	* - Call from main thread only
	*/
	class NOVTABLE control : public service_base
	{
	public:
		/** \brief Retrieves a string from Columns UI */
		virtual bool get_string(const GUID & p_guid, pfc::string_base & p_out) const =0;

		FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(control);
	};

	/**
	* \brief Pointer to the columns_ui::control service, initialised in its constructor
	*
	* If you use the control service, you should make sure you handle the cases where the
	* service is not available (no Columns UI installed / incompatible Columns UI installed)
	* by handling the exceptions below.
	*
	* \throw
	* The constructor for this class may throw:
	* - exception_service_not_found if the service was not found
	* - exception_service_duplicated if the service was duplicated
	*/
	typedef static_api_ptr_t<control> static_control_ptr;

	/**
	* \brief Holds a pointer to the columns_ui::control service
	*/
	typedef service_ptr_t<control> control_ptr;

	class global_variable
	{
	public:
		global_variable(const char * p_name,t_size p_name_length,const char * p_value,t_size p_value_length)
			: m_name(p_name,p_name_length), m_value(p_value,p_value_length)
		{}
		inline const char * get_name() const {return m_name;}
		inline const char * get_value() const {return m_value;}
	private:
		pfc::string_simple m_name,m_value;
	};

	class global_variable_list : public pfc::ptr_list_t<global_variable>
	{
	public:
		const char * find_by_name(const char * p_name, t_size length)
		{
			unsigned n, count = get_count();
			for (n=0;n<count;n++)
			{
				const char * ptr = get_item(n)->get_name();
				if (!stricmp_utf8_ex(p_name,length,ptr,pfc_infinite))
					return get_item(n)->get_value();
			}
			return 0;
		}
		void add_item(const char * p_name,t_size p_name_length,const char * p_value,t_size p_value_length)
		{
			global_variable * var = new global_variable(p_name,p_name_length,p_value,p_value_length);
			pfc::ptr_list_t<global_variable>::add_item(var);
		}
		~global_variable_list() {delete_all();}
	};

	template <bool set = true, bool get = true>
	class titleformat_hook_global_variables : public titleformat_hook
	{
		global_variable_list & p_vars;
	public:
		virtual bool process_field(titleformat_text_out * p_out,const char * p_name,unsigned p_name_length,bool & p_found_flag)
		{
			p_found_flag = false;
			return false;
		}

		virtual bool process_function(titleformat_text_out * p_out,const char * p_name,unsigned p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag)
		{
			p_found_flag = false;
			if (set && !stricmp_utf8_ex(p_name,p_name_length,"set_global",pfc_infinite))
			{
				switch(p_params->get_param_count())
				{
				case 2:
					{
						const char * name;
						const char * value;
						t_size name_length,value_length;
						p_params->get_param(0,name,name_length);
						p_params->get_param(1,value,value_length);
						p_vars.add_item(name,name_length,value,value_length);
						p_found_flag = true;
						return true;
					}
				default:
					return false;
				}
			}
			else if (get && !stricmp_utf8_ex(p_name,p_name_length,"get_global",pfc_infinite))
			{
				switch(p_params->get_param_count())
				{
				case 1:
					{
						const char * name;
						t_size name_length;
						p_params->get_param(0,name,name_length);
						const char * ptr = p_vars.find_by_name(name,name_length);
						if (ptr)
						{
							p_out->write(titleformat_inputtypes::meta, ptr,pfc_infinite);
							p_found_flag = true;
						}
						else p_out->write(titleformat_inputtypes::meta, "[unknown variable]",pfc_infinite);
						return true;
					}
				default:
					return false;
				}
			}
			else return false;
		}
		inline titleformat_hook_global_variables(global_variable_list & vars) : p_vars(vars)
		{
		};
	};

	namespace fcl
	{
		PFC_DECLARE_EXCEPTION(exception_missing_panel, pfc::exception, "Missing panel.");
		/** Namespace containing standard FCL group identifiers. */
		namespace groups
		{
			extern const GUID
				toolbars,
				layout,
				colours_and_fonts,
				title_scripts;
		}
		enum t_fcl_type
		{
			type_public=0, //use export_config/import_config on panels
			type_private=1, //use set_config/get_config on panels
		};

		class NOVTABLE t_import_feedback
		{
		public:
			/**
			* Specifies any panels that you are dependent on that are not installed. You must specify only missing panels.
			*
			* \param [in]	name	Unused. Pass a null-terminiated empty string.
			* \param [in]	guid	GUID of panel.
			*/
			virtual void add_required_panel(const char * name, const GUID & guid)=0;
		};
		class NOVTABLE t_export_feedback
		{
		public:
			/**
			* Specifies panels that you are dependent on. You must specify all dependent panels.
			*
			* \param [in]	panels	GUIDs of panels.
			*/
			virtual void add_required_panels(const pfc::list_base_const_t<GUID> & panels)=0;
			void add_required_panel(GUID guid) {add_required_panels(pfc::list_single_ref_t<GUID>(guid));}
		};

		/** Groups are nodes in the export tree. Each group can have multiple datasets. */
		class NOVTABLE group : public service_base
		{
		public:
			virtual void get_name (pfc::string_base & p_out)const =0 ;
			virtual void get_description (pfc::string_base & p_out)const =0;
			virtual const GUID & get_guid ()const =0 ;

			/** NULL guid <=> root node */
			virtual const GUID & get_parent_guid () const {return pfc::guid_null;}

			FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(group);
		};

		/** Helper. Use group_impl_factory below. */
		class group_impl : public group
		{
		public:
			virtual void get_name (pfc::string_base & p_out) const {p_out = m_name;}
			virtual void get_description (pfc::string_base & p_out) const {p_out = m_desc;}
			virtual const GUID & get_guid () const {return m_guid;}
			virtual const GUID & get_parent_guid () const {return m_parent_guid;}

			GUID m_guid, m_parent_guid;
			pfc::string8 m_name, m_desc;

			group_impl ( const GUID & pguid, const char * pname, const char * pdesc, const GUID & pguidparent = pfc::guid_null)
				: m_guid(pguid), m_parent_guid(pguidparent), m_name(pname), m_desc(pdesc)
			{};

		};

		/** Helper. */
		class group_impl_factory : public service_factory_single_t<group_impl>
		{
		public:
			group_impl_factory ( const GUID & pguid, const char * pname, const char * pdesc, const GUID & pguidparent = pfc::guid_null)
				: service_factory_single_t<group_impl>(pguid, pname, pdesc, pguidparent)
			{};
		};

		class NOVTABLE dataset : public service_base
		{
		public:
			/** User-friendly fully qualified (unambiguous) name. */
			virtual void get_name (pfc::string_base & p_out)const=0 ;
			/** Unique identifier of the dataset. */
			virtual const GUID & get_guid ()const=0 ;
			/** The identifier of the group you belong to. */
			virtual const GUID & get_group ()const=0 ;
			/**
			* Retrieves your data for an export.
			*
			* \param [in]	type	Specifies export mode. See t_fcl_type.
			*/
			virtual void get_data (stream_writer * p_writer, t_uint32 type, t_export_feedback & feedback, abort_callback & p_abort)const=0 ;
			/**
			* Sets your data for an import.
			*
			* \param [in]	type	Specifies export mode. See t_fcl_type.
			*/
			virtual void set_data (stream_reader * p_reader, t_size size, t_uint32 type, t_import_feedback & feedback, abort_callback & p_abort)=0;

			FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(dataset);
		};

		typedef service_ptr_t<dataset> dataset_ptr;
		typedef service_ptr_t<group> group_ptr;

		template<class T>
		class dataset_factory : public service_factory_single_t<T> {};

		template<class T>
		class group_factory : public service_factory_single_t<T> {};

		/** Helper. */
		template <class t_service, class t_service_ptr = service_ptr_t<t_service> >
		class service_list_auto_t : public pfc::list_t<t_service_ptr >
		{
		public:
			service_list_auto_t()
			{
				service_enum_t<t_service> export_enum;
				t_service_ptr ptr;
				while (export_enum.next(ptr))
					add_item(ptr);
			};
			bool find_by_guid(const GUID & guid, t_service_ptr & p_out)
			{
				t_size i, count = get_count();
				for (i=0; i<count; i++)
				{
					if (get_item(i)->get_guid() == guid)
					{
						p_out = get_item(i);
						return true;
					}
				}
				return false;
			}
			void remove_by_guid(const GUID & guid)
			{
				t_size count = get_count();
				for (; count; count--)
				{
					if (get_item(count-1)->get_guid() == guid)
					{
						remove_by_idx(count-1);
					}
				}
			}
			static int g_compare_name(const t_service_ptr & i1, const t_service_ptr & i2)
			{
				pfc::string8 n1, n2;
				i1->get_name(n1);
				i2->get_name(n2);
				return StrCmpLogicalW(pfc::stringcvt::string_os_from_utf8(n1), pfc::stringcvt::string_os_from_utf8(n2));
			}
			void sort_by_name() {sort_t(g_compare_name);}
		};		

		typedef service_list_auto_t<dataset> dataset_list;
		typedef service_list_auto_t<group> group_list;

		/** Helper. */
		class group_list_filtered : public pfc::list_t<group_ptr>
		{
			pfc::list_t<t_size> m_original_indices;
		public:
			t_size get_original_index(t_size index) const {return m_original_indices[index];}
			group_list_filtered( const pfc::list_base_const_t<group_ptr> & list, const GUID & guid)
			{
				t_size i, count = list.get_count();
				for (i=0; i<count; i++)
				{
					if (list[i]->get_parent_guid() == guid)
					{
						add_item(list[i]);
						m_original_indices.add_item(i);
					}
				}
			}
		};
	}
}

#define cui columns_ui

#endif //_COLUMNS_API_COLUMNS_H_