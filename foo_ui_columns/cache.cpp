#include "stdafx.h"
#include "cache.h"

cfg_string cfg_global_nontrack(GUID{0x6c012fdb, 0x30a4, 0x398d, 0x16, 0xf1, 0x75, 0x27, 0x99, 0x77, 0xc9, 0x8f}, "");
cache_manager g_cache_manager;

#if 0
class variable_cache_manager
{
    class variable
    { 
        pfc::string_simple name;
        pfc::string_simple value;
    };
    ptr_list_autodel_t<variable> settings,variables;
    void parse_string(const char * pptr)
    {
        const char * p_str = pptr;
        pfc::string8 temp;
        if (strchr(pptr,'$') || strchr(pptr,'%') || strchr(pptr,'#'))
        {
            titleformat_compiler::g_run(&file_info_i_full(), temp, pptr, 0);
            p_str = temp;
        }
        const char * ptr = p_str;
        const char * start = ptr;
        while(*ptr)
        {
            start = ptr;
            while (*ptr && *ptr != '\x07') ptr++;
            if (ptr > start)
            {
                const char * col = strchr_n(start, ':', ptr-start);
                if (col)
                {
                    if (stricmp_utf8_partial(start, "variables:"))
                    {
    //                    variables.add_item(new(std::nothrow) pfc::string_simple(col+1, ptr-col));
                    }
                }
            }
            while (*ptr && *ptr == '\x07') ptr++;
        }

    };
};
#endif

static const char* g_str_error = "Error";

colourinfo* cache_manager::add_colour(
    /*COLORREF text, COLORREF text_sel, COLORREF back, COLORREF back_sel, COLORREF back_sel_nofocus*/ colourinfo&
        colour_add)
{
    unsigned count = colours.get_count();
    for (unsigned n = 0; n < count; n++) {
        colourinfo* item = colours[n];

        if (*item == colour_add) {
            item->add_ref();
            return item;
        }
    }
    auto new_colourinfo = new colourinfo(colour_add);

    unsigned i = colours.add_item(new_colourinfo);
    //    console::info(pfc::string_printf("colour count: %u",i+1));
    return new_colourinfo;
}

void cache_manager::release_colour(colourinfo* col)
{
    unsigned count = colours.get_count();
    for (unsigned n = 0; n < count; n++) {
        colourinfo* item = colours[n];
        if (item == col) {
            if (item->release())
                colours.delete_by_idx(n);
            break;
        }
    }
}

void display_info::set_display(const char* new_display)
{
    display = new_display;
}
void display_info::get_display(pfc::string_base& out)
{
    out.set_string(display);
}

void display_info::get_colour(colourinfo& out)
{
    if (colours)
        colours->copy_to(out);
};

COLORREF display_info::get_colour(colour_type type)
{
    if (colours) {
        switch (type) {
        case COLOUR_FORE:
            return colours->text_colour;
        case COLOUR_FORE_SELECTED:
            return colours->selected_text_colour;
        case COLOUR_BACK:
            return colours->background_colour;
        case COLOUR_BACK_SELECTED:
            return colours->selected_background_colour;
        case COLOUR_TEXT_SELECTED_NO_FOCUS:
            return colours->selected_text_colour_non_focus;
        case COLOUR_BACK_SELECTED_NO_FOCUS:
            return colours->selected_background_colour_non_focus;
        }
    }
    return 0x0000FF;
}
void display_info::set_colour(
    /*COLORREF text, COLORREF text_sel, COLORREF back, COLORREF back_sel, COLORREF back_sel_nofocus*/ colourinfo&
        colour_add)
{
    if (colours)
        g_cache_manager.release_colour(colours);
    colours = g_cache_manager.add_colour(colour_add);

    /*        text_colour.set(a);
            selected_text_colour.set(b);
            background_colour.set(c);
            selected_background_colour.set(d);*/
}

void playlist_entry_ui::add_display_items(unsigned count)
{
    //    display_info * display = new(std::nothrow) display_info;
    //    display_data.add_item(display);
    if (display_data) {
        delete[] display_data;
        display_data = nullptr;
    }
    display_data = new display_info[count];
}
void playlist_entry_ui::set_display_item(int column, const char* data, colourinfo& colour_add)
{
    //    display_data.get_item(column)->set_display(data);
    display_data[column].set_display(data);

    display_data[column].set_colour(colour_add);
    //    display_data.get_item(column)->set_colour( strtoul(fore, 0, 16),strtoul(fore_sel, 0, 16),strtoul(back, 0,
    //    16),strtoul(back_sel, 0, 16),strtoul(back_sel_no_focus, 0, 16));
}
display_info* playlist_entry_ui::get_item(unsigned col) const
{
    // return display_data.get_item(col);
    return &display_data[col];
}

playlist_entry_ui::~playlist_entry_ui()
{
    delete[] display_data;
}

void process_colour_string(const char* src, colourinfo& out)
{
    const char* ptr = src;
    if (*ptr && *ptr == 3)
        ptr++;

    const char* start = ptr;
    while (*ptr && *ptr != 3 && *ptr != '|')
        ptr++;
    if (ptr > start) {
        out.text_colour.set(strtoul(start, nullptr, 16));
    }
    if (*ptr && *ptr == 3)
        ptr++;
    if (*ptr && *ptr == '|')
        ptr++;
    if (*ptr && *ptr == 3)
        ptr++;

    start = ptr;
    while (*ptr && *ptr != 3 && *ptr != '|')
        ptr++;
    if (ptr > start) {
        out.selected_text_colour.set(strtoul(start, nullptr, 16));
        out.selected_text_colour_non_focus.set(out.selected_text_colour);
    }
    if (*ptr && *ptr == 3)
        ptr++;
    if (*ptr && *ptr == '|')
        ptr++;
    if (*ptr && *ptr == 3)
        ptr++;

    start = ptr;
    while (*ptr && *ptr != 3 && *ptr != '|')
        ptr++;
    if (ptr > start) {
        out.background_colour.set(strtoul(start, nullptr, 16));
    }

    if (*ptr && *ptr == 3)
        ptr++;
    if (*ptr && *ptr == '|')
        ptr++;
    if (*ptr && *ptr == 3)
        ptr++;

    bool b_back = false;
    start = ptr;
    while (*ptr && *ptr != 3 && *ptr != '|')
        ptr++;
    if (ptr > start) {
        b_back = true;
        out.selected_background_colour.set(strtoul(start, nullptr, 16));
    }

    if (*ptr && *ptr == 3)
        ptr++;
    if (*ptr && *ptr == '|')
        ptr++;
    if (*ptr && *ptr == 3)
        ptr++;

    start = ptr;
    while (*ptr && *ptr != 3 && *ptr != '|')
        ptr++;
    if (ptr > start)
        out.selected_background_colour_non_focus.set(strtoul(start, nullptr, 16));
    else if (b_back)
        out.selected_background_colour_non_focus.set(out.selected_background_colour);

    if (*ptr && *ptr == 3)
        ptr++;
    if (*ptr && *ptr == '|')
        ptr++;
    if (*ptr && *ptr == 3)
        ptr++;

    start = ptr;
    while (*ptr && *ptr != 3 && *ptr != '|')
        ptr++;
    if (ptr > start) {
        out.frame_left.set(strtoul(start, nullptr, 16));
        out.use_frame_left = true;
    }
    // else
    //    out.use_frame_left = false;

    if (*ptr && *ptr == 3)
        ptr++;
    if (*ptr && *ptr == '|')
        ptr++;
    if (*ptr && *ptr == 3)
        ptr++;

    start = ptr;
    while (*ptr && *ptr != 3 && *ptr != '|')
        ptr++;
    if (ptr > start) {
        out.frame_top.set(strtoul(start, nullptr, 16));
        out.use_frame_top = true;
    }
    // else
    //    out.use_frame_top = false;

    if (*ptr && *ptr == 3)
        ptr++;
    if (*ptr && *ptr == '|')
        ptr++;
    if (*ptr && *ptr == 3)
        ptr++;

    start = ptr;
    while (*ptr && *ptr != 3 && *ptr != '|')
        ptr++;
    if (ptr > start) {
        out.frame_right.set(strtoul(start, nullptr, 16));
        out.use_frame_right = true;
    }
    // else
    //    out.use_frame_right = false;

    if (*ptr && *ptr == 3)
        ptr++;
    if (*ptr && *ptr == '|')
        ptr++;
    if (*ptr && *ptr == 3)
        ptr++;

    start = ptr;
    while (*ptr && *ptr != 3 && *ptr != '|')
        ptr++;
    if (ptr > start) {
        out.frame_bottom.set(strtoul(start, nullptr, 16));
        out.use_frame_bottom = true;
    }
    // else
    //    out.use_frame_bottom = false;
}
