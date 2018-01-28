#ifndef _COLUMNS_HELPERS_H_
#define _COLUMNS_HELPERS_H_

/*!
 * \file common.h
 *
 * \author musicmusic
 * \date 1 March 2015
 *
 * Some common functions and enumerations
 */

template <typename type_t>
class ptr_list_autodel_t : public pfc::ptr_list_t<type_t> {
public:
    ~ptr_list_autodel_t() { this->delete_all(); }
};

enum playlist_filter_type {
    FILTER_NONE = 0,
    FILTER_SHOW,
    FILTER_HIDE,
};

enum {
    LIBPNG_NOTFOUND,
    LIBPNG_UNKNOWNVERSION,
    LIBPNG_FOUND,
};

enum colour_type {
    COLOUR_FORE = 0,
    COLOUR_FORE_SELECTED,
    COLOUR_BACK,
    COLOUR_BACK_SELECTED,
    COLOUR_BACK_SELECTED_NO_FOCUS,
    COLOUR_TEXT_SELECTED_NO_FOCUS,
};

enum string_type {
    STRING_NAME = 0,
    STRING_DISPLAY,
    STRING_SORT,
    STRING_COLOUR,
    STRING_FILTER,
    STRING_EDIT_FIELD,
};

enum bool_type {
    BOOL_CUSTOM_SORT = 0,
    BOOL_CUSTOM_COLOUR,
    BOOL_SHOW,
};

enum long_type {
    LONG_WIDTH = 0,
    LONG_PARTS,
};

enum alignment {
    ALIGN_LEFT,
    ALIGN_CENTRE,
    ALIGN_RIGHT,
};

enum column_data {
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

enum config_data {
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
    CONFIG_USE_OLD_GLOBAL, // old old

    CONFIG_INCLUDE_DATE,
    CONFIG_MAP_COLOUR_CODES,

    CONFIG_COLOUR_PLIST_SELECTED_TEXT,
    CONFIG_COLOUR_PLIST_SELECTED_FRAME,
    CONFIG_COLOUR_PLIST_SELECTED_BACK,
    CONFIG_COLOUR_PLIST_SELECTED_BACK_NO_FOCUS,
    CONFIG_USE_GLOBAL,

    /*v4*/ /* TODO */
    CONFIG_COLOUR_PLIST_INACTIVE_SELECTED_TEXT,

    CONFIG_USE_CUSTOM_COLOURS,
    CONFIG_USE_SYSTEM_FOCUSED_ITEM_FRAME,
    CONFIG_COLOUR_TEXT,
    CONFIG_COLOUR_SELECTED_TEXT,
    CONFIG_COLOUR_SELECTED_BACK,
    CONFIG_COLOUR_INACTIVE_SELECTED_BACK,
    CONFIG_COLOUR_INACTIVE_SELECTED_TEXT,

};

namespace pfc {
template <>
class traits_t<playlist_filter_type> : public traits_rawobject {
};
template <>
class traits_t<alignment> : public traits_rawobject {
};
template <>
class traits_t<column_data> : public traits_rawobject {
};
template <>
class traits_t<config_data> : public traits_rawobject {
};
} // namespace pfc

const char* strchr_n(const char* src, char c, unsigned len = -1);

struct colour_bytes {
    BYTE B;
    BYTE G;
    BYTE R;
};

struct colour {
    BYTE B{0};
    BYTE G{0};
    BYTE R{0};

    void set(COLORREF new_colour);
    operator COLORREF() const { return RGB(R, G, B); }
};

inline bool operator==(const colour& c1, const colour& c2)
{
    return (c1.B == c2.B && c1.G == c2.G && c1.R == c2.R);
}

class string_pn : public pfc::string8 {
public:
    string_pn(metadb_handle_list_cref handles, const char* format, const char* def = "Untitled");
};

void set_sel_single(int idx, bool toggle, bool focus, bool single_only);
void set_sel_range(int start, int end, bool keep, bool deselect = false);

UINT GetNumScrollLines();

void g_save_playlist(HWND wnd, const pfc::list_base_const_t<metadb_handle_ptr>& p_items, const char* name);

#endif
