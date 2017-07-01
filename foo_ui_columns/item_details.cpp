#include "stdafx.h"
#include "item_details.h"
//#include <Richedit.h>

// {59B4F428-26A5-4a51-89E5-3945D327B4CB}
const GUID g_guid_item_details = 
{ 0x59b4f428, 0x26a5, 0x4a51, { 0x89, 0xe5, 0x39, 0x45, 0xd3, 0x27, 0xb4, 0xcb } };

// {A834BCF6-7230-4ff6-8F30-2ED826EEE1D3}
const GUID g_guid_item_details_tracking_mode = 
{ 0xa834bcf6, 0x7230, 0x4ff6, { 0x8f, 0x30, 0x2e, 0xd8, 0x26, 0xee, 0xe1, 0xd3 } };

// {16345DC1-2B8B-4351-A2B4-64736F667B22}
const GUID g_guid_item_details_script = 
{ 0x16345dc1, 0x2b8b, 0x4351, { 0xa2, 0xb4, 0x64, 0x73, 0x6f, 0x66, 0x7b, 0x22 } };

// {77F3FA70-E39C-46f8-8E8A-6ECC64DDE234}
const GUID g_guid_item_details_font_client = 
{ 0x77f3fa70, 0xe39c, 0x46f8, { 0x8e, 0x8a, 0x6e, 0xcc, 0x64, 0xdd, 0xe2, 0x34 } };

// {4E20CEED-42F6-4743-8EB3-610454457E19}
const GUID g_guid_item_details_colour_client = 
{ 0x4e20ceed, 0x42f6, 0x4743, { 0x8e, 0xb3, 0x61, 0x4, 0x54, 0x45, 0x7e, 0x19 } };

// {3E3D189A-8154-4c9f-8E68-B278301271CD}
const GUID g_guid_item_details_hscroll = 
{ 0x3e3d189a, 0x8154, 0x4c9f, { 0x8e, 0x68, 0xb2, 0x78, 0x30, 0x12, 0x71, 0xcd } };

// {AE00D056-AACB-4ca0-A2EC-FD2BAB599C1B}
const GUID g_guid_item_details_horizontal_alignment = 
{ 0xae00d056, 0xaacb, 0x4ca0, { 0xa2, 0xec, 0xfd, 0x2b, 0xab, 0x59, 0x9c, 0x1b } };

// {07526EBA-2E7A-4e03-83ED-7BDE8FF79E8E}
const GUID g_guid_item_details_vertical_alignment = 
{ 0x7526eba, 0x2e7a, 0x4e03, { 0x83, 0xed, 0x7b, 0xde, 0x8f, 0xf7, 0x9e, 0x8e } };

// {41753F1F-F2D6-4385-BEFA-B4BEC44A4167}
const GUID g_guid_item_details_word_wrapping = 
{ 0x41753f1f, 0xf2d6, 0x4385, { 0xbe, 0xfa, 0xb4, 0xbe, 0xc4, 0x4a, 0x41, 0x67 } };

// {E944E1BF-0822-4141-B417-1F259D738ABC}
const GUID g_guid_item_details_edge_style = 
{ 0xe944e1bf, 0x822, 0x4141, { 0xb4, 0x17, 0x1f, 0x25, 0x9d, 0x73, 0x8a, 0xbc } };


cfg_uint cfg_item_details_tracking_mode(g_guid_item_details_tracking_mode, 0);
cfg_uint cfg_item_details_edge_style(g_guid_item_details_edge_style, 2);
#if 0
cfg_string cfg_item_details_script(g_guid_item_details_script, "[%artist%]$crlf()[%title%]$crlf()[%album%][$crlf()$crlf()%lyrics%]");
#else
pfc::string8 cfg_item_details_script = "$set_font(%default_font_face%,$add(%default_font_size%,4),)[%artist%]$crlf()[%title%]$crlf()[%album%][$crlf()$crlf()%lyrics%]";
#endif
cfg_bool cfg_item_details_hscroll(g_guid_item_details_hscroll, false);
cfg_uint cfg_item_details_horizontal_alignment(g_guid_item_details_horizontal_alignment, ui_helpers::ALIGN_CENTRE);
cfg_uint cfg_item_details_vertical_alignment(g_guid_item_details_vertical_alignment, ui_helpers::ALIGN_CENTRE);
cfg_bool cfg_item_details_word_wrapping(g_guid_item_details_word_wrapping, true);




void item_details_t::menu_node_options::execute()
{
    if (p_this->m_wnd_config)
        SetActiveWindow (p_this->m_wnd_config);
    else
    {
        auto  p_dialog = new item_details_config_t(p_this->m_script, p_this->m_edge_style, p_this->m_horizontal_alignment, p_this->m_vertical_alignment);
        p_dialog->run_modeless(GetAncestor(p_this->get_wnd(), GA_ROOT), p_this.get_ptr());
    }
}

item_details_t::menu_node_options::menu_node_options(item_details_t * p_wnd) : p_this(p_wnd)
{

}

bool item_details_t::menu_node_options::get_description(pfc::string_base & p_out) const
{
    return false;
}

bool item_details_t::menu_node_options::get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const
{
    p_out = "Options";
    p_displayflags = 0;
    return true;
}

bool item_details_t::have_config_popup()const {return true;}
bool item_details_t::show_config_popup(HWND wnd_parent) 
{
    item_details_config_t dialog(m_script, m_edge_style, m_horizontal_alignment, m_vertical_alignment);
    if (dialog.run_modal(wnd_parent))
    {
        m_script = dialog.m_script;
        m_edge_style = dialog.m_edge_style;
        cfg_item_details_edge_style = m_edge_style;
        m_horizontal_alignment = dialog.m_horizontal_alignment;
        m_vertical_alignment = dialog.m_vertical_alignment;
        cfg_item_details_horizontal_alignment = m_horizontal_alignment;
        cfg_item_details_vertical_alignment = m_vertical_alignment;

        if (get_wnd())
        {
            m_to.release();
            static_api_ptr_t<titleformat_compiler>()->compile_safe(m_to, m_script);

            on_edge_style_change();
            refresh_contents();
        }
        return true;
    }
    return false;
}

void item_details_t::set_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort)
{
    if (p_size)
    {
        t_size version;
        p_reader->read_lendian_t(version, p_abort);
        if (version <= stream_version_current)
        {
            p_reader->read_string(m_script, p_abort);
            p_reader->read_lendian_t(m_tracking_mode, p_abort);
            p_reader->read_lendian_t(m_hscroll, p_abort);
            p_reader->read_lendian_t(m_horizontal_alignment, p_abort);
            if (version >= 1)
            {
                p_reader->read_lendian_t(m_word_wrapping, p_abort);
                if (version >= 2)
                {
                    p_reader->read_lendian_t(m_edge_style, p_abort);
                    p_reader->read_lendian_t(m_vertical_alignment, p_abort);
                }
            }
        }
    }
}

void item_details_t::get_config(stream_writer * p_writer, abort_callback & p_abort) const
{
    p_writer->write_lendian_t((t_size)stream_version_current, p_abort);
    p_writer->write_string(m_script, p_abort);
    p_writer->write_lendian_t(m_tracking_mode, p_abort);
    p_writer->write_lendian_t(m_hscroll, p_abort);
    p_writer->write_lendian_t(m_horizontal_alignment, p_abort);
    p_writer->write_lendian_t(m_word_wrapping, p_abort);
    p_writer->write_lendian_t(m_edge_style, p_abort);
    p_writer->write_lendian_t(m_vertical_alignment, p_abort);
}

void item_details_t::get_menu_items (ui_extension::menu_hook_t & p_hook)
{
    ui_extension::menu_node_ptr p_node;
    p_hook.add_node(ui_extension::menu_node_ptr(new menu_node_source_popup(this)));
    //p_node = new menu_node_alignment_popup(this);
    //p_hook.add_node(p_node);
    p_node = new menu_node_hscroll(this);
    p_hook.add_node(p_node);
    p_node = new menu_node_wwrap(this);
    p_hook.add_node(p_node);
    p_node = new menu_node_options(this);
    p_hook.add_node(p_node);
}

item_details_t::message_window_t item_details_t::g_message_window;

std::vector<item_details_t*> item_details_t::g_windows;

item_details_t::message_window_t::class_data & item_details_t::message_window_t::get_class_data() const 
{
    __implement_get_class_data_ex(_T("\r\n{6EB3EA81-7C5E-468d-B507-E5527F52361B}"), _T(""), false, 0, 0, 0, 0);
}

LRESULT item_details_t::message_window_t::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
    switch(msg)
    {
    case WM_CREATE:
        break;
    case WM_ACTIVATEAPP:
        item_details_t::g_on_app_activate(wp != 0);
        break;
    case WM_DESTROY:
        break;

    }
    return DefWindowProc(wnd, msg, wp, lp);
}

void item_details_t::g_on_app_activate(bool b_activated)
{
    for (auto & window : g_windows)
        window->on_app_activate(b_activated);
}
void item_details_t::on_app_activate(bool b_activated)
{
    if (b_activated)
    {
        if (GetFocus() != get_wnd())
            register_callback();
    }
    else
    {
        deregister_callback();
    }
}

const GUID & item_details_t::get_extension_guid() const {return g_guid_item_details;}
void item_details_t::get_name(pfc::string_base & p_out)const {p_out = "Item details";}
void item_details_t::get_category(pfc::string_base & p_out)const {p_out = "Panels";}
unsigned item_details_t::get_type () const {return uie::type_panel;}

void item_details_t::register_callback()
{
    if (!m_callback_registered)
        g_ui_selection_manager_register_callback_no_now_playing_fallback(this);
    m_callback_registered = true;
}
void item_details_t::deregister_callback()
{
    if (m_callback_registered)
        static_api_ptr_t<ui_selection_manager>()->unregister_callback(this);
    m_callback_registered = false;
}

void item_details_t::update_scrollbar_range(bool b_set_pos)
{
    //if (m_update_scrollbar_range_in_progress) return;

    //pfc::vartoggle_t<bool> vart(m_update_scrollbar_range_in_progress, true);
    SCROLLINFO si, si2;
    memset(&si, 0, sizeof(si));
    si.cbSize = sizeof(si);
    si2 = si;

    RECT rc;
    GetClientRect(get_wnd(), &rc);

    RECT rc_old = rc;

    //SIZE & sz = m_display_sz;
    {
        update_font_change_info();

        update_display_info();
    }

    int vMax = 1;
    
    vMax = m_display_sz.cy ? m_display_sz.cy-1 : 0;
    vMax = max (vMax, 1);

    si.fMask = SIF_RANGE|SIF_PAGE;
    si.nPage = RECT_CY(rc);
    si.nPage = max (si.nPage, 1);

    si.nMin = 0;
    si.nMax = vMax;
    SetScrollInfo(get_wnd(), SB_VERT, &si, TRUE);

#if 1
    GetClientRect(get_wnd(), &rc);
    update_display_info();

    vMax = m_display_sz.cy ? m_display_sz.cy-1 : 0;
    vMax = max (vMax, 1);
#else
    GetClientRect(get_wnd(), &rc);
    if (!EqualRect(&rc_old, &rc))
    {
        HDC dc = GetDC(get_wnd());
        HFONT fnt_old = SelectFont(dc, m_font_change_info.m_default_font->m_font.get());
        g_get_multiline_text_dimensions_const(dc, m_current_text, m_line_info, m_font_change_info, uGetTextHeight(dc)+2, sz, m_word_wrapping, RECT_CX(rc)>4 ? RECT_CX(rc)-4:0);
        SelectFont(dc, fnt_old);
        ReleaseDC(get_wnd(), dc);

        rc_old = rc;

        vMax = sz.cy ? sz.cy-1 : 0;
        vMax = max (vMax, 1);
    }
#endif

    si2.fMask = SIF_PAGE;
    GetScrollInfo(get_wnd(), SB_VERT, &si2);
    //bool b_has_vscrollbar = (GetWindowLongPtr(get_wnd(), GWL_STYLE) & WS_VSCROLL) != 0;
    bool b_need_vscrollbar = vMax >= (int)si2.nPage;
    //if (b_need_vscrollbar != b_has_vscrollbar)
        ShowScrollBar(get_wnd(), SB_VERT, b_need_vscrollbar);

    GetClientRect(get_wnd(), &rc);
    update_display_info();

    /*GetClientRect(get_wnd(), &rc);
    if (!EqualRect(&rc_old, &rc))
    {
        HDC dc = GetDC(get_wnd());
        HFONT fnt_old = SelectFont(dc, m_font_change_info.m_default_font->m_font.get());
        get_multiline_text_dimensions_const(dc, m_current_text, m_line_info, m_font_change_info, uGetTextHeight(dc)+2, sz, m_word_wrapping, RECT_CX(rc)>4 ? RECT_CX(rc)-4:0);
        SelectFont(dc, fnt_old);
        ReleaseDC(get_wnd(), dc);

        rc_old = rc;
    }*/
    int hMax = 1;
    hMax = (m_hscroll && m_display_sz.cx) ? (m_display_sz.cx + 4 - 1) : 0;
    hMax = max (hMax, 1);

    if (b_set_pos)
        si.fMask |= SIF_POS;
    si.nPage = RECT_CX(rc);
    si.nPage = max (si.nPage, 1);
    si.nMin = 0;
    si.nMax = hMax;

    if (b_set_pos)
    {
        if (m_horizontal_alignment == ui_helpers::ALIGN_RIGHT)
            si.nPos = si.nMax - (si.nPage ? si.nPage - 1 : 0);
        else if (m_horizontal_alignment == ui_helpers::ALIGN_CENTRE)
            si.nPos = (si.nMax - (si.nPage ? si.nPage - 1 : 0))/2;
        else
            si.fMask &= ~SIF_POS;
    }
    
    
    SetScrollInfo(get_wnd(), SB_HORZ, &si, TRUE);

#if 1

    GetClientRect(get_wnd(), &rc);
    update_display_info();

    hMax = (m_hscroll && m_display_sz.cx) ? (m_display_sz.cx + 4 - 1) : 0;
    hMax = max (hMax, 1);

    /*GetClientRect(get_wnd(), &rc);
    if (!EqualRect(&rc_old, &rc))
    {
        HDC dc = GetDC(get_wnd());
        HFONT fnt_old = SelectFont(dc, m_font_change_info.m_default_font->m_font.get());
        get_multiline_text_dimensions_const(dc, m_current_text, m_line_info, m_font_change_info, uGetTextHeight(dc)+2, sz, m_word_wrapping, RECT_CX(rc)>4 ? RECT_CX(rc)-4:0);
        SelectFont(dc, fnt_old);
        ReleaseDC(get_wnd(), dc);

        rc_old = rc;

        hMax = (m_hscroll && sz.cx) ? (sz.cx + 4 - 1) : 0;
        hMax = max (hMax, 1);
    }*/


    GetScrollInfo(get_wnd(), SB_HORZ, &si2);
    //bool b_has_hscrollbar = (GetWindowLongPtr(get_wnd(), GWL_STYLE) & WS_VSCROLL) != 0;
    bool b_need_hscrollbar = hMax >= (int)si2.nPage;
    //if (b_need_hscrollbar != b_has_hscrollbar)
        ShowScrollBar(get_wnd(), SB_HORZ, b_need_hscrollbar);
#endif
}

void item_details_t::refresh_contents(bool b_new_track)
{
    //DisableRedrawing noRedraw(get_wnd());
    bool b_Update = true;
    if (m_handles.get_count())
    {
        LOGFONT lf;
        static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_item_details_font_client, lf);

        titleformat_hook_change_font tf_hook(lf);
        pfc::string8_fast_aggressive temp, temp2;
        temp.prealloc(2048);
        temp2.prealloc(2048);
        if (m_nowplaying_active)
            static_api_ptr_t<playback_control>()->playback_format_title(&tf_hook, temp, m_to, nullptr, playback_control::display_level_all);
        else
            m_handles[0]->format_title(&tf_hook, temp, m_to, nullptr);

        if (strcmp(temp, m_current_text_raw))
        {
            m_current_text_raw = temp;

            m_font_change_data.set_size(0);
            m_font_change_info.reset(true);
            m_font_change_info_valid = false;

            g_get_text_multiline_data(temp, temp2, m_line_info);
            temp.reset();

            g_get_text_font_data(temp2, temp, m_font_change_data);
            m_current_text = temp;
        }
        else b_Update = false;
    }
    else 
    {
        m_current_text.reset();
        m_current_text_raw.reset();
        reset_font_change_info();
        m_line_info.remove_all();
    }

    if (b_Update)
    {
        reset_display_info();

        update_scrollbar_range(b_new_track);

        invalidate_all();
    }
}

void item_details_t::update_display_info(HDC dc)
{
    if (!m_display_info_valid)
    {
        RECT rc;
        GetClientRect(get_wnd(), &rc);
        t_size widthMax = rc.right>4 ? rc.right-4 : 0;
        m_current_display_text = m_current_text;
        g_get_multiline_text_dimensions(dc, m_current_display_text, m_line_info, m_display_line_info, m_font_change_info, m_font_change_info.m_default_font->m_height, m_display_sz, m_word_wrapping, widthMax);
        m_display_info_valid = true;
    }
}

void item_details_t::update_display_info()
{
    if (!m_display_info_valid)
    {
        HDC dc = GetDC(get_wnd());
        HFONT fnt_old = SelectFont(dc, m_font_change_info.m_default_font->m_font.get());
        update_display_info(dc);
        SelectFont(dc, fnt_old);
        ReleaseDC(get_wnd(), dc);
    }
}
void item_details_t::reset_display_info()
{
    m_current_display_text.force_reset();
    m_display_line_info.remove_all();
    m_display_sz.cy = (m_display_sz.cx = 0);
    m_display_info_valid = false;
}

void item_details_t::update_font_change_info()
{
    if (!m_font_change_info_valid)
    {
        g_get_text_font_info(m_font_change_data, m_font_change_info);
        m_font_change_data.set_size(0);
        m_font_change_info_valid = true;
    }
}

void item_details_t::reset_font_change_info()
{
    m_font_change_data.remove_all();
    m_font_change_info.reset();
    m_font_change_info_valid = false;
}

void item_details_t::on_playback_new_track(metadb_handle_ptr p_track)
{
    if ( g_track_mode_includes_now_playing(m_tracking_mode))
    {
        m_handles.remove_all();
        m_handles.add_item(p_track);
        m_nowplaying_active = true;
        refresh_contents();
    }
}

void item_details_t::on_playback_seek(double p_time) 
{
    if (m_nowplaying_active) refresh_contents(false);
}
void item_details_t::on_playback_pause(bool p_state)
{
    if (m_nowplaying_active) refresh_contents(false);
}
void item_details_t::on_playback_edited(metadb_handle_ptr p_track)
{
    if (m_nowplaying_active) refresh_contents(false);
}
void item_details_t::on_playback_dynamic_info(const file_info & p_info)
{
    if (m_nowplaying_active) refresh_contents(false);
}
void item_details_t::on_playback_dynamic_info_track(const file_info & p_info)
{
    if (m_nowplaying_active) refresh_contents(false);
}
void item_details_t::on_playback_time(double p_time)
{
    if (m_nowplaying_active) refresh_contents(false);
}


void item_details_t::on_playback_stop(play_control::t_stop_reason p_reason)
{
        if ( g_track_mode_includes_now_playing(m_tracking_mode) && p_reason != play_control::stop_reason_starting_another && p_reason != play_control::stop_reason_shutting_down)
        {
            m_nowplaying_active = false;

            metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
            if (m_tracking_mode == track_auto_playlist_playing)
            {
                static_api_ptr_t<playlist_manager_v3>()->activeplaylist_get_selected_items(handles);
            }
            else if (m_tracking_mode == track_auto_selection_playing)
            {
                handles = m_selection_handles;
            }
            m_handles = handles;
            refresh_contents();
        }
    
}

void item_details_t::on_playlist_switch()
{
    if ( g_track_mode_includes_plalist(m_tracking_mode) && (!g_track_mode_includes_auto(m_tracking_mode) || !static_api_ptr_t<play_control>()->is_playing()))
    {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        static_api_ptr_t<playlist_manager_v3>()->activeplaylist_get_selected_items(handles);
        m_handles = handles;
        refresh_contents();
    }
}
void item_details_t::on_items_selection_change(const bit_array & p_affected,const bit_array & p_state)
{
    if ( g_track_mode_includes_plalist(m_tracking_mode) && (!g_track_mode_includes_auto(m_tracking_mode) || !static_api_ptr_t<play_control>()->is_playing()))
    {
        metadb_handle_list_t<pfc::alloc_fast_aggressive> handles;
        static_api_ptr_t<playlist_manager_v3>()->activeplaylist_get_selected_items(handles);
        m_handles = handles;
        refresh_contents();
    }
}

void item_details_t::on_changed_sorted(metadb_handle_list_cref p_items_sorted, bool p_fromhook)
{
    if (!p_fromhook && !m_nowplaying_active)
    {
        bool b_refresh = false;
        t_size i, count = m_handles.get_count();
        for (i=0; i<count && !b_refresh; i++)
        {
            t_size index = pfc_infinite;
            if (p_items_sorted.bsearch_t(pfc::compare_t<metadb_handle_ptr, metadb_handle_ptr>, m_handles[i], index))
                b_refresh = true;
        }
        if (b_refresh)
        {
            refresh_contents(false);
        }
    }
}

bool item_details_t::check_process_on_selection_changed()
{
    HWND wnd_focus = GetFocus();
    if (wnd_focus == nullptr)
        return false ;

    DWORD processid = NULL;
    GetWindowThreadProcessId (wnd_focus, &processid);
    if (processid != GetCurrentProcessId())
        return false;

    return true;
}

void item_details_t::on_selection_changed(const pfc::list_base_const_t<metadb_handle_ptr> & p_selection)
{

    if (check_process_on_selection_changed())
    {
            if (g_ui_selection_manager_is_now_playing_fallback())
                m_selection_handles.remove_all();
            else
                m_selection_handles = p_selection;

            if ( g_track_mode_includes_selection(m_tracking_mode) && (!g_track_mode_includes_auto(m_tracking_mode) || !static_api_ptr_t<play_control>()->is_playing()))
            {
                m_handles = m_selection_handles;
                refresh_contents();
            }

    }

    //pfc::hires_timer timer;
    //timer.start();


    //console::formatter() << "Selection properties panel refreshed in: " << timer.query() << " seconds";
}

void item_details_t::on_tracking_mode_change()
{
    m_handles.remove_all();

    m_nowplaying_active = false;

    if (g_track_mode_includes_now_playing(m_tracking_mode) && static_api_ptr_t<play_control>()->is_playing())
    {
        metadb_handle_ptr item;
        if (static_api_ptr_t
            <playback_control>()->get_now_playing(item))
            m_handles.add_item(item);
        m_nowplaying_active = true;
    }
    else if ( g_track_mode_includes_plalist(m_tracking_mode) )
    {
        static_api_ptr_t<playlist_manager_v3>()->activeplaylist_get_selected_items(m_handles);
    }
    else if (g_track_mode_includes_selection(m_tracking_mode))
    {
        m_handles = m_selection_handles;
    }

    refresh_contents();
}

void item_details_t::update_now()
{
    RedrawWindow(get_wnd(), nullptr, nullptr, RDW_UPDATENOW);
}

void item_details_t::invalidate_all(bool b_update)
{
    RedrawWindow(get_wnd(), nullptr, nullptr, RDW_INVALIDATE|(b_update?RDW_UPDATENOW:NULL));
}

void item_details_t::on_size ()
{
    RECT rc;
    GetClientRect(get_wnd(), &rc);
    on_size (RECT_CX(rc), RECT_CY(rc));
}

void item_details_t::on_size (t_size cx, t_size cy)
{
    reset_display_info();

    invalidate_all(false);

    if (m_word_wrapping)
    {
        update_scrollbar_range();
    }
    else
    {
        SCROLLINFO si;
        memset(&si, 0, sizeof(si));
        si.cbSize = sizeof(si);

        si.fMask = SIF_PAGE;
        si.nPage = max (cy, 1);
        SetScrollInfo(get_wnd(), SB_VERT, &si, TRUE);

        RECT rc;
        GetClientRect(get_wnd(), &rc); //SetScrollInfo above may trigger a WM_SIZE
        si.nPage = RECT_CX(rc);
        si.nPage = max (si.nPage, 1);
        SetScrollInfo(get_wnd(), SB_HORZ, &si, TRUE);
    }
}

void item_details_t::scroll(INT SB, int position, bool b_absolute)
{
    SCROLLINFO si, si2;
    memset(&si, 0, sizeof(SCROLLINFO));
    si.cbSize = sizeof(si);
    si.fMask = SIF_POS|SIF_TRACKPOS|SIF_PAGE|SIF_RANGE;
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

    if (new_pos != si.nPos)
    {
        memset(&si2, 0, sizeof(SCROLLINFO));
        si2.cbSize = sizeof(si);
        si2.fMask = SIF_POS;
        si2.nPos = new_pos;
        new_pos = SetScrollInfo(get_wnd(), SB, &si2, TRUE);

        RECT rc;
        GetClientRect(get_wnd(), &rc);
        int dy = SB == SB_VERT ? si.nPos - new_pos : 0;
        int dx = SB == SB_HORZ ? si.nPos - new_pos : 0;
        ScrollWindowEx(get_wnd(), dx, dy, &rc, &rc, nullptr, nullptr, SW_INVALIDATE);
        RedrawWindow(get_wnd(),nullptr,nullptr,RDW_UPDATENOW);
    }

}

LRESULT item_details_t::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
    switch (msg)
    {
    case WM_CREATE:
        {
            register_callback();
            static_api_ptr_t<play_callback_manager>()->register_callback(this, play_callback::flag_on_playback_all & ~(play_callback::flag_on_volume_change|play_callback::flag_on_playback_starting), false);
            static_api_ptr_t<playlist_manager_v3>()->register_callback(this, playlist_callback_flags);
            static_api_ptr_t<metadb_io_v3>()->register_callback(this);

            m_font_change_info.m_default_font = new font_t;
            m_font_change_info.m_default_font->m_font = static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_item_details_font_client);
            m_font_change_info.m_default_font->m_height = uGetFontHeight (m_font_change_info.m_default_font->m_font);

            if (0 == g_windows.size())
                g_message_window.create(nullptr);
            g_windows.push_back(this);

            LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lp;

            static_api_ptr_t<titleformat_compiler>()->compile_safe(m_to, m_script);

            on_size (/*lpcs->cx, lpcs->cy*/);
            on_tracking_mode_change();
            refresh_contents();

            //FIXME
            //m_library_richedit = LoadLibrary(L"Msftedit.dll");
            //m_wnd_richedit = CreateWindowEx(NULL, MSFTEDIT_CLASS, L"MooMooCoCoBananas", WS_VISIBLE| WS_CHILD | WS_BORDER, 0, 0, 200, 200, wnd, HMENU(1001), core_api::get_my_instance(), NULL);
        }
        break;
    case WM_DESTROY:
        {
            g_windows.erase(std::remove(g_windows.begin(), g_windows.end(), this), g_windows.end());
            if (g_windows.size() == 0)
                g_message_window.destroy();

            m_font_change_info.m_default_font.release();

            static_api_ptr_t<play_callback_manager>()->unregister_callback(this);
            static_api_ptr_t<metadb_io_v3>()->unregister_callback(this);
            static_api_ptr_t<playlist_manager_v3>()->unregister_callback(this);
            deregister_callback();
            m_handles.remove_all();
            m_selection_handles.remove_all();
            m_selection_holder.release();
            m_to.release();
        }
        break;
    case WM_SETFOCUS:
        deregister_callback();
        m_selection_holder = static_api_ptr_t<ui_selection_manager>()->acquire();
        m_selection_holder->set_selection(m_handles);
        break;
    case WM_KILLFOCUS:
        m_selection_holder.release();
        register_callback();
        break;
    case WM_WINDOWPOSCHANGED:
        {
            LPWINDOWPOS lpwp = (LPWINDOWPOS)lp;
            if (!(lpwp->flags & SWP_NOSIZE) || (lpwp->flags & SWP_FRAMECHANGED))
            {
                on_size(lpwp->cx, lpwp->cy);
            }
        }
        break;
    case WM_MOUSEWHEEL:
        {
            LONG_PTR style = GetWindowLongPtr(get_wnd(),GWL_STYLE);
            bool b_horz = (!(style & WS_VSCROLL) || ((wp & MK_CONTROL))) && (style & WS_HSCROLL);

            SCROLLINFO si;
            memset(&si, 0, sizeof(SCROLLINFO));
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_POS|SIF_TRACKPOS|SIF_PAGE|SIF_RANGE;
            GetScrollInfo(get_wnd(), b_horz ? SB_HORZ : SB_VERT, &si);

            UINT scroll_lines = 3; //3 is default
            SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &scroll_lines, 0); //we don't support Win9X

            int line_height = m_font_change_info.m_default_font->m_height + 2;

            if (!si.nPage) si.nPage++;

            if (scroll_lines == -1)
                scroll_lines = si.nPage-1;
            else
                scroll_lines *= line_height;

            int zDelta = short(HIWORD(wp));

            if (scroll_lines == 0)
                scroll_lines = 1;

            //console::formatter() << zDelta;

            int delta = -MulDiv(zDelta, scroll_lines, 120);

            // Limit scrolling to one page ?!?!?! It was in Columns Playlist code...
            if (delta < 0 && (UINT)(delta*-1) > si.nPage)
            {
                delta = si.nPage*-1;
                if (delta <-1) delta++;
            }
            else if (delta > 0 && (UINT)delta > si.nPage)
            {
                delta = si.nPage;
                if (delta >1) delta--;
            }

            scroll(b_horz ? SB_HORZ : SB_VERT, delta, true);

        }
        return 0;
    case WM_HSCROLL:
    case WM_VSCROLL:
        {
            UINT SB = msg == WM_VSCROLL ? SB_VERT : SB_HORZ;
            SCROLLINFO si, si2;
            memset(&si, 0, sizeof(SCROLLINFO));
            si.cbSize = sizeof(si);
            si.fMask = SIF_POS|SIF_TRACKPOS|SIF_PAGE|SIF_RANGE;
            GetScrollInfo(get_wnd(), SB, &si);

            int new_pos = si.nPos;

            int line_height = m_font_change_info.m_default_font->m_height + 2;


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

            if (new_pos != si.nPos)
            {
                memset(&si2, 0, sizeof(SCROLLINFO));
                si2.cbSize = sizeof(si);
                si2.fMask = SIF_POS;
                si2.nPos = new_pos;
                new_pos = SetScrollInfo(wnd, SB, &si2, TRUE);

                RECT rc;
                GetClientRect(get_wnd(), &rc);
                int dy = SB == SB_VERT ? si.nPos - new_pos : 0;
                int dx = SB == SB_HORZ ? si.nPos - new_pos : 0;
                ScrollWindowEx(get_wnd(), dx, dy, &rc, &rc, nullptr, nullptr, SW_INVALIDATE);
                RedrawWindow(get_wnd(),nullptr,nullptr,RDW_UPDATENOW);
            }

        }
        return 0;
    case WM_ERASEBKGND:
        return TRUE;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(wnd, &ps);

            SCROLLINFO siv, sih;
            memset(&siv, 0, sizeof(siv));
            siv.cbSize = sizeof(siv);
            siv.fMask = SIF_POS;
            sih = siv;
            GetScrollInfo(wnd, SB_VERT, &siv);
            GetScrollInfo(wnd, SB_HORZ, &sih);

            HDC dc_mem = CreateCompatibleDC(ps.hdc);

            RECT & rc = ps.rcPaint;
            HBITMAP bm_mem = CreateCompatibleBitmap(ps.hdc, RECT_CX(rc), RECT_CY(rc));
            HBITMAP bm_old = SelectBitmap(dc_mem, bm_mem);

            OffsetWindowOrgEx(dc_mem, +rc.left, +rc.top, nullptr);

            cui::colours::helper p_helper(g_guid_item_details_colour_client);

            HFONT fnt_old = SelectFont(dc_mem, m_font_change_info.m_default_font->m_font.get());
            RECT rc_client;
            GetClientRect(wnd, &rc_client);

            const int client_height = RECT_CY(rc_client);
            
            FillRect(dc_mem, &rc, gdi_object_t<HBRUSH>::ptr_t(
                CreateSolidBrush(p_helper.get_colour(cui::colours::colour_background))
                ));

            int line_height = uGetTextHeight(dc_mem)+2;

            rc_client.top -= siv.nPos;
            rc_client.left -= sih.nPos;

            update_font_change_info();
            update_display_info(dc_mem);

            if (m_display_sz.cy < client_height)
            {
                int extra = client_height - m_display_sz.cy;
                if (m_vertical_alignment == ui_helpers::ALIGN_CENTRE)
                    rc_client.top += extra/2;
                else if (m_vertical_alignment == ui_helpers::ALIGN_RIGHT)
                    rc_client.top += extra;
            }

            //ui_helpers::text_out_colours_tab(dc_mem, m_current_text, m_current_text.get_length(), 0, 2, &rc, NULL, p_helper.get_colour(cui::colours::colour_text), true, true, false, ui_helpers::ALIGN_LEFT);
            //text_out_multiline(dc_mem, rc_client, line_height, m_current_text, p_helper.get_colour(cui::colours::colour_text), (ui_helpers::alignment)m_alignment, m_hscroll, m_word_wrapping);
            g_text_out_multiline_font(dc_mem, rc_client, line_height, m_current_text, m_font_change_info, m_display_line_info, m_display_sz, p_helper.get_colour(cui::colours::colour_text), (ui_helpers::alignment)m_horizontal_alignment, m_hscroll, m_word_wrapping);
            SelectFont(dc_mem, fnt_old);

            BitBlt(ps.hdc,    rc.left, rc.top, RECT_CX(rc), RECT_CY(rc), dc_mem, rc.left, rc.top, SRCCOPY);

            SelectBitmap(dc_mem, bm_old);
            DeleteObject(bm_mem);
            DeleteDC(dc_mem);

            EndPaint(wnd, &ps);
        }
        return 0;
    };
    return DefWindowProc(wnd, msg, wp, lp);
}


void item_details_t::g_on_font_change()
{
    for (auto & window : g_windows)
    {
        window->on_font_change();
    }
}

void item_details_t::g_on_colours_change()
{
    for (auto & window : g_windows)
    {
        window->on_colours_change();
    }
}

void item_details_t::on_font_change()
{
    m_font_change_info.m_default_font->m_font = static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_item_details_font_client);
    refresh_contents(false);
    /*
    invalidate_all(false);
    update_scrollbar_range();
    update_now();
    */
}
void item_details_t::on_colours_change()
{
    invalidate_all();
}

item_details_t::item_details_t() : m_callback_registered(false), m_nowplaying_active(false),
m_tracking_mode(cfg_item_details_tracking_mode), m_font_change_info_valid(false), m_display_info_valid(false),
m_script(cfg_item_details_script), m_horizontal_alignment(cfg_item_details_horizontal_alignment), m_vertical_alignment(cfg_item_details_vertical_alignment),
m_edge_style(cfg_item_details_edge_style), m_hscroll(cfg_item_details_hscroll),
m_word_wrapping(cfg_item_details_word_wrapping),
m_wnd_config(nullptr)
//, m_update_scrollbar_range_in_progress(false)
//m_library_richedit(NULL), m_wnd_richedit(NULL)
{
    m_display_sz.cx = (NULL); m_display_sz.cy = (NULL);
}

void item_details_t::set_config_wnd(HWND wnd)
{
    m_wnd_config = wnd;
}

void item_details_t::set_script(const char * p_script)
{
    m_script = p_script;

    if (get_wnd())
    {
        m_to.release();
        static_api_ptr_t<titleformat_compiler>()->compile_safe(m_to, m_script);

        on_edge_style_change();
        refresh_contents();
    }
}

void item_details_t::on_edge_style_change()
{
    long flags = 0;

    if (m_edge_style == 1) flags |= WS_EX_CLIENTEDGE;
    if (m_edge_style == 2) flags |= WS_EX_STATICEDGE;
    SetWindowLongPtr(get_wnd(), GWL_EXSTYLE, flags);
    SetWindowPos(get_wnd(), nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
}

void item_details_t::set_edge_style(t_size edge_style)
{
    m_edge_style = edge_style;
    if (get_wnd())
    {
        on_edge_style_change();
    }
}

void item_details_t::set_vertical_alignment(t_size vertical_alignment)
{
    if (get_wnd())
    {
        m_vertical_alignment = vertical_alignment;
        invalidate_all(false);
        update_scrollbar_range();
        update_now();
    }
}

void item_details_t::set_horizontal_alignment(t_size horizontal_alignment)
{
    if (get_wnd())
    {
        m_horizontal_alignment = horizontal_alignment;
        invalidate_all(false);
        update_scrollbar_range();
        update_now();
    }
}

void item_details_t::on_playback_order_changed(t_size p_new_index)
{

}

void item_details_t::on_default_format_changed()
{

}

void item_details_t::on_playlist_locked(bool p_locked)
{

}

void item_details_t::on_playlist_renamed(const char * p_new_name, t_size p_new_name_len)
{

}

void item_details_t::on_item_ensure_visible(t_size p_idx)
{

}

void item_details_t::on_items_replaced(const bit_array & p_mask, const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry> & p_data)
{

}

void item_details_t::on_items_modified_fromplayback(const bit_array & p_mask, play_control::t_display_level p_level)
{

}

void item_details_t::on_items_modified(const bit_array & p_mask)
{

}

void item_details_t::on_items_removed(const bit_array & p_mask, t_size p_old_count, t_size p_new_count)
{

}

void item_details_t::on_items_removing(const bit_array & p_mask, t_size p_old_count, t_size p_new_count)
{

}

void item_details_t::on_items_reordered(const t_size * p_order, t_size p_count)
{

}

void item_details_t::on_items_added(t_size p_base, const pfc::list_base_const_t<metadb_handle_ptr> & p_data, const bit_array & p_selection)
{

}

void item_details_t::on_item_focus_change(t_size p_from, t_size p_to)
{

}

void item_details_t::on_volume_change(float p_new_val)
{

}

void item_details_t::on_playback_starting(play_control::t_track_command p_command, bool p_paused)
{

}

const bool item_details_t::g_track_mode_includes_selection(t_size mode)
{
    return mode == track_auto_selection_playing || mode == track_selection;
}

const bool item_details_t::g_track_mode_includes_auto(t_size mode)
{
    return mode == track_auto_playlist_playing || mode == track_auto_selection_playing;
}

const bool item_details_t::g_track_mode_includes_plalist(t_size mode)
{
    return mode == track_auto_playlist_playing || mode == track_playlist;
}

const bool item_details_t::g_track_mode_includes_now_playing(t_size mode)
{
    return mode == track_auto_playlist_playing || mode == track_auto_selection_playing || mode == track_playing;
}

item_details_t::class_data & item_details_t::get_class_data() const
{
    DWORD flags = 0;
    if (m_edge_style == 1) flags |= WS_EX_CLIENTEDGE;
    if (m_edge_style == 2) flags |= WS_EX_STATICEDGE;
    __implement_get_class_data_ex(_T("\r\nCUI_Item_Details_Panel"), _T(""), false, 0, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_CONTROLPARENT | flags, 0);
    //__implement_get_class_data(L"", false);
}


uie::window_factory<item_details_t> g_item_details;




item_details_t::menu_node_wwrap::menu_node_wwrap(item_details_t * p_wnd) : p_this(p_wnd)
{

}

void item_details_t::menu_node_wwrap::execute()
{
    p_this->m_word_wrapping = !p_this->m_word_wrapping;
    cfg_item_details_word_wrapping = p_this->m_word_wrapping;
    p_this->reset_display_info();
    p_this->invalidate_all(false);
    p_this->update_scrollbar_range();
    p_this->update_now();
}

bool item_details_t::menu_node_wwrap::get_description(pfc::string_base & p_out) const
{
    return false;
}

bool item_details_t::menu_node_wwrap::get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const
{
    p_out = "Word wrapping";
    p_displayflags = (p_this->m_word_wrapping) ? ui_extension::menu_node_t::state_checked : 0;
    return true;
}

item_details_t::menu_node_hscroll::menu_node_hscroll(item_details_t * p_wnd) : p_this(p_wnd)
{

}

void item_details_t::menu_node_hscroll::execute()
{
    p_this->m_hscroll = !p_this->m_hscroll;
    cfg_item_details_hscroll = p_this->m_hscroll;
    p_this->reset_display_info();
    p_this->invalidate_all(false);
    p_this->update_scrollbar_range();
    p_this->update_now();
}

bool item_details_t::menu_node_hscroll::get_description(pfc::string_base & p_out) const
{
    return false;
}

bool item_details_t::menu_node_hscroll::get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const
{
    p_out = "Allow horizontal scrolling";
    p_displayflags = (p_this->m_hscroll) ? ui_extension::menu_node_t::state_checked : 0;
    return true;
}

item_details_t::menu_node_alignment_popup::menu_node_alignment_popup(item_details_t * p_wnd)
{
    m_items.add_item(new menu_node_alignment(p_wnd, 0));
    m_items.add_item(new menu_node_alignment(p_wnd, 1));
    m_items.add_item(new menu_node_alignment(p_wnd, 2));
}

void item_details_t::menu_node_alignment_popup::get_child(unsigned p_index, uie::menu_node_ptr & p_out) const
{
    p_out = m_items[p_index].get_ptr();
}

unsigned item_details_t::menu_node_alignment_popup::get_children_count() const
{
    return m_items.get_count();
}

bool item_details_t::menu_node_alignment_popup::get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const
{
    p_out = "Alignment";
    p_displayflags = 0;
    return true;
}

item_details_t::menu_node_alignment::menu_node_alignment(item_details_t * p_wnd, t_size p_value) : p_this(p_wnd), m_type(p_value)
{

}

void item_details_t::menu_node_alignment::execute()
{
    p_this->m_horizontal_alignment = m_type;
    cfg_item_details_horizontal_alignment = m_type;
    p_this->invalidate_all(false);
    p_this->update_scrollbar_range();
    p_this->update_now();
}

bool item_details_t::menu_node_alignment::get_description(pfc::string_base & p_out) const
{
    return false;
}

bool item_details_t::menu_node_alignment::get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const
{
    p_out = get_name(m_type);
    p_displayflags = (m_type == p_this->m_horizontal_alignment) ? ui_extension::menu_node_t::state_radiochecked : 0;
    return true;
}

const char * item_details_t::menu_node_alignment::get_name(t_size source)
{
    if (source == 0) return "Left";
    else if (source == 1) return "Centre";
    else /*if (source == 2)*/ return "Right";
}

item_details_t::menu_node_source_popup::menu_node_source_popup(item_details_t * p_wnd)
{
    m_items.add_item(new menu_node_track_mode(p_wnd, 3));
    m_items.add_item(new menu_node_track_mode(p_wnd, 0));
    m_items.add_item(new uie::menu_node_separator_t());
    m_items.add_item(new menu_node_track_mode(p_wnd, 2));
    m_items.add_item(new menu_node_track_mode(p_wnd, 4));
    m_items.add_item(new menu_node_track_mode(p_wnd, 1));
}

void item_details_t::menu_node_source_popup::get_child(unsigned p_index, uie::menu_node_ptr & p_out) const
{
    p_out = m_items[p_index].get_ptr();
}

unsigned item_details_t::menu_node_source_popup::get_children_count() const
{
    return m_items.get_count();
}

bool item_details_t::menu_node_source_popup::get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const
{
    p_out = "Displayed track";
    p_displayflags = 0;
    return true;
}

item_details_t::menu_node_track_mode::menu_node_track_mode(item_details_t * p_wnd, t_size p_value) : p_this(p_wnd), m_source(p_value)
{

}

void item_details_t::menu_node_track_mode::execute()
{
    p_this->m_tracking_mode = m_source;
    cfg_item_details_tracking_mode = m_source;
    p_this->on_tracking_mode_change();
}

bool item_details_t::menu_node_track_mode::get_description(pfc::string_base & p_out) const
{
    return false;
}

bool item_details_t::menu_node_track_mode::get_display_data(pfc::string_base & p_out, unsigned & p_displayflags) const
{
    p_out = get_name(m_source);
    p_displayflags = (m_source == p_this->m_tracking_mode) ? ui_extension::menu_node_t::state_radiochecked : 0;
    return true;
}

const char * item_details_t::menu_node_track_mode::get_name(t_size source)
{
    if (source == track_playing) return "Playing item";
    if (source == track_playlist) return "Playlist selection";
    if (source == track_auto_selection_playing) return "Automatic (current selection/playing item)";
    if (source == track_selection) return "Current selection";
    return "Automatic (playlist selection/playing item)";
}

