#include "pch.h"
#include "item_details.h"

#include "item_details_text.h"

namespace cui::panels::item_details {

// {59B4F428-26A5-4a51-89E5-3945D327B4CB}
const GUID g_guid_item_details = {0x59b4f428, 0x26a5, 0x4a51, {0x89, 0xe5, 0x39, 0x45, 0xd3, 0x27, 0xb4, 0xcb}};

// {A834BCF6-7230-4ff6-8F30-2ED826EEE1D3}
const GUID g_guid_item_details_tracking_mode
    = {0xa834bcf6, 0x7230, 0x4ff6, {0x8f, 0x30, 0x2e, 0xd8, 0x26, 0xee, 0xe1, 0xd3}};

// {16345DC1-2B8B-4351-A2B4-64736F667B22}
const GUID g_guid_item_details_script = {0x16345dc1, 0x2b8b, 0x4351, {0xa2, 0xb4, 0x64, 0x73, 0x6f, 0x66, 0x7b, 0x22}};

// {77F3FA70-E39C-46f8-8E8A-6ECC64DDE234}
const GUID g_guid_item_details_font_client
    = {0x77f3fa70, 0xe39c, 0x46f8, {0x8e, 0x8a, 0x6e, 0xcc, 0x64, 0xdd, 0xe2, 0x34}};

// {4E20CEED-42F6-4743-8EB3-610454457E19}
const GUID g_guid_item_details_colour_client
    = {0x4e20ceed, 0x42f6, 0x4743, {0x8e, 0xb3, 0x61, 0x4, 0x54, 0x45, 0x7e, 0x19}};

// {3E3D189A-8154-4c9f-8E68-B278301271CD}
const GUID g_guid_item_details_hscroll = {0x3e3d189a, 0x8154, 0x4c9f, {0x8e, 0x68, 0xb2, 0x78, 0x30, 0x12, 0x71, 0xcd}};

// {AE00D056-AACB-4ca0-A2EC-FD2BAB599C1B}
const GUID g_guid_item_details_horizontal_alignment
    = {0xae00d056, 0xaacb, 0x4ca0, {0xa2, 0xec, 0xfd, 0x2b, 0xab, 0x59, 0x9c, 0x1b}};

// {07526EBA-2E7A-4e03-83ED-7BDE8FF79E8E}
const GUID g_guid_item_details_vertical_alignment
    = {0x7526eba, 0x2e7a, 0x4e03, {0x83, 0xed, 0x7b, 0xde, 0x8f, 0xf7, 0x9e, 0x8e}};

// {41753F1F-F2D6-4385-BEFA-B4BEC44A4167}
const GUID g_guid_item_details_word_wrapping
    = {0x41753f1f, 0xf2d6, 0x4385, {0xbe, 0xfa, 0xb4, 0xbe, 0xc4, 0x4a, 0x41, 0x67}};

// {E944E1BF-0822-4141-B417-1F259D738ABC}
const GUID g_guid_item_details_edge_style
    = {0xe944e1bf, 0x822, 0x4141, {0xb4, 0x17, 0x1f, 0x25, 0x9d, 0x73, 0x8a, 0xbc}};

cfg_uint cfg_item_details_tracking_mode(g_guid_item_details_tracking_mode, 0);
cfg_uint cfg_item_details_edge_style(g_guid_item_details_edge_style, 0);
#if 0
cfg_string cfg_item_details_script(g_guid_item_details_script, "[%artist%]$crlf()[%title%]$crlf()[%album%][$crlf()$crlf()%lyrics%]");
#else
pfc::string8 cfg_item_details_script = "$set_font(%default_font_face%,$add(%default_font_size%,4),)[%artist%]$crlf()[%"
                                       "title%]$crlf()[%album%][$crlf()$crlf()%lyrics%]";
#endif
cfg_bool cfg_item_details_hscroll(g_guid_item_details_hscroll, false);
cfg_uint cfg_item_details_horizontal_alignment(g_guid_item_details_horizontal_alignment, uih::ALIGN_CENTRE);
cfg_uint cfg_item_details_vertical_alignment(
    g_guid_item_details_vertical_alignment, WI_EnumValue(VerticalAlignment::Centre));
cfg_bool cfg_item_details_word_wrapping(g_guid_item_details_word_wrapping, true);

void ItemDetails::MenuNodeOptions::execute()
{
    if (p_this->m_wnd_config)
        SetActiveWindow(p_this->m_wnd_config);
    else {
        ItemDetailsConfig(
            p_this->m_script, p_this->m_edge_style, p_this->m_horizontal_alignment, p_this->m_vertical_alignment)
            .run_modeless(GetAncestor(p_this->get_wnd(), GA_ROOT), p_this.get_ptr());
    }
}

ItemDetails::MenuNodeOptions::MenuNodeOptions(ItemDetails* p_wnd) : p_this(p_wnd) {}

bool ItemDetails::MenuNodeOptions::get_description(pfc::string_base& p_out) const
{
    return false;
}

bool ItemDetails::MenuNodeOptions::get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = "Options";
    p_displayflags = 0;
    return true;
}

bool ItemDetails::have_config_popup() const
{
    return true;
}
bool ItemDetails::show_config_popup(HWND wnd_parent)
{
    ItemDetailsConfig dialog(m_script, m_edge_style, m_horizontal_alignment, m_vertical_alignment);
    if (dialog.run_modal(wnd_parent)) {
        m_script = dialog.m_script;
        m_edge_style = dialog.m_edge_style;
        cfg_item_details_edge_style = m_edge_style;
        m_horizontal_alignment = dialog.m_horizontal_alignment;
        m_vertical_alignment = dialog.m_vertical_alignment;
        cfg_item_details_horizontal_alignment = m_horizontal_alignment;
        cfg_item_details_vertical_alignment = WI_EnumValue(m_vertical_alignment);

        if (get_wnd()) {
            m_to.release();
            titleformat_compiler::get()->compile_safe(m_to, m_script);

            on_edge_style_change();
            refresh_contents();
        }
        return true;
    }
    return false;
}

void ItemDetails::set_config(stream_reader* p_reader, size_t p_size, abort_callback& p_abort)
{
    if (p_size) {
        const auto version = p_reader->read_lendian_t<uint32_t>(p_abort);
        if (version <= stream_version_current) {
            p_reader->read_string(m_script, p_abort);
            p_reader->read_lendian_t(m_tracking_mode, p_abort);
            p_reader->read_lendian_t(m_hscroll, p_abort);
            p_reader->read_lendian_t(m_horizontal_alignment, p_abort);
            if (version >= 1) {
                p_reader->read_lendian_t(m_word_wrapping, p_abort);
                if (version >= 2) {
                    p_reader->read_lendian_t(m_edge_style, p_abort);

                    m_vertical_alignment = static_cast<VerticalAlignment>(p_reader->read_lendian_t<int32_t>(p_abort));
                }
            }
        }
    }
}

void ItemDetails::get_config(stream_writer* p_writer, abort_callback& p_abort) const
{
    p_writer->write_lendian_t(static_cast<uint32_t>(stream_version_current), p_abort);
    p_writer->write_string(m_script, p_abort);
    p_writer->write_lendian_t(m_tracking_mode, p_abort);
    p_writer->write_lendian_t(m_hscroll, p_abort);
    p_writer->write_lendian_t(m_horizontal_alignment, p_abort);
    p_writer->write_lendian_t(m_word_wrapping, p_abort);
    p_writer->write_lendian_t(m_edge_style, p_abort);
    p_writer->write_lendian_t(static_cast<int32_t>(m_vertical_alignment), p_abort);
}

void ItemDetails::get_menu_items(ui_extension::menu_hook_t& p_hook)
{
    p_hook.add_node(ui_extension::menu_node_ptr(new MenuNodeSourcePopup(this)));
    // p_node = new menu_node_alignment_popup(this);
    // p_hook.add_node(p_node);
    ui_extension::menu_node_ptr p_node = new MenuNodeHorizontalScrolling(this);
    p_hook.add_node(p_node);
    p_node = new MenuNodeWordWrap(this);
    p_hook.add_node(p_node);
    p_node = new MenuNodeOptions(this);
    p_hook.add_node(p_node);
}

void ItemDetails::s_on_app_activate(bool b_activated)
{
    for (auto& window : s_windows)
        window->on_app_activate(b_activated);
}
void ItemDetails::on_app_activate(bool b_activated)
{
    if (b_activated) {
        if (GetFocus() != get_wnd())
            register_callback();
    } else {
        deregister_callback();
    }
}

const GUID& ItemDetails::get_extension_guid() const
{
    return g_guid_item_details;
}
void ItemDetails::get_name(pfc::string_base& p_out) const
{
    p_out = "Item details";
}
void ItemDetails::get_category(pfc::string_base& p_out) const
{
    p_out = "Panels";
}
unsigned ItemDetails::get_type() const
{
    return uie::type_panel;
}

void ItemDetails::register_callback()
{
    if (!m_callback_registered)
        g_ui_selection_manager_register_callback_no_now_playing_fallback(this);
    m_callback_registered = true;
}
void ItemDetails::deregister_callback()
{
    if (m_callback_registered)
        ui_selection_manager::get()->unregister_callback(this);
    m_callback_registered = false;
}

void ItemDetails::update_scrollbar(ScrollbarType scrollbar_type, bool reset_position)
{
    if (m_is_updating_scroll_bars)
        return;

    m_is_updating_scroll_bars = true;
    auto _ = gsl::finally([this] { m_is_updating_scroll_bars = false; });

    update_display_info();

    double percentage_scrolled{};
    if (!reset_position) {
        SCROLLINFO si_old{};
        si_old.cbSize = sizeof(si_old);
        si_old.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;

        GetScrollInfo(get_wnd(), static_cast<int>(scrollbar_type), &si_old);

        if (const auto range = si_old.nMax - si_old.nMin + 1; range > 0)
            percentage_scrolled = static_cast<double>(si_old.nPos - si_old.nMin) / static_cast<double>(range);
    }

    SCROLLINFO si_new{};
    si_new.cbSize = sizeof(si_new);

    RECT rc{};
    GetClientRect(get_wnd(), &rc);

    si_new.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;

    if (scrollbar_type == ScrollbarType::vertical) {
        si_new.nPage = std::max(wil::rect_height(rc), 1l);
        si_new.nMin = m_text_rect ? m_text_rect->top : 0l;
        si_new.nMax = m_text_rect ? std::max(m_text_rect->bottom - 1, m_text_rect->top) : 0l;
    } else {
        si_new.nPage = std::max(wil::rect_width(rc), 1l);
        si_new.nMin = m_text_rect ? m_text_rect->left : 0l;
        si_new.nMax
            = m_text_rect && m_hscroll && !m_word_wrapping ? std::max(m_text_rect->right - 1, m_text_rect->left) : 0l;
    }

    const auto range = si_new.nMax - si_new.nMin + 1;

    if (range >= gsl::narrow<int>(si_new.nPage)) {
        if (!reset_position) {
            si_new.nPos = si_new.nMin + static_cast<int>(percentage_scrolled * static_cast<double>(range));
        } else if (scrollbar_type == ScrollbarType::vertical) {
            si_new.nPos = si_new.nMin;
        } else {
            if (m_horizontal_alignment == uih::ALIGN_RIGHT)
                si_new.nPos = si_new.nMax;
            else if (m_horizontal_alignment == uih::ALIGN_CENTRE)
                si_new.nPos = si_new.nMin + (range - gsl::narrow<int>(si_new.nPage)) / 2;
            else
                si_new.nPos = si_new.nMin;
        }
    }

    SetScrollInfo(get_wnd(), static_cast<int>(scrollbar_type), &si_new, TRUE);
}

void ItemDetails::update_scrollbars(bool reset_vertical_position, bool reset_horizontal_position)
{
    const auto last_client_height = m_last_cy;
    const auto last_client_width = m_last_cx;
    update_scrollbar(ScrollbarType::vertical, reset_vertical_position);
    update_scrollbar(ScrollbarType::horizontal, reset_horizontal_position);

    if (m_last_cy != last_client_height || (m_word_wrapping && m_last_cx != last_client_width))
        update_scrollbar(ScrollbarType::vertical, reset_vertical_position);
}

void ItemDetails::set_handles(const metadb_handle_list& handles)
{
    const auto old_handles = std::move(m_handles);
    m_handles = handles;
    if (handles.get_count() == 0 || old_handles.get_count() == 0 || handles[0].get_ptr() != old_handles[0].get_ptr()) {
        if (m_full_file_info_request) {
            m_full_file_info_request->abort();
            m_aborting_full_file_info_requests.emplace_back(std::move(m_full_file_info_request));
        }
        m_full_file_info_requested = false;
        m_full_file_info.reset();
    }
    refresh_contents(true, true);
}

void ItemDetails::request_full_file_info()
{
    if (static_api_test_t<metadb_v2>())
        return;

    if (m_full_file_info_requested)
        return;

    m_full_file_info_requested = true;

    if (m_handles.get_count() == 0)
        return;

    const auto handle = m_handles[0];
    if (filesystem::g_is_remote_or_unrecognized(handle->get_path()))
        return;
    m_full_file_info_request = std::make_unique<helpers::FullFileInfoRequest>(
        std::move(handle), [self = service_ptr_t<ItemDetails>{this}](auto&& request) {
            self->on_full_file_info_request_completion(std::forward<decltype(request)>(request));
        });
    m_full_file_info_request->queue();
}

void ItemDetails::on_full_file_info_request_completion(std::shared_ptr<helpers::FullFileInfoRequest> request)
{
    if (m_full_file_info_request == request) {
        m_full_file_info_request.reset();
        if (get_wnd()) {
            m_full_file_info = request->get_safe("Item details");
            refresh_contents(false, true);
        }
    }

    release_aborted_full_file_info_requests();
}

void ItemDetails::release_aborted_full_file_info_requests()
{
    const auto erase_iterator = std::remove_if(m_aborting_full_file_info_requests.begin(),
        m_aborting_full_file_info_requests.end(), [](auto&& item) { return item->is_ready(); });
    m_aborting_full_file_info_requests.erase(erase_iterator, m_aborting_full_file_info_requests.end());
}

void ItemDetails::release_all_full_file_info_requests()
{
    if (m_full_file_info_request) {
        m_full_file_info_request->abort();
        m_full_file_info_request->wait();
    }
    m_full_file_info_request.reset();
    for (auto&& request : m_aborting_full_file_info_requests) {
        request->wait();
    }
    m_aborting_full_file_info_requests.clear();
}

void ItemDetails::refresh_contents(bool reset_vertical_scroll_position, bool reset_horizontal_scroll_position)
{
    bool b_update = true;

    if (m_handles.get_count()) {
        LOGFONT lf;
        fb2k::std_api_get<fonts::manager>()->get_font(g_guid_item_details_font_client, lf);

        TitleformatHookChangeFont tf_hook(lf);
        pfc::string8_fast_aggressive temp;
        temp.prealloc(2048);

        if (m_nowplaying_active) {
            playback_control::get()->playback_format_title(
                &tf_hook, temp, m_to, nullptr, playback_control::display_level_all);
        } else {
            const auto handle = m_handles[0];
            if (m_full_file_info) {
                m_handles[0]->format_title_from_external_info(*m_full_file_info, &tf_hook, temp, m_to, nullptr);
            } else {
                request_full_file_info();
                m_handles[0]->format_title(&tf_hook, temp, m_to, nullptr);
            }
        }

        auto utf16_text = mmh::to_utf16(mmh::to_string_view(temp));
        if (utf16_text != m_formatted_text) {
            m_formatted_text = std::move(utf16_text);
        } else
            b_update = false;
    } else {
        m_formatted_text.clear();
    }

    if (b_update) {
        reset_display_info();

        update_scrollbars(reset_vertical_scroll_position, reset_horizontal_scroll_position);

        invalidate_all();
    }
}

void ItemDetails::update_display_info()
{
    create_text_layout();

    if (!m_text_layout || m_text_rect)
        return;

    DWRITE_TEXT_METRICS text_metrics{};
    DWRITE_OVERHANG_METRICS overhang_metrics{};

    try {
        text_metrics = m_text_layout->get_metrics();
        overhang_metrics = m_text_layout->get_overhang_metrics();
    }
    CATCH_LOG_RETURN()

    const auto scaling_factor = uih::direct_write::get_default_scaling_factor();
    const auto layout_width = m_text_layout->get_max_width();
    const auto layout_height = m_text_layout->get_max_height();
    const auto padding = s_get_padding();

    RECT client_rect{};
    GetClientRect(get_wnd(), &client_rect);

    const auto scale_min
        = [scaling_factor](float value) { return gsl::narrow_cast<long>(std::floor(value * scaling_factor)); };
    const auto scale_max
        = [scaling_factor](float value) { return gsl::narrow_cast<long>(std::ceil(value * scaling_factor)); };

    RECT text_rect = {
        scale_min(text_metrics.left),
        scale_min(text_metrics.top),
        scale_max(text_metrics.left + text_metrics.width),
        scale_max(text_metrics.top + text_metrics.height),
    };

    RECT overhang_rect = {
        scale_min(-overhang_metrics.left),
        scale_min(-overhang_metrics.top),
        scale_max(layout_width + overhang_metrics.right),
        scale_max(layout_height + overhang_metrics.bottom),
    };

    m_text_rect = {std::min(text_rect.left, overhang_rect.left) - padding,
        std::min(text_rect.top, overhang_rect.top) - padding, std::max(text_rect.right, overhang_rect.right) + padding,
        std::max(text_rect.bottom, overhang_rect.bottom) + padding};
}

void ItemDetails::reset_display_info()
{
    m_text_layout.reset();
    m_text_rect.reset();
}

void ItemDetails::on_playback_new_track(metadb_handle_ptr p_track)
{
    if (g_track_mode_includes_now_playing(m_tracking_mode)) {
        m_nowplaying_active = true;
        set_handles(pfc::list_single_ref_t<metadb_handle_ptr>(p_track));
    }
}

void ItemDetails::on_playback_seek(double p_time)
{
    if (m_nowplaying_active)
        refresh_contents();
}
void ItemDetails::on_playback_pause(bool p_state)
{
    if (m_nowplaying_active)
        refresh_contents();
}
void ItemDetails::on_playback_edited(metadb_handle_ptr p_track)
{
    if (m_nowplaying_active)
        refresh_contents();
}
void ItemDetails::on_playback_dynamic_info(const file_info& p_info)
{
    if (m_nowplaying_active)
        refresh_contents();
}
void ItemDetails::on_playback_dynamic_info_track(const file_info& p_info)
{
    if (m_nowplaying_active)
        refresh_contents();
}
void ItemDetails::on_playback_time(double p_time)
{
    if (m_nowplaying_active)
        refresh_contents();
}

void ItemDetails::on_playback_stop(play_control::t_stop_reason p_reason)
{
    if (g_track_mode_includes_now_playing(m_tracking_mode) && p_reason != play_control::stop_reason_starting_another
        && p_reason != play_control::stop_reason_shutting_down) {
        m_nowplaying_active = false;

        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        if (m_tracking_mode == track_auto_playlist_playing) {
            playlist_manager_v3::get()->activeplaylist_get_selected_items(handles);
        } else if (m_tracking_mode == track_auto_selection_playing) {
            handles = m_selection_handles;
        }
        set_handles(handles);
    }
}

void ItemDetails::on_playlist_switch()
{
    if (g_track_mode_includes_plalist(m_tracking_mode)
        && (!g_track_mode_includes_auto(m_tracking_mode) || !play_control::get()->is_playing())) {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        playlist_manager_v3::get()->activeplaylist_get_selected_items(handles);
        set_handles(handles);
    }
}
void ItemDetails::on_items_selection_change(const bit_array& p_affected, const bit_array& p_state)
{
    if (g_track_mode_includes_plalist(m_tracking_mode)
        && (!g_track_mode_includes_auto(m_tracking_mode) || !play_control::get()->is_playing())) {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        playlist_manager_v3::get()->activeplaylist_get_selected_items(handles);
        set_handles(handles);
    }
}

void ItemDetails::on_changed_sorted(metadb_handle_list_cref p_items_sorted, bool p_fromhook)
{
    if (m_nowplaying_active)
        return;

    for (auto&& track : m_handles) {
        if (size_t index{};
            p_items_sorted.bsearch_t(pfc::compare_t<metadb_handle_ptr, metadb_handle_ptr>, track, index)) {
            refresh_contents();
            break;
        }
    }
}

bool ItemDetails::check_process_on_selection_changed()
{
    HWND wnd_focus = GetFocus();
    if (wnd_focus == nullptr)
        return false;

    DWORD processid = NULL;
    GetWindowThreadProcessId(wnd_focus, &processid);
    return processid == GetCurrentProcessId();
}

void ItemDetails::on_selection_changed(const pfc::list_base_const_t<metadb_handle_ptr>& p_selection)
{
    if (check_process_on_selection_changed()) {
        if (g_ui_selection_manager_is_now_playing_fallback())
            m_selection_handles.remove_all();
        else
            m_selection_handles = p_selection;

        if (g_track_mode_includes_selection(m_tracking_mode)
            && (!g_track_mode_includes_auto(m_tracking_mode) || !play_control::get()->is_playing())) {
            set_handles(m_selection_handles);
        }
    }

    // pfc::hires_timer timer;
    // timer.start();

    // console::formatter() << "Selection properties panel refreshed in: " << timer.query() << " seconds";
}

void ItemDetails::on_tracking_mode_change()
{
    metadb_handle_list handles;

    m_nowplaying_active = false;

    if (g_track_mode_includes_now_playing(m_tracking_mode) && play_control::get()->is_playing()) {
        metadb_handle_ptr item;
        if (playback_control::get()->get_now_playing(item))
            handles.add_item(item);
        m_nowplaying_active = true;
    } else if (g_track_mode_includes_plalist(m_tracking_mode)) {
        playlist_manager_v3::get()->activeplaylist_get_selected_items(handles);
    } else if (g_track_mode_includes_selection(m_tracking_mode)) {
        handles = m_selection_handles;
    }
    set_handles(handles);
}

void ItemDetails::update_now()
{
    RedrawWindow(get_wnd(), nullptr, nullptr, RDW_UPDATENOW);
}

void ItemDetails::invalidate_all(bool b_update)
{
    RedrawWindow(get_wnd(), nullptr, nullptr, RDW_INVALIDATE | (b_update ? RDW_UPDATENOW : NULL));
}

void ItemDetails::on_size()
{
    RECT rc;
    GetClientRect(get_wnd(), &rc);
    on_size(wil::rect_width(rc), wil::rect_height(rc));
}

void ItemDetails::on_size(size_t cx, size_t cy)
{
    if (m_text_layout) {
        const auto padding = s_get_padding();
        const auto max_width = std::max(0, gsl::narrow<int>(cx) - padding * 2);
        const auto max_height = std::max(0, gsl::narrow<int>(cy) - padding * 2);

        try {
            m_text_layout->set_max_width(uih::direct_write::px_to_dip(gsl::narrow_cast<float>(max_width)));
            m_text_layout->set_max_height(uih::direct_write::px_to_dip(gsl::narrow_cast<float>(max_height)));
        }
        CATCH_LOG()

        m_text_rect.reset();
    }

    invalidate_all(false);

    if (cx != m_last_cx) {
        m_last_cx = cx;
        update_scrollbar(ScrollbarType::horizontal, false);
    }

    if (m_word_wrapping || cy != m_last_cy) {
        m_last_cy = cy;
        update_scrollbar(ScrollbarType::vertical, false);
    }
}

void ItemDetails::scroll(INT SB, int position, bool b_absolute)
{
    SCROLLINFO si{};
    SCROLLINFO si2{};
    si.cbSize = sizeof(si);
    si.fMask = SIF_POS | SIF_TRACKPOS | SIF_PAGE | SIF_RANGE;
    GetScrollInfo(get_wnd(), SB, &si);

    int new_pos = si.nPos;

    if (b_absolute)
        new_pos = si.nPos + position;
    else
        new_pos = position;

    if (new_pos < si.nMin)
        new_pos = si.nMin;
    if (new_pos > si.nMax)
        new_pos = si.nMax;

    if (new_pos != si.nPos) {
        si2.cbSize = sizeof(si);
        si2.fMask = SIF_POS;
        si2.nPos = new_pos;
        new_pos = SetScrollInfo(get_wnd(), SB, &si2, TRUE);

        RECT rc;
        GetClientRect(get_wnd(), &rc);
        int dy = SB == SB_VERT ? si.nPos - new_pos : 0;
        int dx = SB == SB_HORZ ? si.nPos - new_pos : 0;
        ScrollWindowEx(get_wnd(), dx, dy, &rc, &rc, nullptr, nullptr, SW_INVALIDATE);
        RedrawWindow(get_wnd(), nullptr, nullptr, RDW_UPDATENOW);
    }
}

void ItemDetails::set_window_theme() const
{
    SetWindowTheme(get_wnd(), colours::is_dark_mode_active() ? L"DarkMode_Explorer" : nullptr, nullptr);
}

LRESULT ItemDetails::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        set_window_theme();
        register_callback();
        play_callback_manager::get()->register_callback(
            this, flag_on_playback_all & ~(flag_on_volume_change | flag_on_playback_starting), false);
        playlist_manager_v3::get()->register_callback(this, playlist_callback_flags);
        metadb_io_v3::get()->register_callback(this);

        try {
            m_direct_write_context = uih::direct_write::Context::s_create();
        }
        CATCH_LOG()

        recreate_text_format();

        if (s_windows.empty())
            s_create_message_window();

        s_windows.push_back(this);

        titleformat_compiler::get()->compile_safe(m_to, m_script);

        on_size(/*lpcs->cx, lpcs->cy*/);
        on_tracking_mode_change();
        refresh_contents(true, true);
    } break;
    case WM_DESTROY: {
        std::erase(s_windows, this);

        if (s_windows.empty())
            s_destroy_message_window();

        play_callback_manager::get()->unregister_callback(this);
        metadb_io_v3::get()->unregister_callback(this);
        playlist_manager_v3::get()->unregister_callback(this);
        deregister_callback();
        release_all_full_file_info_requests();
        m_handles.remove_all();
        m_selection_handles.remove_all();
        m_selection_holder.release();
        m_to.release();
        m_text_format.reset();
        m_text_layout.reset();
        m_direct_write_context.reset();
        m_last_cx = 0;
        m_last_cy = 0;
    } break;
    case WM_SETFOCUS:
        deregister_callback();
        m_selection_holder = ui_selection_manager::get()->acquire();
        m_selection_holder->set_selection(m_handles);
        break;
    case WM_KILLFOCUS:
        m_selection_holder.release();
        register_callback();
        break;
    case WM_WINDOWPOSCHANGED: {
        auto lpwp = (LPWINDOWPOS)lp;
        if (!(lpwp->flags & SWP_NOSIZE) || (lpwp->flags & SWP_FRAMECHANGED)) {
            on_size();
        }
    } break;
    case WM_MOUSEWHEEL: {
        LONG_PTR style = GetWindowLongPtr(get_wnd(), GWL_STYLE);
        bool b_horz = (!(style & WS_VSCROLL) || ((wp & MK_CONTROL))) && (style & WS_HSCROLL);

        SCROLLINFO si{};
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_POS | SIF_TRACKPOS | SIF_PAGE | SIF_RANGE;
        GetScrollInfo(get_wnd(), b_horz ? SB_HORZ : SB_VERT, &si);

        UINT scroll_lines = 3; // 3 is default
        SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &scroll_lines, 0); // we don't support Win9X

        const auto line_height = m_text_format ? m_text_format->get_minimum_height() : 0;

        if (!si.nPage)
            si.nPage++;

        if (scroll_lines == -1)
            scroll_lines = si.nPage - 1;
        else
            scroll_lines *= line_height;

        int zDelta = short(HIWORD(wp));

        if (scroll_lines == 0)
            scroll_lines = 1;

        int delta = -MulDiv(zDelta, scroll_lines, 120);

        // Limit scrolling to one page ?!?!?! It was in Columns Playlist code...
        if (delta < 0 && (UINT)(delta * -1) > si.nPage) {
            delta = si.nPage * -1;
            if (delta < -1)
                delta++;
        } else if (delta > 0 && (UINT)delta > si.nPage) {
            delta = si.nPage;
            if (delta > 1)
                delta--;
        }

        scroll(b_horz ? SB_HORZ : SB_VERT, delta, true);
    }
        return 0;
    case WM_HSCROLL:
    case WM_VSCROLL: {
        UINT SB = msg == WM_VSCROLL ? SB_VERT : SB_HORZ;
        SCROLLINFO si{};
        SCROLLINFO si2{};
        si.cbSize = sizeof(si);
        si.fMask = SIF_POS | SIF_TRACKPOS | SIF_PAGE | SIF_RANGE;
        GetScrollInfo(get_wnd(), SB, &si);

        int new_pos = si.nPos;

        const auto line_height = m_text_format ? m_text_format->get_minimum_height() : 0;

        WORD p_value = LOWORD(wp);

        if (p_value == SB_LINEDOWN)
            new_pos = si.nPos + line_height;
        if (p_value == SB_LINEUP)
            new_pos = si.nPos - line_height;
        if (p_value == SB_PAGEUP)
            new_pos = si.nPos - si.nPage;
        if (p_value == SB_PAGEDOWN)
            new_pos = si.nPos + si.nPage;
        if (p_value == SB_THUMBTRACK)
            new_pos = si.nTrackPos;
        if (p_value == SB_BOTTOM)
            new_pos = si.nMax;
        if (p_value == SB_TOP)
            new_pos = si.nMin;

        if (new_pos < si.nMin)
            new_pos = si.nMin;
        if (new_pos > si.nMax)
            new_pos = si.nMax;

        if (new_pos != si.nPos) {
            si2.cbSize = sizeof(si);
            si2.fMask = SIF_POS;
            si2.nPos = new_pos;
            new_pos = SetScrollInfo(wnd, SB, &si2, TRUE);

            RECT rc;
            GetClientRect(get_wnd(), &rc);
            int dy = SB == SB_VERT ? si.nPos - new_pos : 0;
            int dx = SB == SB_HORZ ? si.nPos - new_pos : 0;
            ScrollWindowEx(get_wnd(), dx, dy, &rc, &rc, nullptr, nullptr, SW_INVALIDATE);
            RedrawWindow(get_wnd(), nullptr, nullptr, RDW_UPDATENOW);
        }
    }
        return 0;
    case WM_ERASEBKGND:
        return TRUE;
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        auto dc = wil::BeginPaint(wnd, &ps);

        SCROLLINFO siv{};
        siv.cbSize = sizeof(siv);
        siv.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
        SCROLLINFO sih = siv;
        GetScrollInfo(wnd, SB_VERT, &siv);
        GetScrollInfo(wnd, SB_HORZ, &sih);

        colours::helper p_helper(g_guid_item_details_colour_client);

        update_display_info();

        RECT rc_client;
        GetClientRect(wnd, &rc_client);

        const auto background_colour = p_helper.get_colour(colours::colour_background);
        const auto text_colour = p_helper.get_colour(colours::colour_text);

        if (m_text_layout && m_text_rect) {
            const auto is_horizontal_scroll_bar_visible
                = m_hscroll && !m_word_wrapping && (sih.nMax - sih.nMin - 1) > gsl::narrow_cast<int>(sih.nPage);
            const auto is_vertical_scroll_bar_visible = (siv.nMax - siv.nMin - 1) > gsl::narrow_cast<int>(siv.nPage);
            const auto x_offset = uih::direct_write::px_to_dip(
                gsl::narrow_cast<float>(is_horizontal_scroll_bar_visible ? -sih.nPos : 0));
            const auto y_offset
                = uih::direct_write::px_to_dip(gsl::narrow_cast<float>(is_vertical_scroll_bar_visible ? -siv.nPos : 0));

            try {
                m_text_layout->render_with_solid_background(
                    dc.get(), x_offset, y_offset, ps.rcPaint, background_colour, text_colour);
            }
            CATCH_LOG()
        }
    }
        return 0;
    }
    return DefWindowProc(wnd, msg, wp, lp);
}

void ItemDetails::s_on_font_change()
{
    for (auto& window : s_windows) {
        window->on_font_change();
    }
}

void ItemDetails::s_on_dark_mode_status_change()
{
    for (auto&& window : s_windows) {
        window->set_window_theme();
    }
}

void ItemDetails::s_on_colours_change()
{
    for (auto& window : s_windows) {
        window->on_colours_change();
    }
}

void ItemDetails::recreate_text_format()
{
    if (!m_direct_write_context)
        return;

    m_text_format = create_text_format(m_direct_write_context, static_cast<uih::alignment>(m_horizontal_alignment),
        m_vertical_alignment, m_word_wrapping);
    reset_display_info();
}

void ItemDetails::create_text_layout()
{
    if (!m_text_format || m_text_layout)
        return;

    RECT rect{};
    GetClientRect(get_wnd(), &rect);

    auto [render_text, colour_segments, font_segments] = process_colour_and_font_codes(m_formatted_text);

    const auto padding = s_get_padding();
    const auto max_width = std::max(0, gsl::narrow<int>(wil::rect_width(rect)) - padding * 2);
    const auto max_height = std::max(0, gsl::narrow<int>(wil::rect_height(rect)) - padding * 2);
    m_text_layout = item_details::create_text_layout(*m_text_format, max_width, max_height, render_text);

    if (!m_text_layout)
        return;

    for (auto& [colour, start_character, character_count] : colour_segments) {
        try {
            m_text_layout->set_colour(
                colour, {gsl::narrow<uint32_t>(start_character), gsl::narrow<uint32_t>(character_count)});
        }
        CATCH_LOG()
    }

    for (auto& [font, start_character, character_count] : font_segments) {
        LOGFONT lf{};
        wcsncpy_s(lf.lfFaceName, font.family.c_str(), _TRUNCATE);

        if (font.is_bold)
            lf.lfWeight = FW_BOLD;

        if (font.is_underline)
            lf.lfUnderline = TRUE;

        if (font.is_italic)
            lf.lfItalic = TRUE;

        try {
            const auto direct_write_font = m_direct_write_context->create_font(lf);

            const auto weight = direct_write_font->GetWeight();
            const auto stretch = direct_write_font->GetStretch();
            const auto style = direct_write_font->GetStyle();
            const auto size = uih::direct_write::pt_to_dip(font.size_points);
            const auto text_range
                = DWRITE_TEXT_RANGE{gsl::narrow<uint32_t>(start_character), gsl::narrow<uint32_t>(character_count)};

            m_text_layout->set_font(font.family.c_str(), weight, stretch, style, size, text_range);

            if (font.is_underline)
                m_text_layout->set_underline(true, text_range);
        }
        CATCH_LOG()
    }
}

void ItemDetails::on_font_change()
{
    recreate_text_format();
    reset_display_info();
    update_scrollbars(false, false);
    invalidate_all();
}

void ItemDetails::on_colours_change()
{
    invalidate_all();
}

ItemDetails::ItemDetails()
    : m_tracking_mode(cfg_item_details_tracking_mode)
    , m_script(cfg_item_details_script)
    , m_horizontal_alignment(cfg_item_details_horizontal_alignment)
    , m_vertical_alignment(static_cast<VerticalAlignment>(cfg_item_details_vertical_alignment.get_value()))
    , m_edge_style(cfg_item_details_edge_style)
    , m_hscroll(cfg_item_details_hscroll)
    , m_word_wrapping(cfg_item_details_word_wrapping)
//, m_update_scrollbar_range_in_progress(false)
// m_library_richedit(NULL), m_wnd_richedit(NULL)
{
}

void ItemDetails::s_create_message_window()
{
    uie::container_window_v3_config config(L"{6EB3EA81-7C5E-468d-B507-E5527F52361B}", false);
    config.window_styles = 0;
    config.extended_window_styles = 0;

    s_message_window = std::make_unique<uie::container_window_v3>(
        config, [](auto&& wnd, auto&& msg, auto&& wp, auto&& lp) -> LRESULT {
            if (msg == WM_ACTIVATEAPP)
                s_on_app_activate(wp != 0);
            return DefWindowProc(wnd, msg, wp, lp);
        });
    s_message_window->create(nullptr);
}

void ItemDetails::s_destroy_message_window()
{
    s_message_window->destroy();
    s_message_window.reset();
}

void ItemDetails::set_config_wnd(HWND wnd)
{
    m_wnd_config = wnd;
}

void ItemDetails::set_script(const char* p_script)
{
    m_script = p_script;

    if (get_wnd()) {
        m_to.release();
        titleformat_compiler::get()->compile_safe(m_to, m_script);

        on_edge_style_change();
        refresh_contents();
    }
}

void ItemDetails::on_edge_style_change()
{
    long flags = 0;

    if (m_edge_style == 1)
        flags |= WS_EX_CLIENTEDGE;
    if (m_edge_style == 2)
        flags |= WS_EX_STATICEDGE;
    SetWindowLongPtr(get_wnd(), GWL_EXSTYLE, flags);
    SetWindowPos(get_wnd(), nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
}

void ItemDetails::set_edge_style(uint32_t edge_style)
{
    m_edge_style = edge_style;
    if (get_wnd()) {
        on_edge_style_change();
    }
}

void ItemDetails::set_vertical_alignment(VerticalAlignment vertical_alignment)
{
    if (!get_wnd())
        return;

    m_vertical_alignment = vertical_alignment;

    if (m_text_format) {
        const auto paragraph_alignment = get_paragraph_alignment(m_vertical_alignment);

        try {
            m_text_format->set_paragraph_alignment(paragraph_alignment);
        }
        CATCH_LOG()
    }

    reset_display_info();
    invalidate_all(false);
    update_scrollbars(false, false);
    update_now();
}

void ItemDetails::set_horizontal_alignment(uint32_t horizontal_alignment)
{
    if (!get_wnd())
        return;

    m_horizontal_alignment = horizontal_alignment;

    if (m_text_format) {
        try {
            m_text_format->set_text_alignment(
                uih::direct_write::get_text_alignment(static_cast<uih::alignment>(horizontal_alignment)));
        }
        CATCH_LOG()
    }

    reset_display_info();
    invalidate_all(false);
    update_scrollbars(false, true);
    update_now();
}

void ItemDetails::on_playback_order_changed(size_t p_new_index) {}

void ItemDetails::on_default_format_changed() {}

void ItemDetails::on_playlist_locked(bool p_locked) {}

void ItemDetails::on_playlist_renamed(const char* p_new_name, size_t p_new_name_len) {}

void ItemDetails::on_item_ensure_visible(size_t p_idx) {}

void ItemDetails::on_items_replaced(
    const bit_array& p_mask, const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry>& p_data)
{
}

void ItemDetails::on_items_modified_fromplayback(const bit_array& p_mask, play_control::t_display_level p_level) {}

void ItemDetails::on_items_modified(const bit_array& p_mask) {}

void ItemDetails::on_items_removed(const bit_array& p_mask, size_t p_old_count, size_t p_new_count) {}

void ItemDetails::on_items_removing(const bit_array& p_mask, size_t p_old_count, size_t p_new_count) {}

void ItemDetails::on_items_reordered(const size_t* p_order, size_t p_count) {}

void ItemDetails::on_items_added(
    size_t p_base, const pfc::list_base_const_t<metadb_handle_ptr>& p_data, const bit_array& p_selection)
{
}

void ItemDetails::on_item_focus_change(size_t p_from, size_t p_to) {}

void ItemDetails::on_volume_change(float p_new_val) {}

void ItemDetails::on_playback_starting(play_control::t_track_command p_command, bool p_paused) {}

bool ItemDetails::g_track_mode_includes_selection(size_t mode)
{
    return mode == track_auto_selection_playing || mode == track_selection;
}

bool ItemDetails::g_track_mode_includes_auto(size_t mode)
{
    return mode == track_auto_playlist_playing || mode == track_auto_selection_playing;
}

bool ItemDetails::g_track_mode_includes_plalist(size_t mode)
{
    return mode == track_auto_playlist_playing || mode == track_playlist;
}

uie::container_window_v3_config ItemDetails::get_window_config()
{
    uie::container_window_v3_config config(L"columns_ui_item_details_E0D8v091EU8", false);

    if (m_edge_style == 1)
        config.extended_window_styles |= WS_EX_CLIENTEDGE;
    if (m_edge_style == 2)
        config.extended_window_styles |= WS_EX_STATICEDGE;

    return config;
}

bool ItemDetails::g_track_mode_includes_now_playing(size_t mode)
{
    return mode == track_auto_playlist_playing || mode == track_auto_selection_playing || mode == track_playing;
}

uie::window_factory<ItemDetails> g_item_details;

ItemDetails::MenuNodeWordWrap::MenuNodeWordWrap(ItemDetails* p_wnd) : p_this(p_wnd) {}

void ItemDetails::MenuNodeWordWrap::execute()
{
    p_this->m_word_wrapping = !p_this->m_word_wrapping;
    cfg_item_details_word_wrapping = p_this->m_word_wrapping;

    if (p_this->m_text_format) {
        try {
            p_this->m_text_format->set_word_wrapping(
                p_this->m_word_wrapping ? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP);
        }
        CATCH_LOG()
    }

    p_this->reset_display_info();
    p_this->invalidate_all(false);
    p_this->update_scrollbars(false, true);
    p_this->update_now();
}

bool ItemDetails::MenuNodeWordWrap::get_description(pfc::string_base& p_out) const
{
    return false;
}

bool ItemDetails::MenuNodeWordWrap::get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = "Word wrapping";
    p_displayflags = (p_this->m_word_wrapping) ? state_checked : 0;
    return true;
}

ItemDetails::MenuNodeHorizontalScrolling::MenuNodeHorizontalScrolling(ItemDetails* p_wnd) : p_this(p_wnd) {}

void ItemDetails::MenuNodeHorizontalScrolling::execute()
{
    p_this->m_hscroll = !p_this->m_hscroll;
    cfg_item_details_hscroll = p_this->m_hscroll;
    p_this->reset_display_info();
    p_this->invalidate_all(false);
    p_this->update_scrollbars(false, true);
    p_this->update_now();
}

bool ItemDetails::MenuNodeHorizontalScrolling::get_description(pfc::string_base& p_out) const
{
    return false;
}

bool ItemDetails::MenuNodeHorizontalScrolling::get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = "Allow horizontal scrolling";
    p_displayflags = (p_this->m_hscroll) ? state_checked : 0;
    return true;
}

ItemDetails::MenuNodeAlignmentPopup::MenuNodeAlignmentPopup(ItemDetails* p_wnd)
{
    m_items.add_item(new MenuNodeAlignment(p_wnd, 0));
    m_items.add_item(new MenuNodeAlignment(p_wnd, 1));
    m_items.add_item(new MenuNodeAlignment(p_wnd, 2));
}

void ItemDetails::MenuNodeAlignmentPopup::get_child(size_t p_index, uie::menu_node_ptr& p_out) const
{
    p_out = m_items[p_index].get_ptr();
}

size_t ItemDetails::MenuNodeAlignmentPopup::get_children_count() const
{
    return m_items.get_count();
}

bool ItemDetails::MenuNodeAlignmentPopup::get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = "Alignment";
    p_displayflags = 0;
    return true;
}

ItemDetails::MenuNodeAlignment::MenuNodeAlignment(ItemDetails* p_wnd, uint32_t p_value) : p_this(p_wnd), m_type(p_value)
{
}

void ItemDetails::MenuNodeAlignment::execute()
{
    p_this->m_horizontal_alignment = m_type;
    cfg_item_details_horizontal_alignment = m_type;
    p_this->invalidate_all(false);
    p_this->update_scrollbars(false, true);
    p_this->update_now();
}

bool ItemDetails::MenuNodeAlignment::get_description(pfc::string_base& p_out) const
{
    return false;
}

bool ItemDetails::MenuNodeAlignment::get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = get_name(m_type);
    p_displayflags = (m_type == p_this->m_horizontal_alignment) ? state_radiochecked : 0;
    return true;
}

const char* ItemDetails::MenuNodeAlignment::get_name(uint32_t source)
{
    if (source == 0)
        return "Left";
    if (source == 1)
        return "Centre";
    /*if (source == 2)*/
    return "Right";
}

ItemDetails::MenuNodeSourcePopup::MenuNodeSourcePopup(ItemDetails* p_wnd)
{
    m_items.add_item(new MenuNodeTrackMode(p_wnd, 3));
    m_items.add_item(new MenuNodeTrackMode(p_wnd, 0));
    m_items.add_item(new uie::menu_node_separator_t());
    m_items.add_item(new MenuNodeTrackMode(p_wnd, 2));
    m_items.add_item(new MenuNodeTrackMode(p_wnd, 4));
    m_items.add_item(new MenuNodeTrackMode(p_wnd, 1));
}

void ItemDetails::MenuNodeSourcePopup::get_child(size_t p_index, uie::menu_node_ptr& p_out) const
{
    p_out = m_items[p_index].get_ptr();
}

size_t ItemDetails::MenuNodeSourcePopup::get_children_count() const
{
    return m_items.get_count();
}

bool ItemDetails::MenuNodeSourcePopup::get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = "Displayed track";
    p_displayflags = 0;
    return true;
}

ItemDetails::MenuNodeTrackMode::MenuNodeTrackMode(ItemDetails* p_wnd, uint32_t p_value)
    : p_this(p_wnd)
    , m_source(p_value)
{
}

void ItemDetails::MenuNodeTrackMode::execute()
{
    p_this->m_tracking_mode = m_source;
    cfg_item_details_tracking_mode = m_source;
    p_this->on_tracking_mode_change();
}

bool ItemDetails::MenuNodeTrackMode::get_description(pfc::string_base& p_out) const
{
    return false;
}

bool ItemDetails::MenuNodeTrackMode::get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = get_name(m_source);
    p_displayflags = (m_source == p_this->m_tracking_mode) ? state_radiochecked : 0;
    return true;
}

const char* ItemDetails::MenuNodeTrackMode::get_name(uint32_t source)
{
    if (source == track_playing)
        return "Playing item";
    if (source == track_playlist)
        return "Playlist selection";
    if (source == track_auto_selection_playing)
        return "Automatic (current selection/playing item)";
    if (source == track_selection)
        return "Current selection";
    return "Automatic (playlist selection/playing item)";
}

} // namespace cui::panels::item_details
