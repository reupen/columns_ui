#include "stdafx.h"
#include "common.h"

const char* strchr_n(const char* src, char c, unsigned len)
{
    const char* ptr = src;
    const char* start = ptr;
    while (*ptr && ((unsigned)(ptr - start) < len)) {
        if (*ptr == c)
            return ptr;
        ptr++;
    }
    return nullptr;
}

void Colour::set(COLORREF new_colour)
{
    B = LOBYTE(HIWORD(new_colour));
    G = HIBYTE(LOWORD(new_colour));
    R = LOBYTE(LOWORD(new_colour));
}

StringFormatCommonTrackTitle::StringFormatCommonTrackTitle(metadb_handle_list_cref handles, const char* format, const char* def)
{
    pfc::string8_fast_aggressive a;
    pfc::string8_fast_aggressive b;
    a.prealloc(512);
    b.prealloc(512);
    unsigned count = handles.get_count();
    bool use = false;

    pfc::ptr_list_t<char> specs;

    const char* ptr = format;
    while (*ptr) {
        const char* start = ptr;
        while (*ptr && *ptr != '\\')
            ptr++;
        if (ptr > start)
            specs.add_item(pfc::strdup_n(start, ptr - start));
        while (*ptr == '\\')
            ptr++;
    }

    unsigned fmt_count = specs.get_count();

    for (unsigned f = 0; f < fmt_count; f++) {
        service_ptr_t<titleformat_object> to_temp;
        static_api_ptr_t<titleformat_compiler>()->compile_safe(to_temp, specs[f]);
        for (unsigned n = 0; n < count; n++) {
            if (n == 0) {
                handles[0]->format_title(nullptr, a, to_temp, nullptr);
                use = true;
            } else {
                handles[n]->format_title(nullptr, b, to_temp, nullptr);
                if (strcmp(a, b) != 0) {
                    use = false;
                    break;
                }
            }
        }

        if (use)
            break;
    }

    specs.free_all();

    if (use)
        set_string(a);
    else
        set_string(def);
}

void g_save_playlist(HWND wnd, const pfc::list_base_const_t<metadb_handle_ptr>& p_items, const char* p_name)
{
    pfc::string8 name = p_name;
    // name << p_name;

    pfc::string_formatter ext;
    service_enum_t<playlist_loader> e;
    service_ptr_t<playlist_loader> ptr;
    unsigned def_index = 0;
    unsigned n = 0;

    while (e.next(ptr)) {
        if (ptr->can_write()) {
            ext << ptr->get_extension() << " files|*." << ptr->get_extension() << "|";
            if (!stricmp_utf8(ptr->get_extension(), "fpl"))
                def_index = n;
            n++;
        }
    }
    if (uGetOpenFileName(wnd, ext, def_index, "fpl", "Save playlist...", nullptr, name, TRUE)) {
        try {
            abort_callback_dummy p_abort;
            playlist_loader::g_save_playlist(name, p_items, p_abort);
        } catch (pfc::exception& e) {
            popup_message::g_show(e.what(), "Error writing playlist", popup_message::icon_error);
        }
    }
}
