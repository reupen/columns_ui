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

enum PlaylistFilterType {
    FILTER_NONE = 0,
    FILTER_SHOW,
    FILTER_HIDE,
};

enum Alignment {
    ALIGN_LEFT,
    ALIGN_CENTRE,
    ALIGN_RIGHT,
};

namespace pfc {
template <>
class traits_t<PlaylistFilterType> : public traits_rawobject {
};
template <>
class traits_t<Alignment> : public traits_rawobject {
};
} // namespace pfc

const char* strchr_n(const char* src, char c, unsigned len = -1);

struct Colour {
    BYTE B{0};
    BYTE G{0};
    BYTE R{0};

    void set(COLORREF new_colour);
    operator COLORREF() const { return RGB(R, G, B); }
};

inline bool operator==(const Colour& c1, const Colour& c2)
{
    return (c1.B == c2.B && c1.G == c2.G && c1.R == c2.R);
}

class StringFormatCommonTrackTitle : public pfc::string8 {
public:
    StringFormatCommonTrackTitle(metadb_handle_list_cref handles, const char* format, const char* def = "Untitled");
};

void g_save_playlist(HWND wnd, const pfc::list_base_const_t<metadb_handle_ptr>& p_items, const char* name);

#endif
