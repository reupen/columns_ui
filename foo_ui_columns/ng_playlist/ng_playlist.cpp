#include "pch.h"

#include "ng_playlist.h"

#include "font_manager_v3.h"
#include "ng_playlist_groups.h"
#include "../config_columns_v2.h"
#include "../playlist_item_helpers.h"
#include "../playlist_view_tfhooks.h"
#include "../button_items.h"

namespace cui::artwork_panel {
// extern cfg_string cfg_front;
extern cfg_bool cfg_use_fb2k_artwork;
}; // namespace cui::artwork_panel

namespace cui::panels::playlist_view {

// {775E2746-6019-4b45-83B3-D7DF3A5BAE57}
const GUID g_groups_guid = {0x775e2746, 0x6019, 0x4b45, {0x83, 0xb3, 0xd7, 0xdf, 0x3a, 0x5b, 0xae, 0x57}};

// {85068AD6-A45D-4968-9898-A34F1F3C40EA}
const GUID g_show_artwork_guid = {0x85068ad6, 0xa45d, 0x4968, {0x98, 0x98, 0xa3, 0x4f, 0x1f, 0x3c, 0x40, 0xea}};

// {3003B49E-AD9C-464f-8B54-E9F864D52BBA}
const GUID g_artwork_width_guid = {0x3003b49e, 0xad9c, 0x464f, {0x8b, 0x54, 0xe9, 0xf8, 0x64, 0xd5, 0x2b, 0xba}};

// {C764D238-403F-4a8c-B310-30D23E4C42F6}
const GUID g_artwork_reflection = {0xc764d238, 0x403f, 0x4a8c, {0xb3, 0x10, 0x30, 0xd2, 0x3e, 0x4c, 0x42, 0xf6}};

// {A28CC736-2B8B-484c-B7A9-4CC312DBD357}
const GUID g_guid_grouping = {0xa28cc736, 0x2b8b, 0x484c, {0xb7, 0xa9, 0x4c, 0xc3, 0x12, 0xdb, 0xd3, 0x57}};

std::vector<PlaylistView*> PlaylistView::g_windows;

ConfigGroups g_groups(g_groups_guid);

cfg_bool cfg_artwork_reflection(g_artwork_reflection, false);

fbh::ConfigBool cfg_grouping(g_guid_grouping, true, [](auto&&) { button_items::ShowGroupsButton::s_on_change(); });
fbh::ConfigBool cfg_indent_groups({0x2e3d28c7, 0x7e99, 0x410f, {0xa5, 0x50, 0xd1, 0x5c, 0xc0, 0x6e, 0xa5, 0x51}}, true,
    [](auto&&) { PlaylistView::s_on_indent_groups_change(); });
fbh::ConfigBool cfg_use_custom_group_indentation_amount(
    {0x53cad633, 0xa735, 0x4880, {0x91, 0x45, 0xd9, 0x11, 0x98, 0x5e, 0x51, 0xe6}}, false,
    [](auto&&) { PlaylistView::s_on_group_indentation_amount_change(); });
fbh::ConfigInt32DpiAware cfg_custom_group_indentation_amount(
    {0x3d1b3bce, 0x25d2, 0x4dde, {0x8e, 0x99, 0x20, 0xfb, 0xc9, 0x6c, 0xbf, 0xec}}, 10);
fbh::ConfigBool cfg_show_artwork(
    g_show_artwork_guid, false, [](auto&&) { button_items::ShowArtworkButton::s_on_change(); });
fbh::ConfigUint32DpiAware cfg_artwork_width(g_artwork_width_guid, 100);

void ConfigGroups::swap(size_t index1, size_t index2)
{
    m_groups.swap_items(index1, index2);
    PlaylistView::g_on_groups_change();
}
void ConfigGroups::replace_group(size_t index, const Group& p_group)
{
    m_groups.replace_item(index, p_group);
    PlaylistView::g_on_groups_change();
}
size_t ConfigGroups::add_group(const Group& p_group, bool notify_playlist_views)
{
    size_t ret = m_groups.add_item(p_group);
    if (notify_playlist_views)
        PlaylistView::g_on_groups_change();
    return ret;
}

void ConfigGroups::set_groups(const pfc::list_base_const_t<Group>& p_groups, bool b_update_views)
{
    m_groups.remove_all();
    m_groups.add_items(p_groups);
    if (b_update_views)
        PlaylistView::g_on_groups_change();
}

void ConfigGroups::remove_group(size_t index)
{
    m_groups.remove_by_idx(index);
    PlaylistView::g_on_groups_change();
}

void set_font_size(bool up)
{
    LOGFONT lf_ng;
    const auto api = fb2k::std_api_get<fonts::manager>();
    api->get_font(g_guid_items_font, lf_ng);

    fonts::get_next_font_size_step(lf_ng, up);

    api->set_font(g_guid_items_font, lf_ng);
}

PlaylistView::PlaylistView()
    : ListViewPanelBase(std::make_unique<PlaylistViewRenderer>(this))
    , m_dragging_initial_playlist(pfc_infinite) {};

PlaylistView::~PlaylistView() = default;

void PlaylistView::on_first_show()
{
    const auto playlist_index = playlist_manager::get()->get_active_playlist();

    if (playlist_index == std::numeric_limits<size_t>::max())
        return;

    const auto position = m_playlist_cache.get_item(playlist_index).saved_scroll_position;

    if (position) {
        restore_scroll_position(*position);
    } else if (const size_t focus = get_focus_item(); focus != std::numeric_limits<size_t>::max()) {
        ensure_visible(focus);
    }
}

void PlaylistView::populate_list(const std::optional<uih::lv::SavedScrollPosition>& scroll_position)
{
    metadb_handle_list_t<pfc::alloc_fast_aggressive> data;
    m_playlist_api->activeplaylist_get_all_items(data);
    insert_tracks(0, data, scroll_position);
}

void PlaylistView::insert_tracks(size_t index, const pfc::list_base_const_t<metadb_handle_ptr>& tracks,
    const std::optional<uih::lv::SavedScrollPosition>& scroll_position)
{
    clear_sort_column();
    InsertItemsContainer items;
    get_insert_items(index, tracks.size(), items);
    insert_items(index, items.size(), items.get_ptr(), scroll_position);
    refresh_all_items_text();
}

void PlaylistView::refresh_groups(bool b_update_columns)
{
    const auto p_compiler = titleformat_compiler::get();
    service_ptr_t<titleformat_object> p_script;
    service_ptr_t<titleformat_object> p_script_group;
    m_scripts.remove_all();

    pfc::string8 filter;
    pfc::string8 playlist_name;
    m_playlist_api->activeplaylist_get_name(playlist_name);

    size_t count = cfg_grouping ? g_groups.get_groups().get_count() : 0;
    size_t used_count = 0;
    for (size_t i = 0; i < count; i++) {
        bool b_valid = false;
        switch (g_groups.get_groups()[i].filter_type) {
        case FILTER_NONE:
            b_valid = true;
            break;
        case FILTER_SHOW:
            if (wildcard_helper::test(playlist_name, g_groups.get_groups()[i].filter_playlists, true))
                b_valid = true;
            break;
        case FILTER_HIDE:
            if (!wildcard_helper::test(playlist_name, g_groups.get_groups()[i].filter_playlists, true))
                b_valid = true;
            break;
        }
        if (b_valid) {
            p_compiler->compile_safe(p_script_group, g_groups.get_groups()[i].string);
            m_scripts.add_item(p_script_group);
            used_count++;
        }
    }
    set_group_count(used_count, b_update_columns);
}

size_t PlaylistView::column_index_display_to_actual(size_t display_index)
{
    size_t count = m_column_mask.get_count();
    size_t counter = 0;
    for (size_t i = 0; i < count; i++) {
        if (m_column_mask[i])
            if (counter++ == display_index)
                return i;
    }
    throw pfc::exception_bug_check();
}

size_t PlaylistView::column_index_actual_to_display(size_t actual_index)
{
    size_t count = m_column_mask.get_count();
    size_t counter = 0;
    for (size_t i = 0; i < count; i++) {
        if (m_column_mask[i]) {
            counter++;
            if (i == actual_index)
                return counter;
        }
    }
    return pfc_infinite;
    // throw pfc::exception_bug_check();
}
void PlaylistView::on_column_widths_change()
{
    size_t count = m_column_mask.get_count();
    std::vector<int> widths;
    for (size_t i = 0; i < count; i++)
        if (m_column_mask[i])
            widths.emplace_back(g_columns[i]->width);
    set_column_widths(widths);
}

void PlaylistView::g_on_column_widths_change(const PlaylistView* p_skip)
{
    for (auto& window : g_windows)
        if (window != p_skip)
            window->on_column_widths_change();
}
void PlaylistView::refresh_columns()
{
    const auto p_compiler = titleformat_compiler::get();
    service_ptr_t<titleformat_object> p_script;
    service_ptr_t<titleformat_object> p_script_group;
    m_script_global.release();
    m_script_global_style.release();

    m_column_data.remove_all();
    m_edit_fields.remove_all();
    std::vector<Column> columns;

    pfc::string8 filter;
    pfc::string8 playlist_name;
    m_playlist_api->activeplaylist_get_name(playlist_name);

    if (cfg_global)
        p_compiler->compile_safe(m_script_global, cfg_globalstring);
    p_compiler->compile_safe(m_script_global_style, cfg_colour);

    size_t count = g_columns.get_count();
    m_column_mask.set_size(count);
    for (size_t i = 0; i < count; i++) {
        PlaylistViewColumn* source = g_columns[i].get();
        bool b_valid = false;
        if (source->show) {
            switch (source->filter_type) {
            case FILTER_NONE: {
                b_valid = true;
                break;
            }
            case FILTER_SHOW: {
                if (wildcard_helper::test(playlist_name, source->filter, true))
                    b_valid = true;
            } break;
            case FILTER_HIDE: {
                if (!wildcard_helper::test(playlist_name, source->filter, true))
                    b_valid = true;
            } break;
            }
        }
        if (b_valid) {
            ColumnData temp;
            columns.emplace_back(source->name, source->width, source->parts, (uih::alignment)source->align);
            p_compiler->compile_safe(temp.m_display_script, source->spec);
            if (source->use_custom_colour)
                p_compiler->compile_safe(temp.m_style_script, source->colour_spec);
            if (source->use_custom_sort)
                p_compiler->compile_safe(temp.m_sort_script, source->sort_spec);
            else
                temp.m_sort_script = temp.m_display_script;
            m_column_data.add_item(temp);
            m_edit_fields.add_item(source->edit_field);
        }
        m_column_mask[i] = b_valid;
    }
    set_columns(columns);
}

void PlaylistView::set_group_info_area_size()
{
    ListView::set_group_info_area_size(cfg_artwork_width + get_artwork_left_right_padding() * 2,
        cfg_artwork_width + (cfg_artwork_reflection ? (cfg_artwork_width * 3) / 11 : 0));
}

void PlaylistView::g_on_groups_change()
{
    for (auto& window : g_windows)
        window->on_groups_change();
}

void PlaylistView::s_on_indent_groups_change()
{
    if (!cfg_grouping)
        return;

    for (const auto& window : g_windows) {
        window->on_artwork_width_change();
        window->set_group_level_indentation_enabled(cfg_indent_groups);
    }
}

void PlaylistView::s_on_group_indentation_amount_change()
{
    if (!cfg_grouping)
        return;

    for (const auto& window : g_windows) {
        window->set_group_level_indentation_amount(cfg_use_custom_group_indentation_amount
                ? std::make_optional<int>(cfg_custom_group_indentation_amount)
                : std::nullopt);
    }
}

void PlaylistView::on_groups_change()
{
    if (get_wnd()) {
        clear_all_items();
        refresh_groups(true);
        populate_list();
    }
}

void PlaylistView::update_all_items()
{
    const auto p_compiler = titleformat_compiler::get();
    service_ptr_t<titleformat_object> p_script;
    service_ptr_t<titleformat_object> p_script_group;

    m_script_global.release();
    m_script_global_style.release();

    if (cfg_global)
        p_compiler->compile_safe(m_script_global, cfg_globalstring);
    p_compiler->compile_safe(m_script_global_style, cfg_colour);

    refresh_all_items_text();
}
void PlaylistView::refresh_all_items_text()
{
    update_items(0, get_item_count());
}
void PlaylistView::update_items(size_t index, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        size_t cg = get_item(i + index)->get_group_count();
        for (size_t j = 0; j < cg; j++)
            get_item(i + index)->get_group(j)->m_style_data.release();
        get_item(i + index)->m_style_data.set_count(0);
    }
    ListView::update_items(index, count);
}
void PlaylistView::g_on_autosize_change()
{
    for (auto& window : g_windows)
        window->set_autosize(cfg_nohscroll != 0);
}
void PlaylistView::g_on_show_artwork_change()
{
    for (auto& window : g_windows)
        window->set_show_group_info_area(cfg_show_artwork);
}
void PlaylistView::g_on_alternate_selection_change()
{
    for (auto& window : g_windows)
        window->set_alternate_selection_model(cfg_alternative_sel != 0);
}

void PlaylistView::g_on_artwork_width_change(const PlaylistView* p_skip)
{
    for (auto& window : g_windows) {
        if (window != p_skip) {
            window->on_artwork_width_change();
        }
    }
}

void PlaylistView::on_artwork_width_change()
{
    flush_artwork_images();
    set_group_info_area_size();
}

void PlaylistView::s_flush_artwork(bool b_redraw, const PlaylistView* p_skip)
{
    for (auto& window : g_windows) {
        if (window != p_skip) {
            window->flush_artwork_images();
            if (b_redraw)
                window->invalidate_all();
        }
    }
}

void PlaylistView::g_on_vertical_item_padding_change()
{
    for (auto& window : g_windows)
        window->set_vertical_item_padding(settings::playlist_view_item_padding);
}

void PlaylistView::g_on_font_change()
{
    const auto font = fb2k::std_api_get<fonts::manager_v3>()->get_client_font(g_guid_items_font);
    const auto text_format = font->create_wil_text_format();
    const auto log_font = font->log_font();

    for (auto& window : g_windows)
        window->set_font(text_format, log_font);
}

void PlaylistView::g_on_header_font_change()
{
    const auto font = fb2k::std_api_get<fonts::manager_v3>()->get_client_font(g_guid_header_font);
    const auto log_font = font->log_font();

    for (auto& window : g_windows)
        window->set_header_font(log_font);
}

void PlaylistView::g_on_group_header_font_change()
{
    const auto font = fb2k::std_api_get<fonts::manager_v3>()->get_client_font(g_guid_group_header_font);
    const auto text_format = font->create_wil_text_format();

    for (auto& window : g_windows)
        window->set_group_font(text_format);
}

void PlaylistView::s_update_all_items()
{
    for (auto& window : g_windows)
        window->update_all_items();
}
void PlaylistView::g_on_show_header_change()
{
    for (auto& window : g_windows)
        window->set_show_header(cfg_header != 0);
}
void PlaylistView::g_on_sorting_enabled_change()
{
    for (auto& window : g_windows)
        window->set_sorting_enabled(cfg_header_hottrack != 0);
}
void PlaylistView::g_on_show_sort_indicators_change()
{
    for (auto& window : g_windows)
        window->set_show_sort_indicators(cfg_show_sort_arrows != 0);
}
void PlaylistView::g_on_edge_style_change()
{
    for (auto& window : g_windows)
        window->set_edge_style(cfg_frame);
}
void PlaylistView::g_on_time_change()
{
    for (auto& window : g_windows)
        window->on_time_change();
}
void PlaylistView::g_on_show_tooltips_change()
{
    for (auto& window : g_windows) {
        window->set_show_tooltips(cfg_tooltips_clipped != 0);
    }
}
void PlaylistView::g_on_playback_follows_cursor_change(bool b_val)
{
    for (auto& window : g_windows)
        window->set_always_show_focus(b_val);
}

void PlaylistView::s_on_dark_mode_status_change()
{
    const auto is_dark = colours::is_dark_mode_active();
    for (auto&& window : g_windows) {
        window->flush_artwork_images();
        window->set_use_dark_mode(is_dark);
    }
}

void PlaylistView::g_on_columns_change()
{
    for (auto& window : g_windows)
        window->on_columns_change();
}
void PlaylistView::on_columns_change()
{
    if (get_wnd()) {
        clear_all_items();
        refresh_columns();
        populate_list();
    }
}

void PlaylistView::s_redraw_all()
{
    for (auto&& window : g_windows)
        RedrawWindow(window->get_wnd(), nullptr, nullptr, RDW_INVALIDATE);
}

const char* PlaylistView::PlaylistViewSearchContext::get_item_text(size_t index)
{
    constexpr size_t max_batch_size = 1000;

    if (!m_start_index) {
        m_playlist_manager->activeplaylist_get_all_items(m_tracks);
        m_items.resize(m_tracks.size());
        GetLocalTime(&m_systemtime);
        m_start_index = index;
    } else if (m_items[index]) {
        return m_items[index]->c_str();
    }

    const auto batch_end_index
        = std::min(index + max_batch_size, index < *m_start_index ? *m_start_index : m_tracks.size());

    const auto batch_size = batch_end_index - index;
    const auto batch_tracks = pfc::list_partial_ref_t(m_tracks, index, batch_size);

    const bool has_global_variables = m_global_script.is_valid();

    m_metadb->queryMulti_(batch_tracks, [this, index, has_global_variables](size_t offset, const metadb_v2_rec_t& rec) {
        metadb_handle_v2::ptr track;
        track &= m_tracks[index + offset];

        GlobalVariableList global_variables;
        DateTitleformatHook tf_hook_date(&m_systemtime);
        PlaylistNameTitleformatHook tf_hook_playlist_name;

        if (has_global_variables) {
            SetGlobalTitleformatHook<true, false> tf_hook_set_global(global_variables);
            SplitterTitleformatHook tf_hook(&tf_hook_set_global, &tf_hook_date, &tf_hook_playlist_name);
            pfc::string8 _;
            track->formatTitle_v2(rec, &tf_hook, _, m_global_script, nullptr);
        }

        std::string title;
        mmh::StringAdaptor adapted_title(title);

        SetGlobalTitleformatHook<false, true> tf_hook_get_global(global_variables);
        SplitterTitleformatHook tf_hook(
            has_global_variables ? &tf_hook_get_global : nullptr, &tf_hook_date, &tf_hook_playlist_name);
        track->formatTitle_v2(rec, &tf_hook, adapted_title, m_column_script, nullptr);
        m_items[index + offset] = std::move(title);
    });

    return m_items[index]->c_str();
}

void PlaylistView::s_create_message_window()
{
    uie::container_window_v3_config config(L"{columns_ui_playlist_view_message_window_goJRO8xwg7s}", false);
    config.window_styles = 0;
    config.extended_window_styles = 0;

    s_message_window = std::make_unique<uie::container_window_v3>(
        config, [](auto&& wnd, auto&& msg, auto&& wp, auto&& lp) -> LRESULT {
            if (msg == WM_TIMECHANGE)
                g_on_time_change();
            return DefWindowProc(wnd, msg, wp, lp);
        });
    s_message_window->create(nullptr);
}

void PlaylistView::s_destroy_message_window()
{
    s_message_window->destroy();
    s_message_window.reset();
}

int g_compare_wchar(const pfc::array_t<WCHAR>& a, const pfc::array_t<WCHAR>& b)
{
    return StrCmpLogicalW(a.get_ptr(), b.get_ptr());
}
void PlaylistView::notify_sort_column(size_t index, bool b_descending, bool b_selection_only)
{
    const auto active_playlist = m_playlist_api->get_active_playlist();

    if (active_playlist == std::numeric_limits<size_t>::max()
        || (m_playlist_api->playlist_lock_is_present(active_playlist)
            && (m_playlist_api->playlist_lock_get_filter_mask(active_playlist) & playlist_lock::filter_reorder)))
        return;

    if (static_api_test_t<metadb_v2>())
        sort_by_column_fb2k_v2(index, b_descending, b_selection_only);
    else
        sort_by_column_fb2k_v1(index, b_descending, b_selection_only);
}

void PlaylistView::sort_by_column_fb2k_v1(size_t column_index, bool b_descending, bool b_selection_only)
{
    const auto count = m_playlist_api->activeplaylist_get_item_count();

    std::vector<std::wstring> data{count};
    pfc::list_t<size_t, pfc::alloc_fast_aggressive> source_indices;
    source_indices.prealloc(count);

    pfc::string8_fast_aggressive temp;
    pfc::string8_fast_aggressive temp2;
    temp.prealloc(512);

    bool extra = m_script_global.is_valid() && cfg_global_sort;

    bit_array_bittable mask(count);
    if (b_selection_only)
        m_playlist_api->activeplaylist_get_selection_mask(mask);

    SYSTEMTIME st;
    GetLocalTime(&st);

    size_t counter = 0;

    for (size_t n{0}; n < count; n++) {
        if (!b_selection_only || mask[n]) {
            GlobalVariableList extra_items;

            {
                DateTitleformatHook tf_hook_date(&st);
                PlaylistNameTitleformatHook tf_hook_playlist_name;

                if (extra) {
                    SetGlobalTitleformatHook<true, false> tf_hook_set_global(extra_items);
                    SplitterTitleformatHook tf_hook(&tf_hook_set_global, &tf_hook_date, &tf_hook_playlist_name);
                    pfc::string8 output;
                    m_playlist_api->activeplaylist_item_format_title(
                        n, &tf_hook, output, m_script_global, nullptr, play_control::display_level_none);
                }

                SetGlobalTitleformatHook<false, true> tf_hook_get_global(extra_items);
                SplitterTitleformatHook tf_hook(
                    extra ? &tf_hook_get_global : nullptr, &tf_hook_date, &tf_hook_playlist_name);
                m_playlist_api->activeplaylist_item_format_title(n, &tf_hook, temp,
                    m_column_data[column_index].m_sort_script, nullptr, play_control::display_level_none);
            }

            const char* ptr = temp.get_ptr();
            if (strchr(ptr, 3)) {
                titleformat_compiler::remove_color_marks(ptr, temp2);
                ptr = temp2;
            }

            data[counter].resize(pfc::stringcvt::estimate_utf8_to_wide_quick(ptr));
            pfc::stringcvt::convert_utf8_to_wide_unchecked(data[counter].data(), ptr);

            counter++;
            if (b_selection_only)
                source_indices.add_item(n);
        }
    }
    data.resize(counter);

    mmh::Permutation order(data.size());
    sort_get_permutation(
        data.data(), order, [](auto&& left, auto&& right) { return StrCmpLogicalW(left.c_str(), right.c_str()); }, true,
        b_descending, true);

    m_playlist_api->activeplaylist_undo_backup();
    if (b_selection_only) {
        mmh::Permutation order2(count);
        size_t count2 = data.size();
        for (size_t n{0}; n < count2; n++) {
            order2[source_indices[n]] = source_indices[order[n]];
        }
        m_playlist_api->activeplaylist_reorder_items(order2.data(), count);
    } else
        m_playlist_api->activeplaylist_reorder_items(order.data(), count);
}

void PlaylistView::sort_by_column_fb2k_v2(size_t column_index, bool b_descending, bool b_selection_only)
{
    const auto playlist_size = m_playlist_api->activeplaylist_get_item_count();

    bit_array_bittable mask(playlist_size);
    if (b_selection_only)
        m_playlist_api->activeplaylist_get_selection_mask(mask);

    SYSTEMTIME st{};
    GetLocalTime(&st);

    metadb_handle_list tracks;

    if (b_selection_only)
        m_playlist_api->activeplaylist_get_selected_items(tracks);
    else
        m_playlist_api->activeplaylist_get_all_items(tracks);

    const bool extra = m_script_global.is_valid() && cfg_global_sort;
    std::vector<std::wstring> data{tracks.size()};

    metadb_v2::get()->queryMultiParallel_(
        tracks, [this, &tracks, &data, &st, extra, column_index](size_t index, const metadb_v2::rec_t& rec) {
            metadb_handle_v2::ptr track;
            track &= tracks[index];

            GlobalVariableList extra_items;
            DateTitleformatHook tf_hook_date(&st);
            PlaylistNameTitleformatHook tf_hook_playlist_name;

            if (extra) {
                SetGlobalTitleformatHook<true, false> tf_hook_set_global(extra_items);
                SplitterTitleformatHook tf_hook(&tf_hook_set_global, &tf_hook_date, &tf_hook_playlist_name);
                pfc::string8 output;
                track->formatTitle_v2(rec, &tf_hook, output, m_script_global, nullptr);
            }

            std::string title;
            mmh::StringAdaptor adapted_title(title);

            SetGlobalTitleformatHook<false, true> tf_hook_get_global(extra_items);
            SplitterTitleformatHook tf_hook(
                extra ? &tf_hook_get_global : nullptr, &tf_hook_date, &tf_hook_playlist_name);
            track->formatTitle_v2(rec, &tf_hook, adapted_title, m_column_data[column_index].m_sort_script, nullptr);

            const char* ptr = title.c_str();
            if (strchr(ptr, 3)) {
                pfc::string8_fast_aggressive title_without_colour_codes;
                title_without_colour_codes.prealloc(title.length());

                titleformat_compiler::remove_color_marks(ptr, title_without_colour_codes);
                ptr = title_without_colour_codes;
            }

            data[index].resize(pfc::stringcvt::estimate_utf8_to_wide_quick(ptr));
            pfc::stringcvt::convert_utf8_to_wide_unchecked(data[index].data(), ptr);
        });

    mmh::Permutation sorted_items_order(data.size());
    sort_get_permutation(
        data.data(), sorted_items_order,
        [](auto&& left, auto&& right) { return StrCmpLogicalW(left.c_str(), right.c_str()); }, true, b_descending,
        true);

    m_playlist_api->activeplaylist_undo_backup();

    if (!b_selection_only) {
        m_playlist_api->activeplaylist_reorder_items(sorted_items_order.data(), playlist_size);
    } else {
        std::vector<size_t> source_indices;
        source_indices.reserve(tracks.size());

        for (auto index : std::ranges::views::iota(size_t{}, playlist_size)) {
            if (mask[index])
                source_indices.emplace_back(index);
        }

        mmh::Permutation all_items_order(playlist_size);

        for (auto index : std::ranges::views::iota(size_t{}, tracks.size())) {
            all_items_order[source_indices[index]] = source_indices[sorted_items_order[index]];
        }
        m_playlist_api->activeplaylist_reorder_items(all_items_order.data(), playlist_size);
    }
}

void PlaylistView::notify_on_initialisation()
{
    set_use_dark_mode(colours::is_dark_mode_active());
    set_group_level_indentation_enabled(cfg_indent_groups);

    if (cfg_use_custom_group_indentation_amount)
        set_group_level_indentation_amount(cfg_custom_group_indentation_amount);

    set_group_info_area_size();
    set_show_group_info_area(cfg_show_artwork);
    set_show_header(cfg_header != 0);
    set_autosize(cfg_nohscroll != 0);
    set_always_show_focus(
        config_object::g_get_data_bool_simple(standard_config_objects::bool_playback_follows_cursor, false));
    set_vertical_item_padding(settings::playlist_view_item_padding);

    const auto font_api = fb2k::std_api_get<fonts::manager_v3>();
    const auto items_font = font_api->get_client_font(g_guid_items_font);
    const auto items_text_format = items_font->create_wil_text_format();
    const auto items_log_font = items_font->log_font();
    set_font(items_text_format, items_log_font);

    const auto header_font = font_api->get_client_font(g_guid_header_font);
    set_header_font(header_font->log_font());

    const auto group_font = font_api->get_client_font(g_guid_group_header_font);
    set_group_font(group_font->create_wil_text_format());

    set_sorting_enabled(cfg_header_hottrack != 0);
    set_show_sort_indicators(cfg_show_sort_arrows != 0);
    set_edge_style(cfg_frame);
    set_show_tooltips(cfg_tooltips_clipped != 0);
    set_alternate_selection_model(cfg_alternative_sel != 0);
    set_allow_header_rearrange(true);

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    m_gdiplus_initialised = (Gdiplus::Ok == GdiplusStartup(&m_gdiplus_token, &gdiplusStartupInput, nullptr));
    m_artwork_manager = std::make_shared<ArtworkReaderManager>();
    m_artwork_manager->initialise();

    m_playlist_api = playlist_manager_v4::get();
    m_playlist_cache.initialise(m_initial_scroll_positions);
    m_initial_scroll_positions.clear();

    refresh_columns();
    refresh_groups();
}

void PlaylistView::notify_on_create()
{
    pfc::hires_timer timer;
    timer.start();

    populate_list();

    m_playlist_api->register_callback(this,
        flag_all
            & ~(flag_on_default_format_changed | flag_on_playlist_locked | flag_on_playlist_created
                | flag_on_playlists_reorder | flag_on_playlists_removed | flag_on_playlists_removing));

    wil::com_ptr_t<PlaylistViewDropTarget> IDT_playlist = new PlaylistViewDropTarget(this);
    RegisterDragDrop(get_wnd(), IDT_playlist.get());

    if (g_windows.empty())
        s_create_message_window();

    g_windows.push_back(this);

    set_day_timer();

    console::formatter formatter;
    formatter << "Playlist view initialised in: " << pfc::format_float(timer.query(), 0, 3) << " s";
}

void PlaylistView::notify_on_destroy()
{
    if (const auto active_playlist = m_playlist_api->get_active_playlist();
        active_playlist != std::numeric_limits<size_t>::max())
        m_playlist_cache.set_item(active_playlist, save_scroll_position());

    std::erase(g_windows, this);
    if (g_windows.empty())
        s_destroy_message_window();

    RevokeDragDrop(get_wnd());
    m_playlist_api->unregister_callback(this);
    m_playlist_cache.deinitialise();
    m_column_data.remove_all();
    m_script_global.release();
    m_script_global_style.release();
    m_playlist_api.release();
    m_column_mask.set_size(0);

    m_library_autocomplete_v1.reset();
    m_library_autocomplete_v2.reset();

    m_artwork_manager->deinitialise();
    m_artwork_manager.reset();

    m_selection_holder.release();

    if (m_gdiplus_initialised) {
        Gdiplus::GdiplusShutdown(m_gdiplus_token);
        m_gdiplus_initialised = false;
        m_gdiplus_token = NULL;
    }
}

void PlaylistView::notify_on_set_focus(HWND wnd_lost)
{
    m_selection_holder = ui_selection_manager::get()->acquire();
    m_selection_holder->set_playlist_selection_tracking();
}
void PlaylistView::notify_on_kill_focus(HWND wnd_receiving)
{
    m_selection_holder.release();
}
bool PlaylistView::notify_on_contextmenu_header(const POINT& pt, const HDHITTESTINFO& hittest)
{
    uie::window_ptr p_this_temp = this;
    enum {
        IDM_ASC = 1,
        IDM_DES = 2,
        IDM_SEL_ASC,
        IDM_SEL_DES,
        IDM_AUTOSIZE,
        IDM_PREFS,
        IDM_EDIT_COLUMN,
        IDM_ARTWORK,
        IDM_CUSTOM_BASE
    };

    HMENU menu = CreatePopupMenu();
    HMENU selection_menu = CreatePopupMenu();
    size_t index = pfc_infinite;
    if (!(hittest.flags & HHT_NOWHERE) && is_header_column_real(hittest.iItem)) {
        index = header_column_to_real_column(hittest.iItem);
        uAppendMenu(menu, (MF_STRING), IDM_ASC, "&Sort ascending");
        uAppendMenu(menu, (MF_STRING), IDM_DES, "Sort &descending");
        uAppendMenu(selection_menu, (MF_STRING), IDM_SEL_ASC, "Sort a&scending");
        uAppendMenu(selection_menu, (MF_STRING), IDM_SEL_DES, "Sort d&escending");
        uAppendMenu(menu, MF_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(selection_menu), "Se&lection");
        uAppendMenu(menu, (MF_SEPARATOR), 0, "");
        uAppendMenu(menu, (MF_STRING), IDM_EDIT_COLUMN, "&Edit this column");
        uAppendMenu(menu, (MF_SEPARATOR), 0, "");
        uAppendMenu(
            menu, (MF_STRING | (cfg_nohscroll ? MF_CHECKED : MF_UNCHECKED)), IDM_AUTOSIZE, "&Auto-sizing columns");
        uAppendMenu(menu, (MF_STRING), IDM_PREFS, "&Preferences");
        uAppendMenu(menu, (MF_SEPARATOR), 0, "");
        uAppendMenu(menu, (MF_STRING | (cfg_show_artwork ? MF_CHECKED : MF_UNCHECKED)), IDM_ARTWORK, "&Artwork");
        uAppendMenu(menu, (MF_SEPARATOR), 0, "");

        pfc::string8 playlist_name;
        const auto playlist_api = playlist_manager::get();
        playlist_api->activeplaylist_get_name(playlist_name);

        pfc::string8_fast_aggressive filter;
        pfc::string8_fast_aggressive name;

        const auto e = g_columns.get_count();
        for (size_t s = 0; s < e; s++) {
            bool add = false;
            switch (g_columns[s]->filter_type) {
            case FILTER_NONE: {
                add = true;
                break;
            }
            case FILTER_SHOW: {
                if (wildcard_helper::test(playlist_name, g_columns[s]->filter, true)) {
                    add = true;
                }
            } break;
            case FILTER_HIDE: {
                if (!wildcard_helper::test(playlist_name, g_columns[s]->filter, true)) {
                    add = true;
                }
            } break;
            }
            if (add) {
                uAppendMenu(menu, MF_STRING | (g_columns[s]->show ? MF_CHECKED : MF_UNCHECKED), IDM_CUSTOM_BASE + s,
                    g_columns[s]->name);
            }
        }

    } else {
        uAppendMenu(
            menu, (MF_STRING | (cfg_nohscroll ? MF_CHECKED : MF_UNCHECKED)), IDM_AUTOSIZE, "&Auto-sizing columns");
        uAppendMenu(menu, (MF_STRING), IDM_PREFS, "&Preferences");
        uAppendMenu(menu, (MF_SEPARATOR), 0, "");
        uAppendMenu(menu, (MF_STRING | (cfg_show_artwork ? MF_CHECKED : MF_UNCHECKED)), IDM_ARTWORK, "&Artwork");
    }

    menu_helpers::win32_auto_mnemonics(menu);

    int cmd = TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, get_wnd(), nullptr);
    DestroyMenu(menu);

    if (cmd == IDM_ASC) {
        sort_by_column(index, false);
    } else if (cmd == IDM_DES) {
        sort_by_column(index, true);
    } else if (cmd == IDM_SEL_ASC) {
        sort_by_column(index, false, true);
    } else if (cmd == IDM_SEL_DES) {
        sort_by_column(index, true, true);
    } else if (cmd == IDM_EDIT_COLUMN) {
        TabColumns::get_instance().show_column(column_index_display_to_actual(index));
    } else if (cmd == IDM_AUTOSIZE) {
        cfg_nohscroll = cfg_nohscroll == 0;
        g_on_autosize_change();
    } else if (cmd == IDM_PREFS) {
        ui_control::get()->show_preferences(columns::config_get_playlist_view_guid());
    } else if (cmd == IDM_ARTWORK) {
        cfg_show_artwork = !cfg_show_artwork;
        g_on_show_artwork_change();
    } else if (cmd >= IDM_CUSTOM_BASE) {
        if (size_t(cmd - IDM_CUSTOM_BASE) < g_columns.get_count()) {
            g_columns[cmd - IDM_CUSTOM_BASE]->show = !g_columns[cmd - IDM_CUSTOM_BASE]->show; // g_columns
            g_on_columns_change();
        }
    }
    return true;
}
void PlaylistView::notify_on_menu_select(WPARAM wp, LPARAM lp)
{
    if (HIWORD(wp) & MF_POPUP) {
        m_status_text_override.release();
    } else {
        if (m_contextmenu_manager.is_valid() || m_mainmenu_manager.is_valid()) {
            unsigned id = LOWORD(wp);

            bool set = false;

            pfc::string8 desc;

            if (m_mainmenu_manager.is_valid() && id < m_contextmenu_manager_base) {
                set = m_mainmenu_manager->get_description(id - m_mainmenu_manager_base, desc);
            } else if (m_contextmenu_manager.is_valid() && id >= m_contextmenu_manager_base) {
                contextmenu_node* node = m_contextmenu_manager->find_by_id(id - m_contextmenu_manager_base);
                if (node)
                    set = node->get_description(desc);
            }

            ui_status_text_override::ptr p_status_override;

            if (set) {
                get_host()->override_status_text_create(p_status_override);

                if (p_status_override.is_valid()) {
                    p_status_override->override_text(desc);
                }
            }
            m_status_text_override = p_status_override;
        }
    }
}

bool PlaylistView::notify_on_contextmenu(const POINT& pt, bool from_keyboard)
{
    enum {
        ID_PLAY = 1,
        ID_CUT,
        ID_COPY,
        ID_PASTE,
        ID_SELECTION,
        ID_CUSTOM_BASE = 0x8000,
    };

    playlist_position_reference_tracker selected_item;
    const auto selection_count = m_playlist_api->activeplaylist_get_selection_count(2);
    const auto playlist_selection_exists = selection_count > 0;
    const auto show_shortcuts = standard_config_objects::query_show_keyboard_shortcuts_in_menus();
    HMENU menu = CreatePopupMenu();

    auto mainmenu_api = mainmenu_manager::get();
    const auto mainmenu_flags = show_shortcuts ? mainmenu_manager::flag_show_shortcuts : 0;
    const auto mainmenu_part
        = playlist_selection_exists ? mainmenu_groups::edit_part2_selection : mainmenu_groups::edit_part1;

    if (selection_count == 1) {
        selected_item.m_playlist = m_playlist_api->get_active_playlist();

        pfc::bit_array_bittable selection(m_playlist_api->activeplaylist_get_item_count());
        m_playlist_api->activeplaylist_get_selection_mask(selection);

        for (const auto index : std::ranges::views::iota(size_t{}, selection.size())) {
            if (selection[index]) {
                selected_item.m_item = index;
                break;
            }
        }

        constexpr auto play_text = L"Play"sv;

        MENUITEMINFO mii{};
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_STRING | MIIM_ID | MIIM_STATE;
        mii.dwTypeData = const_cast<wchar_t*>(play_text.data());
        mii.cch = gsl::narrow<UINT>(play_text.size());
        mii.wID = ID_PLAY;
        mii.fState = MFS_DEFAULT;

        InsertMenuItem(menu, 0, TRUE, &mii);
        AppendMenu(menu, MF_SEPARATOR, 0, nullptr);
    }

    mainmenu_api->instantiate(mainmenu_part);
    mainmenu_api->generate_menu_win32(menu, ID_SELECTION, ID_CUSTOM_BASE - ID_SELECTION, mainmenu_flags);

    if (GetMenuItemCount(menu) > 0)
        uAppendMenu(menu, MF_SEPARATOR, 0, "");

    if (playlist_selection_exists) {
        AppendMenu(menu, MF_STRING, ID_CUT, L"Cut");
        AppendMenu(menu, MF_STRING, ID_COPY, L"Copy");
    }

    const auto can_paste = playlist_utils::check_clipboard();
    if (can_paste)
        AppendMenu(menu, MF_STRING, ID_PASTE, L"Paste");
    else if (!playlist_selection_exists)
        AppendMenu(menu, MF_STRING | MF_GRAYED, 0, L"Paste");

    contextmenu_manager::ptr contextmenu_api;

    if (playlist_selection_exists) {
        AppendMenu(menu, MF_SEPARATOR, 0, nullptr);

        contextmenu_api = contextmenu_manager::get();
        const auto contextmenu_flags = show_shortcuts ? contextmenu_manager::flag_show_shortcuts : 0;

        const keyboard_shortcut_manager::shortcut_type shortcuts[]
            = {keyboard_shortcut_manager::TYPE_CONTEXT_PLAYLIST, keyboard_shortcut_manager::TYPE_CONTEXT};
        contextmenu_api->set_shortcut_preference(shortcuts, gsl::narrow<unsigned>(std::size(shortcuts)));
        contextmenu_api->init_context_playlist(contextmenu_flags);
        contextmenu_api->win32_build_menu(menu, ID_CUSTOM_BASE, -1);
    }

    menu_helpers::win32_auto_mnemonics(menu);
    m_contextmenu_manager_base = ID_CUSTOM_BASE;
    m_mainmenu_manager_base = ID_SELECTION;
    m_mainmenu_manager = mainmenu_api;
    m_contextmenu_manager = contextmenu_api;

    uie::window_ptr self_ptr = this;
    const auto tpm_flags = TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD;
    const int cmd = TrackPopupMenu(menu, tpm_flags, pt.x, pt.y, 0, get_wnd(), nullptr);

    DestroyMenu(menu);
    m_status_text_override.release();
    m_mainmenu_manager.release();
    m_contextmenu_manager.release();

    if (cmd == ID_PLAY) {
        if (selected_item.m_playlist != std::numeric_limits<size_t>::max()
            && selected_item.m_item != std::numeric_limits<size_t>::max()) {
            m_playlist_api->playlist_execute_default_action(selected_item.m_playlist, selected_item.m_item);
        }
    } else if (cmd == ID_CUT) {
        playlist_utils::cut();
    } else if (cmd == ID_COPY) {
        playlist_utils::copy();
    } else if (cmd == ID_PASTE) {
        if (playlist_selection_exists || from_keyboard) {
            playlist_utils::paste_at_focused_item(get_wnd());
        } else {
            playlist_utils::paste(get_wnd(), (std::numeric_limits<size_t>::max)());
        }
    } else if (cmd >= ID_SELECTION && cmd < ID_CUSTOM_BASE && mainmenu_api.is_valid()) {
        mainmenu_api->execute_command(cmd - ID_SELECTION);
    } else if (cmd >= ID_CUSTOM_BASE && contextmenu_api.is_valid()) {
        contextmenu_api->execute_by_id(cmd - ID_CUSTOM_BASE);
    }
    return true;
}

void PlaylistView::notify_update_item_data(size_t index)
{
    string_array& p_out = get_item_subitems(index);
    PlaylistViewItem* p_item = get_item(index);

    pfc::string8_fast_aggressive temp;
    pfc::string8_fast_aggressive str_dummy;
    temp.prealloc(32);
    GlobalVariableList globals;
    bool b_global = m_script_global.is_valid();
    SYSTEMTIME st{};
    GetLocalTime(&st);

    DateTitleformatHook tf_hook_date(&st);
    PlaylistNameTitleformatHook tf_hook_playlist_name;

    if (b_global) {
        SetGlobalTitleformatHook<true, false> tf_hook_set_global(globals);
        SplitterTitleformatHook tf_hook(&tf_hook_set_global, &tf_hook_date, &tf_hook_playlist_name);
        m_playlist_api->activeplaylist_item_format_title(
            index, &tf_hook, str_dummy, m_script_global, nullptr, play_control::display_level_all);
    }

    CellStyleData style_data_item = CellStyleData::g_create_default();

    bool colour_global_av = false;
    size_t i;
    size_t count = m_column_data.get_count();
    size_t count_display_groups = get_item_display_group_count(index);
    p_out.resize(count);
    get_item(index)->m_style_data.set_count(count);

    metadb_handle_ptr ptr;
    m_playlist_api->activeplaylist_get_item_handle(ptr, index);

    SetGlobalTitleformatHook<false, true> tf_hook_get_global(globals);

    size_t item_index = get_item_display_index(index);
    for (i = 0; i < count_display_groups; i++) {
        size_t count_groups = p_item->get_group_count();
        CellStyleData style_data_group = CellStyleData::g_create_default();
        if (ptr.is_valid() && m_script_global_style.is_valid()) {
            StyleTitleformatHook tf_hook_style(style_data_group, item_index - i - 1, true);
            SplitterTitleformatHook tf_hook(
                &tf_hook_style, b_global ? &tf_hook_get_global : nullptr, &tf_hook_date, &tf_hook_playlist_name);
            ptr->format_title(&tf_hook, temp, m_script_global_style, nullptr);
        }
        // count_display_groups > 0 => count_groups > 0
        style_cache_manager::g_add_object(style_data_group, p_item->get_group(count_groups - i - 1)->m_style_data);
    }

    for (i = 0; i < count; i++) {
        {
            SplitterTitleformatHook tf_hook(
                b_global ? &tf_hook_get_global : nullptr, &tf_hook_date, &tf_hook_playlist_name);
            m_playlist_api->activeplaylist_item_format_title(
                index, &tf_hook, temp, m_column_data[i].m_display_script, nullptr, playback_control::display_level_all);
            p_out[i] = temp;
        }

        CellStyleData style_temp = CellStyleData::g_create_default();

        bool b_custom = m_column_data[i].m_style_script.is_valid();

        {
            if (!colour_global_av) {
                if (m_script_global_style.is_valid()) {
                    StyleTitleformatHook tf_hook_style(style_data_item, item_index);
                    SplitterTitleformatHook tf_hook(&tf_hook_style, b_global ? &tf_hook_get_global : nullptr,
                        &tf_hook_date, &tf_hook_playlist_name);
                    m_playlist_api->activeplaylist_item_format_title(
                        index, &tf_hook, temp, m_script_global_style, nullptr, play_control::display_level_all);
                }

                colour_global_av = true;
            }
            style_temp = style_data_item;
        }
        if (b_custom) {
            if (m_column_data[i].m_style_script.is_valid()) {
                StyleTitleformatHook tf_hook_style(style_temp, item_index);
                SplitterTitleformatHook tf_hook(
                    &tf_hook_style, b_global ? &tf_hook_get_global : nullptr, &tf_hook_date, &tf_hook_playlist_name);
                m_playlist_api->activeplaylist_item_format_title(
                    index, &tf_hook, temp, m_column_data[i].m_style_script, nullptr, play_control::display_level_all);
            }
        }
        style_cache_manager::g_add_object(style_temp, get_item(index)->m_style_data[i]);
    }
}

const style_data_t& PlaylistView::get_style_data(size_t index)
{
    if (get_item(index)->m_style_data.get_count() != get_column_count()) {
        notify_update_item_data(index);
    }
    return get_item(index)->m_style_data;
}
bool PlaylistView::notify_on_middleclick(bool on_item, size_t index)
{
    return playlist_item_helpers::MiddleClickActionManager::run(cfg_playlist_middle_action, on_item, index);
}
bool PlaylistView::notify_on_doubleleftclick_nowhere()
{
    if (cfg_playlist_double.get_value().m_command != pfc::guid_null)
        return helpers::execute_main_menu_command(cfg_playlist_double);
    return false;
}

void PlaylistView::get_insert_items(
    /*size_t p_playlist, */ size_t start, size_t count, InsertItemsContainer& items)
{
    items.set_count(count);

    metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
    handles.prealloc(count);

    bit_array_range bit_table(start, count);
    m_playlist_api->activeplaylist_get_items(handles, bit_table);

    const auto group_count = m_scripts.get_count();
    const auto metadb_v2_api = metadb_v2::tryGet();

    if (metadb_v2_api.is_valid()) {
        metadb_v2_api->queryMultiParallel_(
            handles, [this, &handles, &items, group_count](size_t index, const metadb_v2::rec_t& rec) {
                metadb_handle_v2::ptr track;
                track &= handles[index];

                std::string title;
                mmh::StringAdaptor adapted_string(title);

                items[index].m_groups.resize(group_count);

                for (auto&& [script, group] : ranges::views::zip(m_scripts, items[index].m_groups)) {
                    track->formatTitle_v2(rec, nullptr, adapted_string, script, nullptr);
                    group = title.c_str();
                }
            });

        return;
    }

    concurrency::parallel_for(size_t{0}, count, [this, &items, &handles, group_count](size_t index) {
        pfc::string8_fast temp;
        temp.prealloc(32);
        items[index].m_groups.resize(group_count);
        for (size_t i = 0; i < group_count; i++) {
            handles[index]->format_title(nullptr, temp, m_scripts[i], nullptr);
            items[index].m_groups[i] = temp;
        }
    });
}

void PlaylistView::flush_items()
{
    InsertItemsContainer items;
    get_insert_items(0, m_playlist_api->activeplaylist_get_item_count(), items);
    replace_items(0, items);
}
void PlaylistView::reset_items()
{
    clear_all_items();
    InsertItemsContainer items;
    get_insert_items(0, m_playlist_api->activeplaylist_get_item_count(), items);
    insert_items(0, items.get_size(), items.get_ptr());
}

size_t PlaylistView::get_highlight_item()
{
    if (play_control::get()->is_playing()) {
        size_t playing_index;
        size_t playing_playlist;
        m_playlist_api->get_playing_item_location(&playing_playlist, &playing_index);
        if (playing_playlist == m_playlist_api->get_active_playlist())
            return playing_index;
    }
    return pfc_infinite;
}

bool PlaylistView::notify_on_keyboard_keydown_filter(UINT msg, WPARAM wp, LPARAM lp)
{
    uie::window_ptr p_this = this;
    bool ret = get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp);
    return ret;
}
bool PlaylistView::notify_on_keyboard_keydown_remove()
{
    m_playlist_api->activeplaylist_undo_backup();
    m_playlist_api->activeplaylist_remove_selection();
    return true;
}

bool PlaylistView::notify_on_keyboard_keydown_search()
{
    return standard_commands::main_playlist_search();
}

bool PlaylistView::notify_on_keyboard_keydown_undo()
{
    m_playlist_api->activeplaylist_undo_restore();
    return true;
}
bool PlaylistView::notify_on_keyboard_keydown_redo()
{
    m_playlist_api->activeplaylist_redo_restore();
    return true;
}
bool PlaylistView::notify_on_keyboard_keydown_cut()
{
    return playlist_utils::cut();
}
bool PlaylistView::notify_on_keyboard_keydown_copy()
{
    return playlist_utils::copy();
}
bool PlaylistView::notify_on_keyboard_keydown_paste()
{
    return playlist_utils::paste_at_focused_item(get_wnd());
}

size_t PlaylistView::storage_get_focus_item()
{
    return playlist_manager::get()->activeplaylist_get_focus_item();
}
void PlaylistView::storage_set_focus_item(size_t index)
{
    pfc::vartoggle_t<bool> tog(m_ignore_callback, true);
    playlist_manager::get()->activeplaylist_set_focus_item(index);
}
void PlaylistView::storage_get_selection_state(bit_array_var& out)
{
    playlist_manager::get()->activeplaylist_get_selection_mask(out);
}
bool PlaylistView::storage_set_selection_state(
    const bit_array& p_affected, const bit_array& p_status, bit_array_var* p_changed)
{
    pfc::vartoggle_t<bool> tog(m_ignore_callback, true);

    const auto api = playlist_manager::get();

    size_t count = api->activeplaylist_get_item_count();
    bit_array_bittable previous_state(count);

    api->activeplaylist_get_selection_mask(previous_state);

    bool b_changed = false;

    for (size_t i = 0; i < count; i++)
        if (p_affected.get(i) && (p_status.get(i) != previous_state.get(i))) {
            b_changed = true;
            if (p_changed)
                p_changed->set(i, true);
            else
                break;
        }

    api->activeplaylist_set_selection(p_affected, p_status);
    return b_changed;
}
bool PlaylistView::storage_get_item_selected(size_t index)
{
    return playlist_manager::get()->activeplaylist_is_item_selected(index);
}
size_t PlaylistView::storage_get_selection_count(size_t max)
{
    return playlist_manager::get()->activeplaylist_get_selection_count(max);
}

void PlaylistView::execute_default_action(size_t index, size_t column, bool b_keyboard, bool b_ctrl)
{
    if (b_keyboard && b_ctrl) {
        size_t active = m_playlist_api->get_active_playlist();
        if (active != -1)
            m_playlist_api->queue_add_item_playlist(active, index);
    } else {
        m_playlist_api->activeplaylist_execute_default_action(index);
    }
}
void PlaylistView::move_selection(int delta)
{
    m_playlist_api->activeplaylist_undo_backup();
    m_playlist_api->activeplaylist_move_selection(delta);
}

const GUID& PlaylistView::get_extension_guid() const
{
    return g_extension_guid;
}

void PlaylistView::get_name(pfc::string_base& out) const
{
    out.set_string("Playlist view");
}
bool PlaylistView::get_short_name(pfc::string_base& out) const
{
    out.set_string("Playlist");
    return true;
}
void PlaylistView::get_category(pfc::string_base& out) const
{
    out.set_string("Playlist Views");
}
unsigned PlaylistView::get_type() const
{
    return uie::type_panel | uie::type_playlist;
}

// {FB059406-5F14-4bd0-8A11-4242854CBBA5}
const GUID PlaylistView::g_extension_guid
    = {0xfb059406, 0x5f14, 0x4bd0, {0x8a, 0x11, 0x42, 0x42, 0x85, 0x4c, 0xbb, 0xa5}};

uie::window_factory<PlaylistView> g_pvt;

ColoursClient::factory<ColoursClient> g_appearance_client_ngpv_impl;

// {19F8E0B3-E822-4f07-B200-D4A67E4872F9}
const GUID g_guid_items_font = {0x19f8e0b3, 0xe822, 0x4f07, {0xb2, 0x0, 0xd4, 0xa6, 0x7e, 0x48, 0x72, 0xf9}};

// {30FBD64C-2031-4f0b-A937-F21671A2E195}
const GUID g_guid_header_font = {0x30fbd64c, 0x2031, 0x4f0b, {0xa9, 0x37, 0xf2, 0x16, 0x71, 0xa2, 0xe1, 0x95}};

// {FB127FFA-1B35-4572-9C1A-4B96A5C5D537}
const GUID g_guid_group_header_font = {0xfb127ffa, 0x1b35, 0x4572, {0x9c, 0x1a, 0x4b, 0x96, 0xa5, 0xc5, 0xd5, 0x37}};

class PlaylistViewItemFontClient : public fonts::client {
public:
    const GUID& get_client_guid() const override { return g_guid_items_font; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Playlist view: Items"; }

    fonts::font_type_t get_default_font_type() const override { return fonts::font_type_items; }

    void on_font_changed() const override { PlaylistView::g_on_font_change(); }
};

class PlaylistViewHeaderFontClient : public fonts::client {
public:
    const GUID& get_client_guid() const override { return g_guid_header_font; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Playlist view: Column titles"; }

    fonts::font_type_t get_default_font_type() const override { return fonts::font_type_items; }

    void on_font_changed() const override { PlaylistView::g_on_header_font_change(); }
};

class PlaylistViewGroupFontClient : public fonts::client {
public:
    const GUID& get_client_guid() const override { return g_guid_group_header_font; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Playlist view: Group titles"; }

    fonts::font_type_t get_default_font_type() const override { return fonts::font_type_items; }

    void on_font_changed() const override { PlaylistView::g_on_group_header_font_change(); }
};

PlaylistViewItemFontClient::factory<PlaylistViewItemFontClient> g_font_client_ngpv;
PlaylistViewHeaderFontClient::factory<PlaylistViewHeaderFontClient> g_font_header_client_ngpv;
PlaylistViewGroupFontClient::factory<PlaylistViewGroupFontClient> g_font_group_header_client_ngpv;

void ColoursClient::on_colour_changed(uint32_t mask) const
{
    if (cfg_show_artwork && cfg_artwork_reflection && (mask & (colours::colour_flag_background)))
        PlaylistView::s_flush_artwork();

    PlaylistView::s_update_all_items();
    PlaylistView::s_redraw_all();
}

void ColoursClient::on_bool_changed(uint32_t mask) const
{
    if (mask & colours::bool_flag_dark_mode_enabled)
        PlaylistView::s_on_dark_mode_status_change();
}

} // namespace cui::panels::playlist_view
