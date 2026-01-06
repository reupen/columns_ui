#include "pch.h"
#include "playlist_switcher_v2.h"

#include "playlist_switcher_title_formatting.h"
#include "tf_text_format.h"
#include "tf_utils.h"

namespace cui::panels::playlist_switcher {

std::vector<PlaylistSwitcher*> PlaylistSwitcher::s_windows;

void PlaylistSwitcher::get_insert_items(size_t base, size_t count, pfc::list_t<InsertItem>& p_out) const
{
    p_out.set_count(count);

    titleformat_object::ptr tf_object;

    if (cfg_playlist_switcher_use_tagz)
        tf_object = titleformat_compiler::get()->compile(cfg_playlist_switcher_tagz);

    for (size_t i = 0; i < count; i++) {
        p_out[i].m_subitems.resize(1);
        p_out[i].m_subitems[0].set_string(format_playlist_title(i + base, tf_object, get_items_font_size_pt()));
    }
}

void PlaylistSwitcher::repopulate_items()
{
    remove_items(bit_array_true());

    add_items(0, m_playlist_api->get_playlist_count());

    size_t index = m_playlist_api->get_active_playlist();
    if (index != pfc_infinite)
        set_item_selected_single(index, false);
}

void PlaylistSwitcher::refresh_all_items()
{
    refresh_items(0, get_item_count());
}

void PlaylistSwitcher::refresh_items(size_t base, size_t count)
{
    pfc::list_t<InsertItem> items_insert;
    get_insert_items(base, count, items_insert);
    replace_items(base, items_insert);
}

void PlaylistSwitcher::add_items(size_t base, size_t count)
{
    pfc::list_t<InsertItem> items_insert;
    get_insert_items(base, count, items_insert);
    insert_items(base, items_insert.get_count(), items_insert.get_ptr());
}

void PlaylistSwitcher::refresh_columns()
{
    set_columns({{"Name", 100}});
}

void PlaylistSwitcher::move_selection(int delta)
{
    size_t count = m_playlist_api->get_playlist_count();
    order_helper order(count);
    {
        size_t from = get_selected_item_single();
        if (from != pfc_infinite) {
            size_t to = from;
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
void PlaylistSwitcher::s_on_edgestyle_change()
{
    for (auto& window : s_windows) {
        window->set_edge_style(cfg_plistframe);
    }
}
void PlaylistSwitcher::s_on_vertical_item_padding_change()
{
    for (auto& window : s_windows) {
        window->set_vertical_item_padding(settings::playlist_switcher_item_padding);
    }
}
void PlaylistSwitcher::s_redraw_all()
{
    for (auto& window : s_windows)
        RedrawWindow(window->get_wnd(), nullptr, nullptr, RDW_UPDATENOW | RDW_INVALIDATE);
}

void PlaylistSwitcher::s_on_dark_mode_status_change()
{
    const auto is_dark = colours::is_dark_mode_active();
    for (auto&& window : s_windows)
        window->set_use_dark_mode(is_dark);
}

void PlaylistSwitcher::s_refresh_all_items()
{
    for (auto& window : s_windows)
        window->refresh_all_items();
}

void PlaylistSwitcher::s_on_font_items_change()
{
    const auto is_default_font_size_used = cfg_playlist_switcher_use_tagz
        && tf::is_field_used(cfg_playlist_switcher_tagz, tf::TextFormatTitleformatHook::default_font_size_field_name);

    for (auto& window : s_windows) {
        window->recreate_items_text_format();

        if (is_default_font_size_used)
            window->refresh_all_items();
    }
}

void PlaylistSwitcher::notify_on_initialisation()
{
    set_use_dark_mode(colours::is_dark_mode_active());
    set_autosize(true);
    set_selection_mode(SelectionMode::SingleStrict);
    set_show_header(false);
    set_edge_style(cfg_plistframe);
    set_vertical_item_padding(settings::playlist_switcher_item_padding);

    recreate_items_text_format();
}

void PlaylistSwitcher::notify_on_create()
{
    m_playlist_api = standard_api_create_t<playlist_manager_v3>();
    m_playback_api = standard_api_create_t<playback_control>();

    m_playing_playlist = get_playing_playlist();

    refresh_columns();

    repopulate_items();

    m_playlist_api->register_callback(this, flag_all);
    standard_api_create_t<play_callback_manager>()->register_callback(this, flag_on_playback_all, false);

    wil::com_ptr<DropTarget> drop_target = new DropTarget(this);
    RegisterDragDrop(get_wnd(), drop_target.get());

    s_windows.push_back(this);
}
void PlaylistSwitcher::notify_on_destroy()
{
    m_selection_holder.release();

    std::erase(s_windows, this);

    RevokeDragDrop(get_wnd());

    standard_api_create_t<play_callback_manager>()->unregister_callback(this);
    m_playlist_api->unregister_callback(this);
    m_playlist_api.release();
    m_playback_api.release();
}

namespace {
uie::window_factory<PlaylistSwitcher> g_playlist_switcher;
}

} // namespace cui::panels::playlist_switcher
