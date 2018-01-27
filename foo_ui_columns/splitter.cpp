#include "stdafx.h"
#include "splitter.h"

pfc::ptr_list_t<splitter_window_impl> splitter_window_impl::g_instances;

void g_get_panel_list(uie::window_info_list_simple& p_out, uie::window_host_ptr& p_host)
{
    service_enum_t<ui_extension::window> e;
    uie::window_ptr l;

    if (e.first(l))
        do {
            if (true) {
                uie::window_info_simple info;

                if (l->is_available(p_host)) {
                    l->get_name(info.name);
                    l->get_category(info.category);
                    info.guid = l->get_extension_guid();
                    info.prefer_multiple_instances = l->get_prefer_multiple_instances();
                    info.type = l->get_type();
                    p_out.add_item(info);
                }
            }
        } while (e.next(l));

    p_out.sort_by_category_and_name();
}

void g_append_menu_panels(HMENU menu, const uie::window_info_list_simple& panels, UINT base)
{
    HMENU popup = nullptr;
    unsigned n, count = panels.get_count();
    for (n = 0; n < count; n++) {
        if (!n || uStringCompare(panels[n - 1].category, panels[n].category)) {
            if (n)
                uAppendMenu(menu, MF_STRING | MF_POPUP, (UINT)popup, panels[n - 1].category);
            popup = CreatePopupMenu();
        }
        uAppendMenu(popup, (MF_STRING), base + n, panels[n].name);
        if (n == count - 1)
            uAppendMenu(menu, MF_STRING | MF_POPUP, (UINT)popup, panels[n].category);
    }
}

void g_append_menu_splitters(HMENU menu, const uie::window_info_list_simple& panels, UINT base)
{
    unsigned n, count = panels.get_count();
    for (n = 0; n < count; n++) {
        if (panels[n].type & uie::type_splitter)
            uAppendMenu(menu, (MF_STRING), base + n, panels[n].name);
    }
}

/// don't pass smartptrs by reference as they may be nuked when destroying stuff
void g_run_live_edit_contextmenu(HWND wnd, POINT pt_menu, uih::TranslucentFillWindow& p_overlay, const RECT& rc_overlay,
    uie::window_ptr ptr, uie::splitter_window_ptr p_container, t_size index, uie::window_host_ptr& p_host)
{
    // console::print("g_run_live_edit_contextmenu");
    // if (!m_trans_fill.get_wnd())
    {
        HWND wnd_over = p_overlay.create(wnd, uih::WindowPosition(rc_overlay));
        HWND wnd_root = (GetAncestor(wnd, GA_ROOT));
        // HWND wnd_next = GetWindow(wnd_root, GW_HWNDNEXT);
        WindowEnum_t WindowEnum(wnd_root);
        WindowEnum.run();
        // console::formatter() << WindowEnum.m_wnd_list.get_count() << pfc::format_hex((t_size)wnd_root, 8) << " " <<
        // pfc::format_hex((t_size)wnd_next, 8); SetWindowPos(wnd_over, GetWindow(GetAncestor(wnd, GA_ROOT),
        // GW_HWNDNEXT), 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
        t_size count_owned = WindowEnum.m_wnd_list.get_count();
        if (count_owned) {
            // console::formatter() << count_owned << " " <<
            // pfc::format_hex((t_uint32)WindowEnum.m_wnd_list[count_owned-1]) << " " <<
            // string_utf8_from_window(WindowEnum.m_wnd_list[count_owned-1]);
            SetWindowPos(wnd_over, WindowEnum.m_wnd_list[count_owned - 1], 0, 0, 0, 0,
                SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
        }
        ShowWindow(wnd_over, SW_SHOWNOACTIVATE);

        HMENU menu = CreatePopupMenu();
        HMENU menu_change = CreatePopupMenu();
        uie::window_info_list_simple panels;
        g_get_panel_list(panels, p_host);
        enum { ID_CLOSE = 1, ID_CHANGE_BASE = 2 };

        uie::splitter_window_ptr p_splitter;
        if (ptr.is_valid())
            ptr->service_query_t(p_splitter);

        g_append_menu_panels(menu_change, panels, ID_CHANGE_BASE);
        pfc::string8 temp;
        if (ptr.is_valid())
            ptr->get_name(temp);
        uAppendMenu(menu, MF_STRING | MF_GRAYED, (UINT_PTR)0, temp);
        // uAppendMenu(menu, MF_MENUBREAK, (UINT_PTR)0, NULL);

        const UINT_PTR ID_ADD_BASE = ID_CHANGE_BASE + panels.get_count();
        const UINT_PTR ID_CHANGE_SPLITTER_BASE = ID_ADD_BASE + panels.get_count();
        const UINT_PTR ID_PARENT_ADD_BASE = ID_CHANGE_SPLITTER_BASE + panels.get_count();
        if (p_splitter.is_valid()) {
            if (p_splitter->get_panel_count() < p_splitter->get_maximum_panel_count()) {
                HMENU menu_add = CreatePopupMenu();
                g_append_menu_panels(menu_add, panels, ID_ADD_BASE);
                AppendMenu(menu, MF_STRING | MF_POPUP, (UINT_PTR)menu_add, L"Add panel");
            }
            HMENU menu_change = CreatePopupMenu();
            g_append_menu_splitters(menu_change, panels, ID_CHANGE_SPLITTER_BASE);
            AppendMenu(menu, MF_STRING | MF_POPUP, (UINT_PTR)menu_change, L"Change splitter");
        }

        AppendMenu(menu, MF_STRING | MF_POPUP, (UINT_PTR)menu_change, L"Change panel");
        AppendMenu(menu, MF_STRING, ID_CLOSE, L"Close");

        if (p_container->get_panel_count() < p_container->get_maximum_panel_count()) {
            uAppendMenu(menu, MF_MENUBREAK, (UINT_PTR)0, nullptr);
            p_container->get_name(temp);
            uAppendMenu(menu, MF_STRING | MF_GRAYED, (UINT_PTR)0, temp);

            HMENU menu_add = CreatePopupMenu();
            g_append_menu_panels(menu_add, panels, ID_PARENT_ADD_BASE);
            AppendMenu(menu, MF_STRING | MF_POPUP, (UINT_PTR)menu_add, L"Add panel");
        }
        auto cmd = (unsigned)TrackPopupMenu(
            menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt_menu.x, pt_menu.y, 0, wnd, nullptr);
        p_overlay.destroy();
        {
            {
                if (cmd) {
                    if (cmd == ID_CLOSE) {
                        p_container->remove_panel(index);
                    } else if (cmd >= ID_CHANGE_BASE && cmd < panels.get_count() + ID_CHANGE_BASE) {
                        t_size panel_index = cmd - ID_CHANGE_BASE;
                        uie::splitter_item_ptr si;
                        si = new uie::splitter_item_simple_t;
                        si->set_panel_guid(panels[panel_index].guid);
                        p_container->replace_panel(index, si.get_ptr());
                    } else if (cmd >= ID_ADD_BASE && cmd < panels.get_count() + ID_ADD_BASE) {
                        t_size panel_index = cmd - ID_ADD_BASE;
                        uie::splitter_item_ptr si;
                        si = new uie::splitter_item_simple_t;
                        si->set_panel_guid(panels[panel_index].guid);
                        p_splitter->add_panel(si.get_ptr());
                    } else if (cmd >= ID_CHANGE_SPLITTER_BASE && cmd < panels.get_count() + ID_CHANGE_SPLITTER_BASE) {
                        t_size panel_index = cmd - ID_CHANGE_SPLITTER_BASE;

                        uie::window_ptr window;
                        service_ptr_t<uie::splitter_window> splitter;
                        if (uie::window::create_by_guid(panels[panel_index].guid, window)
                            && window->service_query_t(splitter)) {
                            unsigned n, count = min(p_splitter->get_panel_count(), splitter->get_maximum_panel_count());
                            if (count == p_splitter->get_panel_count()
                                || MessageBox(wnd,
                                       _T("The number of child items will not fit in the selected splitter type. ")
                                       _T("Continue?"),
                                       _T("Warning"), MB_YESNO | MB_ICONEXCLAMATION)
                                    == IDYES) {
                                for (n = 0; n < count; n++) {
                                    uie::splitter_item_ptr ptr;
                                    p_splitter->get_panel(n, ptr);
                                    splitter->add_panel(ptr.get_ptr());
                                }
                                uie::splitter_item_ptr newsi;
                                p_container->get_panel(index, newsi);

                                stream_writer_memblock conf;
                                abort_callback_dummy p_abort;
                                try {
                                    splitter->get_config(&conf, p_abort);
                                } catch (const pfc::exception&) {
                                };
                                newsi->set_panel_guid(panels[panel_index].guid);
                                newsi->set_panel_config_from_ptr(conf.m_data.get_ptr(), conf.m_data.get_size());

                                p_container->replace_panel(index, newsi.get_ptr());
                            }
                        }

                    } else if (cmd >= ID_PARENT_ADD_BASE && cmd < panels.get_count() + ID_PARENT_ADD_BASE) {
                        t_size panel_index = cmd - ID_PARENT_ADD_BASE;
                        uie::splitter_item_ptr si;
                        si = new uie::splitter_item_simple_t;
                        si->set_panel_guid(panels[panel_index].guid);
                        p_container->add_panel(si.get_ptr());
                    }
                }
            }
        }
        DestroyMenu(menu);
    }
}

void clip_minmaxinfo(MINMAXINFO& mmi)
{
    mmi.ptMinTrackSize.x = min(mmi.ptMinTrackSize.x, MAXSHORT);
    mmi.ptMinTrackSize.y = min(mmi.ptMinTrackSize.y, MAXSHORT);
    mmi.ptMaxTrackSize.y = min(mmi.ptMaxTrackSize.y, MAXSHORT);
    mmi.ptMaxTrackSize.x = min(mmi.ptMaxTrackSize.x, MAXSHORT);
}

BOOL uDrawPanelTitle(HDC dc, const RECT* rc_clip, const char* text, int len, bool vert, bool world)
{
    COLORREF cr_back = GetSysColor(COLOR_3DFACE);
    COLORREF cr_fore = GetSysColor(COLOR_MENUTEXT);
    COLORREF cr_line = GetSysColor(COLOR_3DSHADOW);

    {
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, cr_fore);

        SIZE sz;
        uGetTextExtentPoint32(dc, text, len, &sz);
        int extra = vert ? rc_clip->bottom - sz.cy : (rc_clip->bottom - rc_clip->top - sz.cy - 1) / 2;
        /*
        if (world)
        {
        SetGraphicsMode(dc, GM_ADVANCED);
        XFORM xf;
        xf.eM11 = 0;
        xf.eM21 = 1;
        xf.eDx = 0;
        xf.eM12 = -1;
        xf.eM22 = 0;
        xf.eDy = rc_clip->right;
        SetWorldTransform(dc, &xf);
        }
        */
        //        HFONT old = SelectFont(dc, fnt_menu);

        uExtTextOut(dc, 5 + rc_clip->left, extra, ETO_CLIPPED, rc_clip, text, len, nullptr);
        //        SelectFont(dc, old);

        return TRUE;
    }
    return FALSE;
}

class splitter_window_horizontal : public splitter_window_impl {
    class_data& get_class_data() const override
    {
        __implement_get_class_data_ex(_T("{72FACC90-BB7E-4733-8449-D7537232AD26}"), _T(""), false, 0,
            WS_CHILD | WS_CLIPCHILDREN, WS_EX_CONTROLPARENT, CS_DBLCLKS);
    }
    void get_name(pfc::string_base& p_out) const override { p_out = "Horizontal splitter"; }
    const GUID& get_extension_guid() const override
    {
        // {8FA0BC24-882A-4fff-8A3B-215EA7FBD07F}
        static const GUID rv = {0x8fa0bc24, 0x882a, 0x4fff, {0x8a, 0x3b, 0x21, 0x5e, 0xa7, 0xfb, 0xd0, 0x7f}};
        return rv;
    }
    orientation_t get_orientation() const override { return horizontal; }
};

class splitter_window_vertical : public splitter_window_impl {
    class_data& get_class_data() const override
    {
        __implement_get_class_data_ex(_T("{77653A44-66D1-49e0-9A7A-1C71898C0441}"), _T(""), false, 0,
            WS_CHILD | WS_CLIPCHILDREN, WS_EX_CONTROLPARENT, CS_DBLCLKS);
    }
    void get_name(pfc::string_base& p_out) const override { p_out = "Vertical splitter"; }
    const GUID& get_extension_guid() const override
    {
        // {77653A44-66D1-49e0-9A7A-1C71898C0441}
        static const GUID rv = {0x77653a44, 0x66d1, 0x49e0, {0x9a, 0x7a, 0x1c, 0x71, 0x89, 0x8c, 0x4, 0x41}};
        return rv;
    }
    orientation_t get_orientation() const override { return vertical; }
};

uie::window_factory<splitter_window_horizontal> g_splitter_window_horizontal;
uie::window_factory<splitter_window_vertical> g_splitter_window_vertical;

splitter_window_impl::panel::ptr splitter_window_impl::panel::null_ptr = splitter_window_impl::panel::ptr();

#if 0
template <orientation_t t_orientation>
class dummy_class
{
    static ui_helpers::container_window::class_data myint;
};

template <orientation_t t_orientation>
int dummy_class<t_orientation>::myint = {_T("dummy"), _T(""), 0, false, false, 0, WS_CHILD|WS_CLIPCHILDREN, WS_EX_CONTROLPARENT, 0};
#endif

//#endif
