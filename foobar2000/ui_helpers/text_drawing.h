#ifndef _TEXT_DRAWING_H_COLUMNS_
#define _TEXT_DRAWING_H_COLUMNS_

unsigned strtoul_n(const char * p_val, unsigned p_val_length, unsigned base=10);
t_int64 strtol64_n(const char * p_val, unsigned p_val_length, unsigned base= 10);
t_uint64 strtoul64_n(const char * p_val, unsigned p_val_length, unsigned base= 10);
template <typename char_t>
unsigned strtoul_t(const char_t * p_val, unsigned p_val_length, unsigned base = 10);

namespace ui_helpers {

class ScriptString_instance
{
public:
	ScriptString_instance()
	{
		initialise();
	}
	~ScriptString_instance() {cleanup();}

	ScriptString_instance(HDC dc, const wchar_t * p_str, size_t p_str_len, int max_cx, bool b_clip)
	{
		initialise();
		analyse (dc, p_str, p_str_len, max_cx, b_clip);
	}

	ScriptString_instance(HDC dc, const char * p_str, size_t p_str_len, int max_cx, bool b_clip)
	{
		initialise();
		analyse (dc, p_str, p_str_len, max_cx, b_clip);
	}

	void analyse(HDC dc, const wchar_t * p_str, size_t p_str_len, int max_cx, bool b_clip)
	{
//		profiler(analyse);
		if (m_ssa)
		{
			cleanup();
			initialise();
		}
		m_string_length = p_str_len;
		if (m_string_length)
			ScriptStringAnalyse(dc, p_str, p_str_len, NULL, -1, SSA_FALLBACK|SSA_GLYPHS|SSA_LINK|(b_clip ? SSA_CLIP : NULL), max_cx, &m_sc, &m_ss, NULL, NULL, NULL, &m_ssa);
	}
	void analyse(HDC dc, const char * p_str, size_t p_str_len, int max_cx, bool b_clip)
	{
		pfc::stringcvt::string_wide_from_utf8 wstr(p_str, p_str_len);
		analyse(dc, wstr.get_ptr(), wstr.length(), max_cx, b_clip);
	}
	void text_out (int x, int y, UINT flags, const RECT * p_rc)
	{
//		profiler(text_out);
		if (m_ssa)
			ScriptStringOut (m_ssa, x, y, flags, p_rc, 0, 0, FALSE);
	}
	int get_output_character_count() 
	{
		const int * len = m_ssa ? ScriptString_pcOutChars(m_ssa) : NULL; //b_clip = true only
		return len ? *len : m_string_length;
	}
	void get_output_size(SIZE & p_sz)
	{
//		profiler(get_output_size);
		const SIZE * sz = m_ssa ? ScriptString_pSize(m_ssa) : NULL;
		if (sz) p_sz = *sz;
		else {p_sz.cx = (p_sz.cy = 0);}
	}
	int get_output_width()
	{
		SIZE sz;
		get_output_size(sz);
		return sz.cx;
	}
	void get_character_logical_widths(int * p_array_out)
	{
//		profiler(get_character_logical_widths);
		if (m_ssa)
			ScriptStringGetLogicalWidths(m_ssa, p_array_out);
	}
	void get_character_logical_extents(int * p_array_out, int offset = 0) //N.B RTL !
	{
		get_character_logical_widths(p_array_out);
		int i, count = get_output_character_count();
		for (i=1; i<count; i++)
		{
			p_array_out[i] += p_array_out[i-1];
		}
		if (offset)
			for (i=0; i<count; i++)
				p_array_out[i] += offset;
	}
private:
	void initialise()
	{
		{
//			profiler(initialise_a);
			m_ssa = NULL;
			m_string_length = NULL;
			memset(&m_sc, 0, sizeof(m_sc));
			memset(&m_ss, 0, sizeof(m_ss));
		}

		{
//			profiler(initialise_b);
			if (!m_sdg_valid)
			{
				ScriptRecordDigitSubstitution(LOCALE_USER_DEFAULT, &m_sdg);
				m_sdg_valid = true;
			}
			ScriptApplyDigitSubstitution(&m_sdg, &m_sc, &m_ss);
		}
	}
	void cleanup()
	{
//		profiler(cleanup);
		if (m_ssa)
		{
			ScriptStringFree(&m_ssa);
			m_ssa = NULL;
		}
	}

	SCRIPT_STRING_ANALYSIS m_ssa;
	SCRIPT_CONTROL m_sc;
	SCRIPT_STATE m_ss;
	size_t m_string_length;

	static SCRIPT_DIGITSUBSTITUTE m_sdg;
	static bool m_sdg_valid;
};

class uGetTextExtentExPoint_helper
{
	pfc::string8_fast_aggressive temp;
	pfc::stringcvt::string_wide_from_utf8_fast text_w;
	pfc::stringcvt::string_utf8_from_wide w_utf8;
public:
	uGetTextExtentExPoint_helper() {temp.prealloc(64);}
	BOOL run(HDC dc, const char * text, int length, int max_width, LPINT max_chars, LPINT width_array, LPSIZE sz, unsigned * width_out, bool trunc);
};
	
//BOOL uGetTextExtentExPoint(HDC dc, const char * text, int length, int max_width, LPINT max_chars, LPINT width_array, LPSIZE sz, unsigned & width_out, bool trunc = true);
//BOOL uGetTextExtentExPoint2(HDC dc, const char * text, int length, int max_width, LPINT max_chars, LPINT width_array, LPSIZE sz, unsigned * width_out = NULL, bool trunc = true);
enum alignment
{
	ALIGN_LEFT,
	ALIGN_CENTRE,
	ALIGN_RIGHT,
};
bool is_rect_null(const RECT * r);
void get_text_size(HDC dc,const char * src,int len, SIZE & sz);
int get_text_width(HDC dc,const char * src,int len);
int get_text_width_color(HDC dc,const char * src,int len, bool b_ignore_tabs = false);
int get_text_width_color_legacy(HDC dc,const char * src,int len, bool b_ignore_tabs = false);
unsigned get_trunc_len(const char * src, unsigned len);
BOOL text_out_colours_ellipsis(HDC dc,const char * src,int len,int x_offset,int pos_y,const RECT * base_clip,bool selected,bool show_ellipsis,DWORD default_color,alignment align, unsigned * p_width = NULL, bool b_set_default_colours = true, unsigned * p_position = NULL);
BOOL text_out_colours_tab(HDC dc,const char * display,int display_len,int left_offset,int border,const RECT * base_clip,bool selected,DWORD default_color,bool columns,bool tab,bool show_ellipsis,alignment align, unsigned * p_width = NULL, bool b_set_default_colours = true, bool b_vertical_align_centre = true, unsigned * p_position = NULL);
};

#endif