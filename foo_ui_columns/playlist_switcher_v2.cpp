#include "stdafx.h"
#include "playlist_switcher_v2.h"

// {70A5C273-67AB-4bb6-B61C-F7975A6871FD}
const GUID PlaylistSwitcher::g_guid_font
    = {0x70a5c273, 0x67ab, 0x4bb6, {0xb6, 0x1c, 0xf7, 0x97, 0x5a, 0x68, 0x71, 0xfd}};

std::vector<PlaylistSwitcher*> PlaylistSwitcher::g_windows;

void PlaylistSwitcher::get_insert_items(t_size base, t_size count, pfc::list_t<uih::ListView::InsertItem>& p_out)
{
    p_out.set_count(count);

    for (t_size i = 0; i < count; i++) {
        p_out[i].m_subitems.set_count(1);
        pfc::string8 temp;
        m_playlist_api->playlist_get_name(i + base, temp);
        p_out[i].m_subitems[0].set_string(playlist_format_name_t(i + base, temp, get_playing_playlist()));
    }
}

void PlaylistSwitcher::refresh_all_items()
{
    remove_items(pfc::bit_array_true(), false);

    add_items(0, m_playlist_api->get_playlist_count());

    t_size index = m_playlist_api->get_active_playlist();
    if (index != pfc_infinite)
        set_item_selected_single(index, false);
}

void PlaylistSwitcher::refresh_items(t_size base, t_size count, bool b_update)
{
    pfc::list_t<uih::ListView::InsertItem> items_insert;
    get_insert_items(base, count, items_insert);
    replace_items(base, items_insert, b_update);
}

void PlaylistSwitcher::add_items(t_size base, t_size count)
{
    pfc::list_t<uih::ListView::InsertItem> items_insert;
    get_insert_items(base, count, items_insert);
    insert_items(base, items_insert.get_count(), items_insert.get_ptr());
}

void PlaylistSwitcher::refresh_columns()
{
    set_columns(pfc::list_single_ref_t<Column>(Column("Name", 100)));
}

void PlaylistSwitcher::move_selection(int delta)
{
    t_size count = m_playlist_api->get_playlist_count();
    order_helper order(count);
    {
        t_size from = get_selected_item_single();
        if (from != pfc_infinite) {
            t_size to = from;
            if (delta) {
                if (delta > 0)
                    while (delta && to + 1 < count) {
                        order.swap(from, from + 1);
                        to++;
                        delta--;
                    }
                else if (delta < 0)
                    while (delta && to > 0) {
                        order.swap(from, from - 1);
                        to--;
                        delta++;
                    }
                {
                    m_playlist_api->reorder(order.get_ptr(), count);
                }
            }
        }
    }
}
void PlaylistSwitcher::g_on_edgestyle_change()
{
    for (auto& window : g_windows) {
        window->set_edge_style(cfg_plistframe);
    }
}
void PlaylistSwitcher::g_on_vertical_item_padding_change()
{
    for (auto& window : g_windows) {
        window->set_vertical_item_padding(settings::playlist_switcher_item_padding);
    }
}
void PlaylistSwitcher::g_redraw_all()
{
    for (auto& window : g_windows)
        RedrawWindow(window->get_wnd(), nullptr, nullptr, RDW_UPDATENOW | RDW_INVALIDATE);
}
void PlaylistSwitcher::g_refresh_all_items()
{
    for (auto& window : g_windows)
        window->refresh_all_items();
}
void PlaylistSwitcher::g_on_font_items_change()
{
    LOGFONT lf;
    static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_font, lf);
    for (auto& window : g_windows) {
        window->set_font(&lf);
    }
}
void PlaylistSwitcher::notify_on_initialisation()
{
    set_autosize(true);
    set_single_selection(true);
    set_show_header(false);
    set_edge_style(cfg_plistframe);
    set_vertical_item_padding(settings::playlist_switcher_item_padding);

    LOGFONT lf;
    static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_font, lf);
    set_font(&lf);
};
void PlaylistSwitcher::notify_on_create()
{
    m_playlist_api = standard_api_create_t<playlist_manager_v3>();
    m_playback_api = standard_api_create_t<playback_control>();

    m_playing_playlist = get_playing_playlist();

    refresh_columns();

    refresh_all_items();

    m_playlist_api->register_callback(this, playlist_callback::flag_all);
    standard_api_create_t<play_callback_manager>()->register_callback(this, play_callback::flag_on_playback_all, false);

    pfc::com_ptr_t<IDropTarget_t> drop_target = new IDropTarget_t(this);
    RegisterDragDrop(get_wnd(), drop_target.get_ptr());

    g_windows.push_back(this);
}
void PlaylistSwitcher::notify_on_destroy()
{
    m_selection_holder.release();

    g_windows.erase(std::remove(g_windows.begin(), g_windows.end(), this), g_windows.end());

    RevokeDragDrop(get_wnd());

    standard_api_create_t<play_callback_manager>()->unregister_callback(this);
    m_playlist_api->unregister_callback(this);
    m_playlist_api.release();
    m_playback_api.release();
}

namespace {
uie::window_factory<PlaylistSwitcher> g_playlist_switcher;
}