#ifndef _COLUMNS_HELPERS_H_
#define _COLUMNS_HELPERS_H_


#include <DelayImp.h>

#ifdef _DEBUG
#define profiler_debug(x) profiler(x)
#else
#define profiler_debug(x)
#endif

template <typename type_t>
class ptr_list_autodel_t : public pfc::ptr_list_t < type_t >
{
public:
	~ptr_list_autodel_t()
	{
		delete_all();
	}
};

class format_win32_delayload_error {
public:
	format_win32_delayload_error(PDelayLoadInfo pdli)
	{
		if (pdli)
		{
			if (pdli->szDll)
				m_buffer << "Delay load failure - Module: " << pdli->szDll;
			if (!pdli->dlp.fImportByName)
				m_buffer << ", Ordinal: " << pfc::format_hex(pdli->dlp.dwOrdinal) << "h";
			else if (pdli->dlp.szProcName)
				m_buffer << ", Procedure: " << pdli->dlp.szProcName;

			pfc::string8 temp;
			m_buffer << ", Error: ";
			if (uFormatSystemErrorMessage(temp,pdli->dwLastError))
			{
				m_buffer << temp;
			}
			else
				m_buffer << (unsigned)pdli->dwLastError;
		}
	}

	const char * get_ptr() const {return m_buffer.get_ptr();}
	operator const char*() const {return m_buffer.get_ptr();}
private:
	pfc::string8 m_buffer;
};


class exception_delayload : public pfc::exception {
public:
	exception_delayload(PDelayLoadInfo pdli) : pfc::exception(format_win32_delayload_error(pdli)) {}
};

class exception_delayload_module_not_found : public exception_delayload {
public:
	exception_delayload_module_not_found(PDelayLoadInfo pdli) : exception_delayload(pdli) {}
};

class exception_delayload_procedure_not_found : public exception_delayload {
public:
	exception_delayload_procedure_not_found(PDelayLoadInfo pdli) : exception_delayload(pdli) {}
};

namespace types
{
struct t_guid : public GUID
{
public:
	void reset()
	{
		*this = pfc::guid_null;
	}
	t_guid()
		: GUID(pfc::guid_null) {}
	t_guid(const GUID & p_guid)
		: GUID(p_guid) {}
	t_guid(const t_guid & p_guid)
		: GUID(p_guid) {}
};
}

enum playlist_filter_type
{
	FILTER_NONE = 0,
	FILTER_SHOW,
	FILTER_HIDE,
};

enum
{
	LIBPNG_NOTFOUND,
	LIBPNG_UNKNOWNVERSION,
	LIBPNG_FOUND,
};

enum colour_type
{
	COLOUR_FORE = 0,
	COLOUR_FORE_SELECTED,
	COLOUR_BACK,
	COLOUR_BACK_SELECTED,
	COLOUR_BACK_SELECTED_NO_FOCUS,
	COLOUR_TEXT_SELECTED_NO_FOCUS,
};

enum string_type
{
	STRING_NAME = 0,
	STRING_DISPLAY,
	STRING_SORT,
	STRING_COLOUR,
	STRING_FILTER,
	STRING_EDIT_FIELD,
};

enum bool_type
{
	BOOL_CUSTOM_SORT = 0,
	BOOL_CUSTOM_COLOUR,
	BOOL_SHOW,
};

enum long_type
{
	LONG_WIDTH = 0,
	LONG_PARTS,
};

enum alignment
{
	ALIGN_LEFT,
	ALIGN_CENTRE,
	ALIGN_RIGHT,
};

enum column_data
{
	COLUMN_END = -1,
	COLUMN_NAME = 0,
	COLUMN_SPEC,
	COLUMN_USE_COLOUR,
	COLUMN_COLOUR,
	COLUMN_USE_SORT,
	COLUMN_SORT,
	COLUMN_WIDTH,
	COLUMN_ALIGNMENT,
	COLUMN_FILTER_TYPE,
	COLUMN_FILTER,

	/*v2*/
	COLUMN_RESIZE,

	/*v3*/
	COLUMN_SHOW,

	/*v4*/
	COLUMN_EDIT_FIELD,
};

enum config_data
{
	CONFIG_END = -1,
	CONFIG_COLUMN = 0,
	CONFIG_USE_LEGACY_GLOBAL,
	CONFIG_GLOBAL,
	CONFIG_COLOUR,
	CONFIG_SHOW_HEADER,
	CONFIG_SHOW_PLIST,
	CONFIG_SHOW_TABS,
	CONFIG_COLOUR_BACK,
	CONFIG_COLOUR_FRAME,
	CONFIG_COLOUR_PLIST_FORE,
	CONFIG_COLOUR_PLIST_BACK,
	CONFIG_COLOUR_VIS_FORE,
	CONFIG_COLOUR_VIS_BACK,
	CONFIG_FONT_PLAYLIST,
	CONFIG_FONT_HEADER,
	CONFIG_FONT_STATUS,
	CONFIG_FONT_PLIST,
	CONFIG_FB2K_STATUS,
	CONFIG_FB2K_SYSTRAY,
	CONFIG_FB2K_WTITLE,

	/*v2*/
	CONFIG_NOHSCROLL,
	CONFIG_USE_GLOBAL_SORT,
	CONFIG_HEIGHT,
	CONFIG_PLHEIGHT,

	/*v3 +*/
	CONFIG_USE_OLD_GLOBAL,//old old

	CONFIG_INCLUDE_DATE,
	CONFIG_MAP_COLOUR_CODES,

	CONFIG_COLOUR_PLIST_SELECTED_TEXT,
	CONFIG_COLOUR_PLIST_SELECTED_FRAME,
	CONFIG_COLOUR_PLIST_SELECTED_BACK,
	CONFIG_COLOUR_PLIST_SELECTED_BACK_NO_FOCUS,
	CONFIG_USE_GLOBAL,

	
	/*v4*/  /* TODO */
	CONFIG_COLOUR_PLIST_INACTIVE_SELECTED_TEXT,

	CONFIG_USE_CUSTOM_COLOURS,
	CONFIG_USE_SYSTEM_FOCUSED_ITEM_FRAME,
	CONFIG_COLOUR_TEXT,
	CONFIG_COLOUR_SELECTED_TEXT,
	CONFIG_COLOUR_SELECTED_BACK,
	CONFIG_COLOUR_INACTIVE_SELECTED_BACK,
	CONFIG_COLOUR_INACTIVE_SELECTED_TEXT,

};

namespace pfc
{
	template<> class traits_t<playlist_filter_type> : public traits_rawobject {};
	template<> class traits_t<alignment> : public traits_rawobject {};
	template<> class traits_t<column_data> : public traits_rawobject {};
	template<> class traits_t<config_data> : public traits_rawobject {};
}


const char * strchr_n(const char * src, char c, unsigned len = -1);

struct create_guid : public GUID
{
	create_guid(t_uint32 p_data1, t_uint16 p_data2, t_uint16 p_data3, t_uint8 p_data41, t_uint8 p_data42, t_uint8 p_data43, t_uint8 p_data44, t_uint8 p_data45, t_uint8 p_data46, t_uint8 p_data47, t_uint8 p_data48) 
	{
		Data1 = p_data1;
		Data2 = p_data2;
		Data3 = p_data3;
		Data4[0] = p_data41;
		Data4[1] = p_data42;
		Data4[2] = p_data43;
		Data4[3] = p_data44;
		Data4[4] = p_data45;
		Data4[5] = p_data46;
		Data4[6] = p_data47;
		Data4[7] = p_data48;
	}
};

struct colour_bytes {BYTE B; BYTE G; BYTE R;};

struct colour
{

	BYTE B;
	BYTE G;
	BYTE R;
//public:
	colour() : R(0),G(0),B(0) {}
	void set(COLORREF new_colour);
	inline operator COLORREF () const
	{
		return RGB(R, G, B);
	}
};

	inline bool operator==(const colour & c1, const  colour &c2) 
	{
		return (c1.B == c2.B && c1.G == c2.G && c1.R == c2.R);
	}

class string_pn: public pfc::string8
{
public:
	string_pn(metadb_handle_list_cref handles, const char * format, const char * def = "Untitled");
};

__int64 get_timestamp();

class performance_counter_v2
{
	__int64 startc, endc, diff,count;
public:
	performance_counter_v2() :  startc(0), endc(0), diff(0), count(0) {};
	__inline void start()
	{startc = get_timestamp();};
	__inline void stop()
	{endc = get_timestamp();diff+=(endc-startc);count++;}
	void reset()
	{diff = 0;count=0;}
	__int64 get_cycles()
	{return diff;}
	__int64 get_mean_cycles()
	{return count ? (diff/count) : 0;}
};


class performance_counter
{
	bool supported;
	__int64 interval, a, b, diff,count;
public:
	performance_counter() : interval(0), a(0), b(0), diff(0), count(0) {supported = (QueryPerformanceFrequency((LARGE_INTEGER*)&interval) != 0);};
	bool get_supported()
	{return supported;}
	void start()
	{QueryPerformanceCounter((LARGE_INTEGER*)&a);};
	void stop()
	{QueryPerformanceCounter((LARGE_INTEGER*)&b); diff += (b-a);count++;}
	void reset()
	{diff = 0;count=0;}
	__int64 get_diff()
	{return diff;}
	double get_diff_seconds()
	{if (count) return (double)diff / (double)(interval*count); else return 0;}
	double get_diff_milliseconds()
	{if (count) return (double)(diff*1000) / (double)(interval*count); else return 0;}
	double get_diff_nanoseconds()
	{if (count) return (double)(diff*1000000000) / (double)(interval*count); else return 0;}
};

void set_sel_single(int idx, bool toggle, bool focus, bool single_only);
void set_sel_range(int start, int end, bool keep, bool deselect=false);

int rebar_id_to_idx(HWND wnd, unsigned id);
void rebar_show_all_bands(HWND wnd);

UINT GetNumScrollLines();

class menu_item_identifier
{
public:
	types::t_guid m_command;
	types::t_guid m_subcommand;
	inline const menu_item_identifier & operator=(const menu_item_identifier & p_source) {m_command = p_source.m_command;m_subcommand = p_source.m_subcommand;return *this;}
	menu_item_identifier(){};
	menu_item_identifier(const GUID & p_val, const GUID & psub = pfc::guid_null)
		: m_command(p_val), m_subcommand(psub){};
};

bool operator==(const menu_item_identifier & p1, const menu_item_identifier & p2);
bool operator!=(const menu_item_identifier & p1, const menu_item_identifier & p2);

class menu_item_cache
{
	class menu_item_info : public menu_item_identifier
	{
	public:
		pfc::string8 m_name;
		pfc::string8 m_desc;
	};
public:
	menu_item_cache();
	const menu_item_info & get_item(unsigned n) const;
	inline unsigned get_count()
	{
		return m_data.get_count();
	}
private:
	pfc::ptr_list_t<menu_item_info> m_data;
};

void populate_menu_combo(HWND wnd, unsigned ID, unsigned ID_DESC, const menu_item_identifier & p_item, menu_item_cache & p_cache, bool insert_none);
void on_menu_combo_change(HWND wnd, LPARAM lp, class cfg_menu_item & cfg_menu_store, menu_item_cache & p_cache, unsigned ID_DESC) ;

void g_save_playlist(HWND wnd, const pfc::list_base_const_t<metadb_handle_ptr> & p_items, const char * name);

#endif
