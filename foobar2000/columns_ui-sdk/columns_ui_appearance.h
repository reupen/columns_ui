#ifndef _COLUMNS_UI_APPEARANCE_
#define _COLUMNS_UI_APPEARANCE_

namespace columns_ui
{
	namespace colours
	{
		enum colour_identifier_t
		{
			colour_text,
			colour_selection_text,
			colour_inactive_selection_text,
			colour_background,
			colour_selection_background,
			colour_inactive_selection_background,
			colour_active_item_frame,
			/** Reserved */
			colour_group_foreground, 
			/** Reserved */
			colour_group_background, 
		};
		enum colour_flag_t
		{
			colour_flag_text = 1<<colour_text,
			colour_flag_selection_text = 1<<colour_selection_text,
			colour_flag_inactive_selection_text= 1<<colour_inactive_selection_text,
			colour_flag_background= 1<<colour_background,
			colour_flag_selection_background= 1<<colour_selection_background,
			colour_flag_inactive_selection_background= 1<<colour_inactive_selection_background,
			colour_flag_active_item_frame= 1<<colour_active_item_frame,
			colour_flag_group_foreground= 1<<colour_group_foreground,
			colour_flag_group_background= 1<<colour_group_background,

			colour_flag_all = (1<<colour_text)|(1<<colour_selection_text)|(1<<colour_inactive_selection_text)|(1<<
				colour_background)|(1<<colour_selection_background)|(1<<colour_inactive_selection_background)|(1<<
				colour_active_item_frame)|(1<<colour_group_foreground)|(1<<colour_group_background),

		};
		enum bool_identifier_t
		{
			bool_use_custom_active_item_frame,
		};
		enum bool_flag_t
		{
			bool_flag_use_custom_active_item_frame = (1<<bool_use_custom_active_item_frame),
		};
		enum colour_mode_t
		{
			colour_mode_global,
			colour_mode_system,
			colour_mode_themed,
			colour_mode_custom
		};

		static COLORREF g_get_system_color(const colour_identifier_t p_identifier)
		{
			switch (p_identifier)
			{
			case colour_text:
				return GetSysColor(COLOR_WINDOWTEXT);
			case colour_selection_text:
				return GetSysColor(COLOR_HIGHLIGHTTEXT);
			case colour_background:
				return GetSysColor(COLOR_WINDOW);
			case colour_selection_background:
				return GetSysColor(COLOR_HIGHLIGHT);
			case colour_inactive_selection_text:
				return GetSysColor(COLOR_BTNTEXT);
			case colour_inactive_selection_background:
				return GetSysColor(COLOR_BTNFACE);
			case colour_active_item_frame:
				return GetSysColor(COLOR_WINDOWFRAME);
			default:
				return 0;
			}
		}

		/** One implementation in Columns UI - do not reimplement! */
		class NOVTABLE manager_instance : public service_base
		{
		public:
			/** \brief Get the specified colour */
			virtual COLORREF get_colour(const colour_identifier_t & p_identifier) const =0;
			/** \brief Get the specified colour */
			virtual bool get_bool(const bool_identifier_t & p_identifier) const =0;

			/**
			* Only returns true if your appearance_client::get_themes_supported does.
			* Indicates selected items should be drawn using Theme API.
			*/
			virtual bool get_themed() const =0;

			FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(manager_instance);
		};
		
		/**
		* Use this class if you wish to use the global colours only rather than implementing the client class
		*/
		class NOVTABLE common_callback
		{
		public:
			virtual void on_colour_changed(t_size mask) const =0;
			virtual void on_bool_changed(t_size mask) const =0;
		};

		/**
		* One implementation in Columns UI - do not reimplement! 
		*
		* It is not recommended to use this class directly - use the helper class instead. 
		*/
		class NOVTABLE manager : public service_base
		{
		public:
			/** \brief Creates a manager_instance for the given client (null GUID implies global settings). */
			virtual void create_instance (const GUID & p_client_guid, cui::colours::manager_instance::ptr & p_out)=0;

			virtual void register_common_callback (common_callback * p_callback) {};
			virtual void deregister_common_callback (common_callback * p_callback) {};

			FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(manager);
		};

		/** Helper to simplify retrieving colours. */
		class helper
		{
		public:
			COLORREF get_colour(const cui::colours::colour_identifier_t & p_identifier) const
			{
				if (m_api.is_valid())
				{
					return m_api->get_colour(p_identifier);
				}
				return g_get_system_color(p_identifier);
			}
			bool get_bool(const cui::colours::bool_identifier_t & p_identifier) const
			{
				if (m_api.is_valid())
				{
					return m_api->get_bool(p_identifier);
				}
				return false;
			}
			bool get_themed() const
			{
				if (m_api.is_valid())
				{
					return m_api->get_themed();
				}
				return false;
			}
			/** You can specify a NULL GUID for the global colours */
			helper(GUID p_guid)
			{
				try
				{
					manager::ptr api;
					standard_api_create_t(api);
					api->create_instance(p_guid, m_api);
				}
				catch (const exception_service_not_found &) {};
			}
		private:
			service_ptr_t<cui::colours::manager_instance> m_api;
		};

		class NOVTABLE client : public service_base
		{
		public:
			virtual const GUID & get_client_guid() const = 0;
			virtual void get_name (pfc::string_base & p_out) const = 0;

			virtual t_size get_supported_colours() const {return cui::colours::colour_flag_all & ~ cui::colours::colour_flag_group_foreground|cui::colours::colour_flag_group_background;}; //bit-mask
			virtual t_size get_supported_bools() const = 0; //bit-mask
			/** Indicates whether you are Theme API aware and can draw selected items using Theme API */
			virtual bool get_themes_supported() const = 0;

			virtual void on_colour_changed(t_size mask) const =0;
			virtual void on_bool_changed(t_size mask) const =0;

			template <class tClass>
			class factory : public service_factory_t<tClass> {};

			FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(client);
		};
	};
	namespace fonts
	{
		enum font_mode_t
		{
			font_mode_common_items,
			font_mode_common_labels,
			font_mode_custom,
			font_mode_system,
		};

		enum font_type_t
		{
			font_type_items,
			font_type_labels,
		};
		enum font_type_flag_t
		{
			font_type_flag_items = (1<<0),
			font_type_flag_labels = (1<<1),
		};

		/**
		* Use this class if you wish to use the common fonts rather than implementing client
		*/
		class NOVTABLE common_callback
		{
		public:
			virtual void on_font_changed(t_size mask) const = 0;
		};

		/** One implementation in Columns UI - do not reimplement! */
		class NOVTABLE manager : public service_base
		{
		public:
			/** \brief Retreives the font for the given client */
			virtual void get_font(const GUID & p_guid, LOGFONT & p_out) const = 0;

			/** \brief Retrieves common fonts. */
			virtual void get_font(const font_type_t p_type, LOGFONT & p_out) const = 0;
			/** \brief Sets your font as 'Custom' and to p_font */
			virtual void set_font(const GUID & p_guid, const LOGFONT & p_font) = 0;

			virtual void register_common_callback (common_callback * p_callback)=0;
			virtual void deregister_common_callback (common_callback * p_callback)=0;

			/** Helper */
			HFONT get_font(const GUID & p_guid) const
			{
				LOGFONT lf;
				memset(&lf, 0, sizeof(LOGFONT));
				get_font(p_guid, lf);
				return CreateFontIndirect(&lf);
			}

			/** Helper */
			HFONT get_font(const font_type_t p_type) const
			{
				LOGFONT lf;
				memset(&lf, 0, sizeof(LOGFONT));
				get_font(p_type, lf);
				return CreateFontIndirect(&lf);
			}

			FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(manager);
		};
		
		/** Helper to simplify retrieving the font for a specific client. */
		class helper
		{
		public:
			void get_font(LOGFONT & p_out) const
			{
				if (m_api.is_valid())
				{
					m_api->get_font(m_guid, p_out);
				}
				else
					uGetIconFont(&p_out);
			}
			HFONT get_font() const
			{
				if (m_api.is_valid())
				{
					return m_api->get_font(m_guid);
				}
				return uCreateIconFont();
			}
			helper(GUID p_guid) : m_guid(p_guid)
			{
				try
				{
					standard_api_create_t(m_api);
				}
				catch (const exception_service_not_found &) {};
			}
		private:
			service_ptr_t<manager> m_api;
			GUID m_guid;
		};
		class NOVTABLE client : public service_base
		{
		public:
			virtual const GUID & get_client_guid() const = 0;
			virtual void get_name (pfc::string_base & p_out) const = 0;

			virtual font_type_t get_default_font_type() const = 0;

			virtual void on_font_changed() const = 0;

			template <class tClass>
			class factory : public service_factory_t<tClass> {};

			FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(client);

		public:
			static bool create_by_guid (const GUID & p_guid, class ptr & p_out);
		};
	};
};

#endif