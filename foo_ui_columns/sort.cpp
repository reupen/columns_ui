#include "stdafx.h"
#include "playlist_view.h"
#include "sort.h"

void create_image_list() /*sort arrows*/
{
    g_imagelist = ImageList_Create(12, 11, ILC_COLOR | ILC_MASK, 2, 0);

    auto g_down = (HBITMAP)uLoadImage(core_api::get_my_instance(), uMAKEINTRESOURCE(IDB_ARROWS), IMAGE_BITMAP, 0, 0, 0);
    ImageList_AddMasked(g_imagelist, g_down, 0xFFFFFF);

    DeleteObject(g_down);

    unsigned n, pcount = playlist_view::list_playlist.get_count();
    for (n = 0; n < pcount; n++) {
        playlist_view* p_playlist = playlist_view::list_playlist.get_item(n);
        if (p_playlist->wnd_header) {
            Header_SetImageList(p_playlist->wnd_header, g_imagelist);
        }
    }
}
