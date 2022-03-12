#include "pch.h"

#include "filter_search_bar.h"

#include "filter_config_var.h"
#include "filter_utils.h"

namespace cui::panels::filter {

template <class tHandles>
void g_send_metadb_handles_to_playlist(tHandles& handles, bool b_play = false)
{
    const auto playlist_api = playlist_manager::get();
    const auto playback_api = play_control::get();
    size_t index_insert = pfc_infinite;
    if (!b_play && playback_api->is_playing()) {
        size_t playlist = playlist_api->get_playing_playlist();
        pfc::string8 name;
        if (playlist_api->playlist_get_name(playlist, name) && !stricmp_utf8("Filter Results", name)) {
            size_t index_old = playlist_api->find_playlist("Filter Results (Playback)", pfc_infinite);
            playlist_api->playlist_rename(playlist, "Filter Results (Playback)", pfc_infinite);
            index_insert = index_old < playlist ? playlist : playlist + 1;
            if (index_old != pfc_infinite)
                playlist_api->remove_playlist(index_old);
        }
    }
    // size_t index_remove = playlist_api->find_playlist("Filter Results", pfc_infinite);
    size_t index = NULL;
    if (index_insert != pfc_infinite)
        index = playlist_api->create_playlist(
            b_play ? "Filter Results (Playback)" : "Filter Results", pfc_infinite, index_insert);
    else
        index = playlist_api->find_or_create_playlist(
            b_play ? "Filter Results (Playback)" : "Filter Results", pfc_infinite);
    playlist_api->playlist_clear(index);

    sort_tracks(handles);
    playlist_api->playlist_add_items(index, handles, bit_array_false());

    playlist_api->set_active_playlist(index);
    if (b_play) {
        playlist_api->set_playing_playlist(index);
        playback_api->play_start(play_control::track_command_default);
    }
    // if (index_remove != pfc_infinite)
    //    playlist_api->remove_playlist(index+1);
}

void g_get_search_bar_sibling_streams(FilterSearchToolbar const* p_serach_bar, pfc::list_t<FilterStream::ptr>& p_out)
{
    if (cfg_orderedbysplitters && p_serach_bar->get_wnd() && p_serach_bar->get_host().is_valid()) {
        pfc::list_t<uie::window_ptr> siblings;
        uie::window_host_ex::ptr hostex;
        if (p_serach_bar->get_host()->service_query_t(hostex))
            hostex->get_children(siblings);

        // Let's avoid recursion for once
        size_t j = siblings.get_count();
        while (j) {
            j--;
            uie::window_ptr p_window = siblings[j];
            siblings.remove_by_idx(j);

            uie::splitter_window_ptr p_splitter;

            if (p_window->get_extension_guid() == guid_filter) {
                auto* p_filter = static_cast<FilterPanel*>(p_window.get_ptr());
                if (!p_out.have_item(p_filter->m_stream))
                    p_out.add_item(p_filter->m_stream);
            } else if (p_window->service_query_t(p_splitter)) {
                size_t splitter_child_count = p_splitter->get_panel_count();
                for (size_t k = 0; k < splitter_child_count; k++) {
                    uie::splitter_item_ptr p_splitter_child;
                    p_splitter->get_panel(k, p_splitter_child);
                    if (p_splitter_child->get_window_ptr().is_valid()
                        && p_splitter_child->get_window_ptr()->get_wnd()) {
                        siblings.add_item(p_splitter_child->get_window_ptr());
                        ++j;
                    }
                }
            }
        }
    }
}

namespace {
uie::window_factory<FilterSearchToolbar> g_filter_search_bar;
}

void FilterSearchToolbar::set_config(stream_reader* p_reader, size_t p_size, abort_callback& p_abort)
{
    if (p_size) {
        const auto version = p_reader->read_lendian_t<uint32_t>(p_abort);
        if (version <= config_version_current) {
            p_reader->read_lendian_t(m_show_clear_button, p_abort);
        }
    }
}
void FilterSearchToolbar::get_config(stream_writer* p_writer, abort_callback& p_abort) const
{
    p_writer->write_lendian_t(static_cast<uint32_t>(config_version_current), p_abort);
    p_writer->write_lendian_t(m_show_clear_button, p_abort);
}

void FilterSearchToolbar::get_menu_items(uie::menu_hook_t& p_hook)
{
    p_hook.add_node(new uie::simple_command_menu_node("Show clear button", "Shows or hides the clear button",
        m_show_clear_button ? uie::menu_node_t::state_checked : 0, [this, ref = ptr{this}] {
            m_show_clear_button = !m_show_clear_button;
            on_show_clear_button_change();
        }));
}

void FilterSearchToolbar::on_show_clear_button_change()
{
    TBBUTTONINFO tbbi{};
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_STATE;
    tbbi.fsState = TBSTATE_ENABLED | (m_show_clear_button ? NULL : TBSTATE_HIDDEN);
    SendMessage(m_wnd_toolbar, TB_SETBUTTONINFO, idc_clear, (LPARAM)&tbbi);

    recalculate_dimensions();

    if (get_host().is_valid())
        get_host()->on_size_limit_change(get_wnd(), uie::size_limit_all);
    on_size();
}

bool FilterSearchToolbar::g_activate()
{
    if (s_windows.empty())
        return false;

    s_windows[0]->activate();
    return true;
}

bool FilterSearchToolbar::g_filter_search_bar_has_stream(
    FilterSearchToolbar const* p_seach_bar, const FilterStream::ptr& p_stream)
{
    pfc::list_t<FilterStream::ptr> p_streams;
    g_get_search_bar_sibling_streams(p_seach_bar, p_streams);
    return p_streams.have_item(p_stream);
}

void FilterSearchToolbar::s_on_favourites_change()
{
    for (auto&& window : s_windows) {
        window->refresh_favourites(false);
    }
}

void FilterSearchToolbar::activate()
{
    m_wnd_last_focused = GetFocus();
    SetFocus(m_search_editbox);
}

void FilterSearchToolbar::on_size(int cx, int cy)
{
    // RECT rc_tbb = {0};
    // SendMessage(m_wnd_toolbar, TB_GETITEMRECT, 0, (LPARAM)(&rc_tbb));
    SetWindowPos(m_search_editbox, nullptr, 0, 0, cx - m_toolbar_cx, 200, SWP_NOZORDER);
    SetWindowPos(m_wnd_toolbar, nullptr, cx - m_toolbar_cx, 0, m_toolbar_cx, cy, SWP_NOZORDER);
}

void FilterSearchToolbar::on_search_editbox_change()
{
    if (m_query_timer_active)
        KillTimer(get_wnd(), TIMER_QUERY);
    SetTimer(get_wnd(), TIMER_QUERY, 500, nullptr);
    m_query_timer_active = true;
    update_favourite_icon();
}
void FilterSearchToolbar::commit_search_results(const char* str, bool b_force_autosend, bool b_stream_update)
{
    pfc::list_t<FilterStream::ptr> p_streams;
    g_get_search_bar_sibling_streams(this, p_streams);
    if (p_streams.get_count() == 0)
        p_streams = FilterPanel::g_streams;

    size_t stream_count = p_streams.get_count();
    bool b_diff = strcmp(m_active_search_string, str) != 0;
    if (!stream_count)
        b_force_autosend = b_diff;
    if (b_diff || b_force_autosend || b_stream_update) {
        m_active_search_string = str;

        bool b_reset = m_active_search_string.is_empty();

        if (b_reset) {
            m_active_handles.remove_all();
        } else if (b_diff) {
            library_manager::get()->get_all_items(m_active_handles);
            try {
                auto api = search_filter_manager_v2::get()->create_ex(m_active_search_string,
                    fb2k::service_new<completion_notify_dummy>(), search_filter_manager_v2::KFlagSuppressNotify);
                pfc::array_t<bool> data;
                data.set_size(m_active_handles.get_count());
                api->test_multi(m_active_handles, data.get_ptr());
                m_active_handles.remove_mask(
                    pfc::bit_array_not(pfc::bit_array_table(data.get_ptr(), data.get_count())));
            } catch (pfc::exception const&) {
            }
        }

        bool b_autosent = false;
        for (size_t i = 0; i < stream_count; i++) {
            FilterStream::ptr p_stream = p_streams[i];

            p_stream->m_source_overriden = !b_reset;
            p_stream->m_source_handles = m_active_handles;

            if (!b_stream_update) {
                size_t filter_count = p_stream->m_windows.get_count();
                if (filter_count) {
                    bool b_stream_visible = p_stream->is_visible(); // mask_visible.get(i);
                    pfc::list_t<FilterPanel*> ordered_windows;
                    p_stream->m_windows[0]->get_windows(ordered_windows);
                    if (ordered_windows.get_count()) {
                        if (b_diff)
                            ordered_windows[0]->refresh((b_stream_visible || stream_count == 1) && !b_autosent);
                        if (!b_autosent) {
                            if ((b_stream_visible || stream_count == 1) && b_force_autosend && !cfg_autosend)
                                ordered_windows[0]->send_results_to_playlist();
                            b_autosent = b_stream_visible || stream_count == 1;
                        }
                    }
                }
            }
        }
        if (!b_stream_update && (stream_count == 0 || !b_autosent) && (cfg_autosend || b_force_autosend))
            g_send_metadb_handles_to_playlist(m_active_handles);
    }
}

LRESULT FilterSearchToolbar::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE:
        s_windows.emplace_back(this);

        if (!s_font)
            s_recreate_font();

        if (!s_background_brush)
            s_recreate_background_brush();

        create_edit();

        break;
    case WM_NCDESTROY:
        std::erase(s_windows, this);

        if (!core_api::is_shutting_down())
            commit_search_results("");

        if (s_windows.empty()) {
            s_font.reset();
            s_background_brush.reset();
        }

        m_active_handles.remove_all();
        m_active_search_string.reset();
        if (m_imagelist) {
            ImageList_Destroy(m_imagelist);
            m_imagelist = nullptr;
        }
        break;
    case WM_TIMER:
        switch (wp) {
        case TIMER_QUERY:
            KillTimer(get_wnd(), TIMER_QUERY);
            if (m_query_timer_active)
                commit_search_results(uGetWindowText(m_search_editbox));
            m_query_timer_active = false;
            return 0;
        }
        break;
    case WM_SETFOCUS:
        break;
    case WM_KILLFOCUS:
        break;
    case msg_favourite_selected:
        if (m_query_timer_active) {
            KillTimer(get_wnd(), TIMER_QUERY);
            m_query_timer_active = false;
        }
        commit_search_results(uGetWindowText(m_search_editbox));
        update_favourite_icon();
        return 0;
    case WM_GETMINMAXINFO: {
        auto mmi = LPMINMAXINFO(lp);
        mmi->ptMinTrackSize.x = m_combo_cx + m_toolbar_cx;
        mmi->ptMinTrackSize.y = std::max(m_combo_cy, m_toolbar_cy);
        mmi->ptMaxTrackSize.y = mmi->ptMinTrackSize.y;
    }
        return 0;
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX: {
        const auto dc = reinterpret_cast<HDC>(wp);

        colours::helper colour_helper(colour_client_id);

        SetTextColor(dc, colour_helper.get_colour(colours::colour_text));
        SetBkColor(dc, colour_helper.get_colour(colours::colour_background));

        return reinterpret_cast<LRESULT>(s_background_brush.get());
    }
    case WM_NOTIFY: {
        auto lpnm = (LPNMHDR)lp;
        switch (lpnm->idFrom) {
        case id_toolbar:
            switch (lpnm->code) {
            case TBN_GETINFOTIP: {
                auto lpnmtbgit = (LPNMTBGETINFOTIP)lp;
                pfc::string8 temp;
                if (lpnmtbgit->iItem == idc_favourite) {
                    const auto query = uGetWindowText(m_search_editbox);
                    temp = !query.is_empty() && cfg_favourites.have_item(query) ? "Remove from favourites"
                                                                                : "Add to favourites";
                } else if (lpnmtbgit->iItem == idc_clear)
                    temp = "Clear";
                StringCchCopy(lpnmtbgit->pszText, lpnmtbgit->cchTextMax, pfc::stringcvt::string_wide_from_utf8(temp));
            }
                return 0;
            }
            break;
        }
    } break;
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case id_edit:
            switch (HIWORD(wp)) {
            case CBN_EDITCHANGE:
                on_search_editbox_change();
                break;
            case CBN_SELCHANGE:
                PostMessage(wnd, msg_favourite_selected, NULL, NULL);
                break;
            case CBN_SETFOCUS:
                PostMessage(m_search_editbox, CB_SETEDITSEL, 0, MAKELPARAM(0, -1));
                break;
            case CBN_KILLFOCUS:
                m_wnd_last_focused = nullptr;
                break;
            }
            break;
        case idc_clear:
            if (m_query_timer_active) {
                KillTimer(get_wnd(), TIMER_QUERY);
                m_query_timer_active = false;
            }
            ComboBox_SetText(m_search_editbox, L"");
            UpdateWindow(m_search_editbox);
            update_favourite_icon();
            commit_search_results("");
            break;
        case idc_favourite: {
            const auto query = uGetWindowText(m_search_editbox);
            size_t index;
            if (!query.is_empty()) {
                if (cfg_favourites.find_item(query, index)) {
                    cfg_favourites.remove_by_idx(index);
                    for (auto&& window : s_windows)
                        if (window->m_search_editbox)
                            ComboBox_DeleteString(window->m_search_editbox, index);
                } else {
                    cfg_favourites.add_item(query);
                    for (auto&& window : s_windows)
                        if (window->m_search_editbox) {
                            const auto wide_query = pfc::stringcvt::string_wide_from_utf8(query);
                            ComboBox_AddString(window->m_search_editbox, wide_query.get_ptr());
                        }
                }
                update_favourite_icon();
            }
        } break;
        }
        break;
    }
    return DefWindowProc(wnd, msg, wp, lp);
}

void FilterSearchToolbar::set_window_themes() const
{
    const auto is_dark = cui::colours::is_dark_mode_active();

    if (m_search_editbox)
        SetWindowTheme(m_search_editbox, is_dark ? L"DarkMode_CFD" : nullptr, nullptr);

    if (m_wnd_toolbar)
        SetWindowTheme(m_wnd_toolbar, is_dark ? L"DarkMode" : nullptr, nullptr);
}

void FilterSearchToolbar::update_toolbar_icons() const
{
    const auto is_dark = cui::colours::is_dark_mode_active();
    const int cx = GetSystemMetrics(SM_CXSMICON);
    const int cy = GetSystemMetrics(SM_CYSMICON);

    const WORD grey_star_resource_id = is_dark ? IDI_DARK_STAROFF : IDI_LIGHT_STAROFF;
    const WORD gold_star_resource_id = is_dark ? IDI_DARK_STARON : IDI_LIGHT_STARON;
    const WORD reset_resource_id = is_dark ? IDI_DARK_RESET : IDI_LIGHT_RESET;

    wil::unique_hicon grey_star(static_cast<HICON>(
        LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(grey_star_resource_id), IMAGE_ICON, cx, cy, NULL)));
    wil::unique_hicon gold_star(static_cast<HICON>(
        LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(gold_star_resource_id), IMAGE_ICON, cx, cy, NULL)));
    wil::unique_hicon reset(static_cast<HICON>(
        LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(reset_resource_id), IMAGE_ICON, cx, cy, NULL)));

    ImageList_ReplaceIcon(m_imagelist, 0, grey_star.get());
    ImageList_ReplaceIcon(m_imagelist, 1, gold_star.get());
    ImageList_ReplaceIcon(m_imagelist, 2, reset.get());
}

void FilterSearchToolbar::refresh_favourites(bool is_initial)
{
    if (!is_initial)
        ComboBox_ResetContent(m_search_editbox);

    for (size_t i{0}, count{cfg_favourites.get_count()}; i < count; i++) {
        pfc::stringcvt::string_os_from_utf8 favourite(cfg_favourites[i]);
        ComboBox_AddString(m_search_editbox, favourite);
    }

    if (!is_initial)
        update_favourite_icon();
}

void FilterSearchToolbar::update_favourite_icon(const char* p_new)
{
    bool new_state = cfg_favourites.have_item(p_new ? p_new : uGetWindowText(m_search_editbox).get_ptr());
    if (m_favourite_state != new_state) {
        TBBUTTONINFO tbbi{};
        tbbi.cbSize = sizeof(tbbi);
        tbbi.dwMask = TBIF_IMAGE;
        tbbi.iImage = new_state ? 1 : 0;
        // tbbi.pszText = (LPWSTR)(new_state ? L"Remove from favourites" : L"Add to favourites");
        SendMessage(m_wnd_toolbar, TB_SETBUTTONINFO, idc_favourite, (LPARAM)&tbbi);
        UpdateWindow(m_wnd_toolbar);
        m_favourite_state = new_state;
    }
}

void FilterSearchToolbar::create_edit()
{
    m_favourite_state = false;

    m_search_editbox
        = CreateWindowEx(WS_EX_CLIENTEDGE, WC_COMBOBOX, L"" /*pfc::stringcvt::string_os_from_utf8("").get_ptr()*/,
            WS_CHILD | WS_CLIPSIBLINGS | ES_LEFT | WS_VISIBLE | WS_CLIPCHILDREN | CBS_DROPDOWN | CBS_AUTOHSCROLL
                | WS_TABSTOP | WS_VSCROLL,
            0, 0, 100, 200, get_wnd(), HMENU(id_edit), core_api::get_my_instance(), nullptr);

    ComboBox_SetMinVisible(m_search_editbox, 25);

    m_wnd_toolbar = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, nullptr,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TRANSPARENT
            | TBSTYLE_TOOLTIPS | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER,
        0, 0, 0, 0, get_wnd(), (HMENU)id_toolbar, core_api::get_my_instance(), nullptr);

    set_window_themes();

    const unsigned cx = GetSystemMetrics(SM_CXSMICON);
    const unsigned cy = GetSystemMetrics(SM_CYSMICON);

    m_imagelist = ImageList_Create(cx, cy, ILC_COLOR32, 3, 0);
    ImageList_SetImageCount(m_imagelist, 3);
    update_toolbar_icons();

    TBBUTTON tbb[2]{};
    tbb[0].iBitmap = 2;
    tbb[0].idCommand = idc_clear;
    tbb[0].fsState = TBSTATE_ENABLED | (m_show_clear_button ? NULL : TBSTATE_HIDDEN);
    tbb[0].fsStyle = BTNS_AUTOSIZE | BTNS_BUTTON;
    // tbb[0].iString = (INT_PTR)(L"C");
    tbb[1].iBitmap = 0;
    tbb[1].idCommand = idc_favourite;
    tbb[1].fsState = TBSTATE_ENABLED;
    tbb[1].fsStyle = BTNS_AUTOSIZE | BTNS_BUTTON;
    tbb[1].iString = -1;
    // tb.iString = (INT_PTR)(L"Add to favourites");

    const auto ex_style = SendMessage(m_wnd_toolbar, TB_GETEXTENDEDSTYLE, 0, 0);
    SendMessage(m_wnd_toolbar, TB_SETEXTENDEDSTYLE, 0, ex_style | TBSTYLE_EX_DRAWDDARROWS | TBSTYLE_EX_MIXEDBUTTONS);
    SendMessage(m_wnd_toolbar, TB_SETBITMAPSIZE, (WPARAM)0, MAKELONG(cx, cy));
    SendMessage(m_wnd_toolbar, TB_GETPADDING, (WPARAM)0, 0);

    SendMessage(m_wnd_toolbar, TB_SETIMAGELIST, (WPARAM)0, (LPARAM)m_imagelist);
    SendMessage(m_wnd_toolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(m_wnd_toolbar, TB_ADDBUTTONS, (WPARAM)std::size(tbb), (LPARAM)&tbb[0]);
    ShowWindow(m_wnd_toolbar, SW_SHOWNORMAL);
    SendMessage(m_wnd_toolbar, TB_AUTOSIZE, 0, 0);

    SetWindowFont(m_search_editbox, s_font.get(), TRUE);

    COMBOBOXINFO cbi{};
    cbi.cbSize = sizeof(cbi);
    SendMessage(m_search_editbox, CB_GETCOMBOBOXINFO, NULL, (LPARAM)&cbi);

    recalculate_dimensions();

    SetWindowLongPtr(m_search_editbox, GWLP_USERDATA, (LPARAM)(this));
    SetWindowLongPtr(cbi.hwndItem, GWLP_USERDATA, (LPARAM)(this));
    m_proc_search_edit = (WNDPROC)SetWindowLongPtr(cbi.hwndItem, GWLP_WNDPROC, (LPARAM)(g_on_search_edit_message));
    Edit_SetCueBannerText(cbi.hwndItem, L"Search Filters");

    refresh_favourites(true);

    on_size();
}

void FilterSearchToolbar::recalculate_dimensions()
{
    COMBOBOXINFO cbi{};
    cbi.cbSize = sizeof(cbi);
    SendMessage(m_search_editbox, CB_GETCOMBOBOXINFO, NULL, (LPARAM)&cbi);

    RECT client_rect{};
    GetClientRect(m_search_editbox, &client_rect);
    m_combo_cx = RECT_CX(client_rect) - RECT_CX(cbi.rcItem);

    RECT window_rect{};
    GetWindowRect(m_search_editbox, &window_rect);
    m_combo_cy = window_rect.bottom - window_rect.top;

    SendMessage(m_wnd_toolbar, TB_GETITEMRECT, 1, (LPARAM)(&window_rect));

    m_toolbar_cx = window_rect.right;
    m_toolbar_cy = window_rect.bottom;
}

void FilterSearchToolbar::ColourClient::on_bool_changed(uint32_t mask) const
{
    if (mask & colours::bool_flag_dark_mode_enabled)
        FilterSearchToolbar::s_on_dark_mode_status_change();
}

LRESULT WINAPI FilterSearchToolbar::g_on_search_edit_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    auto p_this = reinterpret_cast<FilterSearchToolbar*>(GetWindowLongPtr(wnd, GWLP_USERDATA));
    return p_this ? p_this->on_search_edit_message(wnd, msg, wp, lp) : DefWindowProc(wnd, msg, wp, lp);
}

void FilterSearchToolbar::s_recreate_font()
{
    const fonts::helper font_helper(font_client_id);
    s_font.reset(font_helper.get_font());
}

void FilterSearchToolbar::s_recreate_background_brush()
{
    const colours::helper colour_helper(colour_client_id);
    s_background_brush.reset(CreateSolidBrush(colour_helper.get_colour(colours::colour_background)));
}

void FilterSearchToolbar::s_on_dark_mode_status_change()
{
    for (auto&& window : s_windows) {
        window->set_window_themes();
        window->update_toolbar_icons();
        RedrawWindow(window->get_wnd(), nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
    }
}

void FilterSearchToolbar::s_update_colours()
{
    s_recreate_background_brush();

    for (auto&& window : s_windows) {
        const HWND wnd = window->m_search_editbox;
        if (wnd)
            RedrawWindow(wnd, nullptr, nullptr, RDW_INVALIDATE);
    }
}

void FilterSearchToolbar::s_update_font()
{
    s_recreate_font();

    for (auto&& window : s_windows) {
        const HWND wnd = window->m_search_editbox;

        if (!wnd)
            continue;

        SetWindowFont(wnd, s_font.get(), TRUE);
        window->recalculate_dimensions();
        window->get_host()->on_size_limit_change(window->get_wnd(),
            uie::size_limit_minimum_height | uie::size_limit_maximum_height | uie::size_limit_minimum_width);
    }
}

LRESULT FilterSearchToolbar::on_search_edit_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_KILLFOCUS:
        // m_wnd_last_focused = NULL;
        break;
    case WM_SETFOCUS:
        // m_wnd_last_focused = (HWND)wp;
        break;
    case WM_GETDLGCODE:
        // return CallWindowProc(m_proc_search_edit,wnd,msg,wp,lp)|DLGC_WANTALLKEYS;
        break;
    case WM_KEYDOWN:
        switch (wp) {
        case VK_TAB: {
            g_on_tab(wnd);
        }
        // return 0;
        break;
        case VK_ESCAPE:
            if (m_wnd_last_focused && IsWindow(m_wnd_last_focused))
                SetFocus(m_wnd_last_focused);
            return 0;
        case VK_DELETE: {
#if 0
                if (ComboBox_GetDroppedState(m_search_editbox) == TRUE)
                {
                    int index = ComboBox_GetCurSel(m_search_editbox);
                    if (index != -1 && (size_t)index < cfg_favourites.get_count())
                    {
                        cfg_favourites.remove_by_idx(index);
                        for (size_t i = 0, count = g_active_instances.get_count(); i<count; i++)
                            if (g_active_instances[i]->m_search_editbox)
                                ComboBox_DeleteString(g_active_instances[i]->m_search_editbox, index);
                    }
                }
#endif
        } break;
        case VK_RETURN:
            if (m_query_timer_active) {
                KillTimer(get_wnd(), TIMER_QUERY);
                m_query_timer_active = false;
            }
            commit_search_results(uGetWindowText(m_search_editbox), true);
            return 0;
        }
        break;
    case WM_CHAR:
        switch (wp) {
        case VK_ESCAPE:
        case VK_RETURN:
            return 0;
        }
        break;
    }
    return CallWindowProc(m_proc_search_edit, wnd, msg, wp, lp);
}

} // namespace cui::panels::filter
