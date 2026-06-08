#include "pch.h"

#include "common.h"
#include "playlist_switcher_v2.h"

namespace cui::panels::playlist_switcher {

bool PlaylistSwitcher::notify_on_contextmenu(const POINT& pt, bool from_keyboard)
{
    uie::window_ptr p_this_temp = this;

    uih::Menu menu;
    uih::MenuCommandCollector collector;

    size_t focused_item = get_focus_item();

    size_t playlist_count = m_playlist_api->get_playlist_count();
    size_t active_playlist_index = m_playlist_api->get_active_playlist();

    const auto is_focused_item_valid = focused_item < playlist_count;
    const auto is_on_item = [&] {
        if (!is_focused_item_valid)
            return false;

        if (from_keyboard)
            return true;

        POINT client_pt{pt};
        ScreenToClient(get_wnd(), &client_pt);

        HitTestResult hit_test_result;
        this->hit_test_ex(client_pt, hit_test_result);
        return hit_test_result.category == HitTestCategory::OnUnobscuredItem
            || hit_test_result.category == HitTestCategory::OnItemObscuredAbove
            || hit_test_result.category == HitTestCategory::OnItemObscuredBelow;
    }();

    playlist_position_reference_tracker tracked_focused_item(false);

    if (is_on_item) {
        tracked_focused_item.m_playlist = focused_item;
        set_highlight_selected_item(focused_item);
    }

    size_t playlist_item_count{};

    if (is_on_item) {
        const auto autoplaylist_api = autoplaylist_manager::get();
        autoplaylist_client_v2::ptr autoplaylist_v2;

        try {
            autoplaylist_client::ptr autoplaylist_v1 = autoplaylist_api->query_client(focused_item);
            autoplaylist_v2 &= autoplaylist_v1;
        } catch (pfc::exception const&) {
        }

        playlist_item_count = m_playlist_api->playlist_get_item_count(focused_item);

        if (playlist_item_count > 0)
            menu.append_command(collector.add([&] {
                if (tracked_focused_item.m_playlist == SIZE_MAX)
                    return;

                m_playlist_api->set_playing_playlist(tracked_focused_item.m_playlist);
                play_control::get()->start();
            }),
                L"Play"_zv, {.is_default = true});

        if (active_playlist_index != focused_item)
            menu.append_command(collector.add([&] {
                if (tracked_focused_item.m_playlist != SIZE_MAX)
                    m_playlist_api->set_active_playlist(tracked_focused_item.m_playlist);
            }),
                L"Activate"_zv, {.is_default = playlist_item_count == 0});

        menu.append_command(collector.add([&] {
            if (tracked_focused_item.m_playlist != SIZE_MAX)
                activate_inline_editing(tracked_focused_item.m_playlist, 0);
        }),
            L"Rename"_zv);

        menu.append_command(collector.add([&] {
            if (tracked_focused_item.m_playlist != SIZE_MAX)
                m_playlist_api->remove_playlist_switch(tracked_focused_item.m_playlist);
        }),
            L"Remove"_zv);

        if (autoplaylist_v2.is_valid() && autoplaylist_v2->show_ui_available()) {
            menu.append_separator();

            pfc::string8 name;
            autoplaylist_v2->get_display_name(name);

            menu.append_command(collector.add([&tracked_focused_item, autoplaylist_v2] {
                if (tracked_focused_item.m_playlist != SIZE_MAX)
                    autoplaylist_v2->show_ui(tracked_focused_item.m_playlist);
            }),
                fmt::format(L"{} properties", mmh::to_utf16(name.c_str())));
        }

        menu.append_separator();

        menu.append_command(collector.add([&] {
            if (tracked_focused_item.m_playlist != SIZE_MAX)
                playlist_manager_utils::cut(pfc::list_single_ref_t(tracked_focused_item.m_playlist));
        }),
            L"Cut"_zv);

        menu.append_command(collector.add([&] {
            if (tracked_focused_item.m_playlist != SIZE_MAX)
                playlist_manager_utils::copy(pfc::list_single_ref_t(tracked_focused_item.m_playlist));
        }),
            L"Copy"_zv);

        if (playlist_manager_utils::check_clipboard())
            menu.append_command(collector.add([&] {
                playlist_manager_utils::paste(get_wnd(),
                    tracked_focused_item.m_playlist != SIZE_MAX ? tracked_focused_item.m_playlist + 1 : SIZE_MAX);
            }),
                L"Paste"_zv);

        menu.append_separator();
    }

    menu.append_command(collector.add([&] {
        const auto new_index = m_playlist_api->create_playlist("Untitled", SIZE_MAX,
            tracked_focused_item.m_playlist != SIZE_MAX ? tracked_focused_item.m_playlist + 1 : SIZE_MAX);

        if (new_index != SIZE_MAX) {
            if (m_playlist_api->get_active_playlist() == SIZE_MAX)
                m_playlist_api->set_active_playlist(new_index);

            activate_inline_editing(new_index, 0);
        }
    }),
        L"New"_zv);

    menu.append_command(collector.add([&] { standard_commands::main_load_playlist(); }), L"Load…"_zv);

    if (is_on_item)
        menu.append_command(collector.add([&] {
            if (tracked_focused_item.m_playlist == SIZE_MAX)
                return;

            metadb_handle_list_t<pfc::alloc_fast_aggressive> tracks;
            tracks.prealloc(m_playlist_api->playlist_get_item_count(tracked_focused_item.m_playlist));
            m_playlist_api->playlist_get_all_items(tracked_focused_item.m_playlist, tracks);

            pfc::string8 name;
            m_playlist_api->playlist_get_name(tracked_focused_item.m_playlist, name);
            g_save_playlist(get_wnd(), tracks, name);
        }),
            L"Save as…"_zv);

    if (playlist_count > 0)
        menu.append_command(collector.add([&] { standard_commands::main_save_all_playlists(); }), L"Save all as…"_zv);

    {
        const auto recycler_count
            = gsl::narrow<unsigned>(std::min(m_playlist_api->recycler_get_count(), size_t{UINT32_MAX}));

        if (recycler_count > 0) {
            uih::Menu recycler_popup;
            pfc::string8_fast_aggressive temp;

            for (size_t i = 0; i < recycler_count; i++) {
                m_playlist_api->recycler_get_name(i, temp);
                const auto recycler_id = m_playlist_api->recycler_get_id(i); // Menu Message Loop !

                recycler_popup.append_command(
                    collector.add([this, recycler_id] { m_playlist_api->recycler_restore_by_id(recycler_id); }),
                    mmh::to_utf16(temp.c_str()));
            }

            recycler_popup.append_separator();
            recycler_popup.append_command(
                collector.add([&] { m_playlist_api->recycler_purge(bit_array_true()); }), L"Clear"_zv);

            menu.append_submenu(std::move(recycler_popup), L"History"_zv);
        }
    }

    const auto collector_num_items = collector.size();
    std::optional<uint32_t> context_manager_base_id;
    contextmenu_manager::ptr contextmenu_manager;

    if (playlist_item_count > 0) {
        menu.append_separator();

        uih::Menu items_submenu;

        if (collector_num_items < INT32_MAX - 1) {
            context_manager_base_id = gsl::narrow<uint32_t>(collector_num_items + 1);

            metadb_handle_list_t<pfc::alloc_fast_aggressive> tracks;
            tracks.prealloc(playlist_item_count);
            m_playlist_api->playlist_get_all_items(focused_item, tracks);

            contextmenu_manager::g_create(contextmenu_manager);
            contextmenu_manager->init_context(tracks, 0);
            contextmenu_manager->win32_build_menu(
                items_submenu.get(), gsl::narrow<int>(*context_manager_base_id), INT32_MAX);

            menu.append_submenu(std::move(items_submenu), L"Items"_zv);
        }
    }

    menu_helpers::win32_auto_mnemonics(menu.get());
    const auto command_id = menu.run(get_wnd(), pt);

    m_status_text_override.release();
    remove_highlight_selected_item();

    if (context_manager_base_id && command_id >= *context_manager_base_id)
        contextmenu_manager->execute_by_id(command_id - *context_manager_base_id);
    else
        collector.execute(command_id);

    return true;
}

} // namespace cui::panels::playlist_switcher
