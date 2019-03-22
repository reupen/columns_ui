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

enum playlist_filter_type {
    FILTER_NONE = 0,
    FILTER_SHOW,
    FILTER_HIDE,
};

enum alignment {
    ALIGN_LEFT,
    ALIGN_CENTRE,
    ALIGN_RIGHT,
};

namespace pfc {
template <>
class traits_t<playlist_filter_type> : public traits_rawobject {
};
template <>
class traits_t<alignment> : public traits_rawobject {
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

void g_save_playlist(HWND wnd, const pfc::list_base_const_t<metadb_handle_ptr>& p_items, const char* name);

#endif
