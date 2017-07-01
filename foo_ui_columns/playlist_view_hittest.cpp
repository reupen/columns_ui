#include "stdafx.h"
#include "playlist_view.h"

int playlist_view::hittest_item(int x, int y, bool check_in_column)
{
    RECT playlist;
    get_playlist_rect(&playlist);
    POINT pt = { x, y };

    if (check_in_column && !PtInRect(&playlist, pt) || !check_in_column && (pt.y < playlist.top || pt.y > playlist.bottom)) return -1;

    if (check_in_column && (x + horizontal_offset > get_columns_total_width())) return -1;



    int item_height = get_item_height();
    int idx = ((y - get_header_height()) / item_height) + scroll_item_offset;
    static_api_ptr_t<playlist_manager> playlist_api;

    if (idx >= 0 && idx < playlist_api->activeplaylist_get_item_count())
        return idx;
    else
        return -1;
}

int playlist_view::hittest_item_no_scroll(int x, int y, bool check_in_column)
{

    POINT pt = { x, y };

    if (check_in_column && (x + horizontal_offset > get_columns_total_width())) return -1;



    int item_height = get_item_height();
    int idx = ((y - get_header_height()) / item_height);

    return idx;
}

int playlist_view::hittest_column(int x, long &width)
{
    int column = 0;

    pfc::array_t<int, pfc::alloc_fast_aggressive> widths;

    int tw = get_column_widths(widths);

    int cx = 0,
        count = widths.get_size();

    if (x + horizontal_offset > tw || x < 0) return -1;

    while (cx < tw && column + 1 < count && cx < x + horizontal_offset)
    {
        cx += widths[column];
        column++;
    }

    if (cx > x + horizontal_offset && column > 0)
    {
        column--;
        cx -= widths[column];
    }

    width = cx - horizontal_offset;

    return column;
}
