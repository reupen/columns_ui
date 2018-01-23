#include "stdafx.h"
#include "NG Playlist/ng_playlist.h"
#include "artwork.h"
#include "config.h"

namespace artwork_panel {
// extern cfg_string cfg_front, cfg_back, cfg_disc;//, cfg_icon;
extern cfg_uint cfg_fb2k_artwork_mode, cfg_edge_style;
void g_on_repository_change();
} // namespace artwork_panel

class artwork_source_t {
public:
    cfg_objList<pfc::string8>* m_scripts;
    const char* m_name;

    artwork_source_t(cfg_objList<pfc::string8>& p_scripts, const char* p_name)
        : m_scripts(&p_scripts), m_name(p_name){};
};

artwork_source_t g_artwork_sources[] = {
    artwork_source_t(artwork_panel::cfg_front_scripts, "Front cover"),
    artwork_source_t(artwork_panel::cfg_back_scripts, "Back cover"),
    artwork_source_t(artwork_panel::cfg_disc_scripts, "Disc cover"),
    artwork_source_t(artwork_panel::cfg_artist_scripts, "Artist picture"),
};

static class tab_artwork : public preferences_tab {
public:
    tab_artwork() = default;

    static t_size get_combined_index(t_size index, t_size subindex)
    {
        t_size i = 0, ret = 0;
        while (i < tabsize(g_artwork_sources) && i < index) {
            ret += g_artwork_sources[i].m_scripts->get_count();
            i++;
        }
        ret += subindex;
        return ret;
    }

    static bool get_separated_index(t_size combined_index, t_size& index, t_size& subindex)
    {
        index = 0;
        subindex = 0;
        while (
            index < tabsize(g_artwork_sources) && g_artwork_sources[index].m_scripts->get_count() <= combined_index) {
            combined_index -= g_artwork_sources[index].m_scripts->get_count();
            index++;
        }
        subindex = combined_index;
        return (index < tabsize(g_artwork_sources) && subindex < g_artwork_sources[index].m_scripts->get_count());
    }

    static void get_group_combined_index(t_size index, t_size& combined_index_start, t_size& count)
    {
        combined_index_start = 0;
        count = 0;
        t_size i = 0;
        while (i < tabsize(g_artwork_sources) && i < index) {
            combined_index_start += g_artwork_sources[i].m_scripts->get_count();
            i++;
        }
        count = g_artwork_sources[index].m_scripts->get_count();
        // return (index < tabsize(g_artwork_sources) && subindex < g_artwork_sources[index].m_scripts->get_count());
    }

    static void get_group_from_combined_index(
        t_size combined_index, t_size& index, t_size& subindex, t_size& combined_index_start, t_size& count)
    {
        get_separated_index(combined_index, index, subindex);

        get_group_combined_index(index, combined_index_start, count);
    }

    class t_list_view_artwork : public uih::ListView {
    public:
        t_size m_edit_index, m_edit_subindex, m_edit_combined_index;
        t_list_view_artwork()
            : m_edit_index(pfc_infinite), m_edit_subindex(pfc_infinite), m_edit_combined_index(pfc_infinite){};

        void notify_on_create() override
        {
            set_single_selection(true);
            set_group_count(1);
            set_autosize(true);
            set_show_header(false);
            pfc::list_t<Column> columns;
            columns.set_count(1);
            columns[0].m_title = "Source script";
            columns[0].m_size = 100;
            uih::ListView::set_columns(columns);
        };
        bool notify_before_create_inline_edit(
            const pfc::list_base_const_t<t_size>& indices, unsigned column, bool b_source_mouse) override
        {
            if (column == 0 && indices.get_count() == 1)
                return true;
            return false;
        };
        bool notify_create_inline_edit(const pfc::list_base_const_t<t_size>& indices, unsigned column,
            pfc::string_base& p_text, t_size& p_flags, mmh::ComPtr<IUnknown>& pAutocompleteEntries) override
        {
            t_size indices_count = indices.get_count();
            if (indices_count == 1) {
                t_size index, subindex;
                if (get_separated_index(indices[0], index, subindex)) {
                    m_edit_index = index;
                    m_edit_subindex = subindex;
                    m_edit_combined_index = indices[0];

                    p_text = g_artwork_sources[index].m_scripts->get_item(subindex);

                    return true;
                }
            }
            return false;
        };
        void notify_save_inline_edit(const char* value) override
        {
            if (m_edit_index < tabsize(g_artwork_sources)
                && m_edit_subindex < g_artwork_sources[m_edit_index].m_scripts->get_count()) {
                pfc::string8& dest = (*g_artwork_sources[m_edit_index].m_scripts)[m_edit_subindex];
                if (strcmp(dest, value)) {
                    dest = value;
                    pfc::list_t<uih::ListView::InsertItem> items;
                    items.set_count(1);
                    {
                        items[0].m_groups.set_size(1);
                        items[0].m_subitems.set_size(1);

                        items[0].m_groups[0] = g_artwork_sources[m_edit_index].m_name;
                        items[0].m_subitems[0] = dest;
                    }
                    replace_items(m_edit_combined_index, items);

                    m_changed = true;
                }
            }

            m_edit_subindex = pfc_infinite;
            m_edit_index = pfc_infinite;
            m_edit_combined_index = pfc_infinite;
        }
        void notify_on_kill_focus(HWND wnd_receiving) override { on_scripts_change(); };
        void execute_default_action(t_size index, t_size column, bool b_keyboard, bool b_ctrl) override
        {
            if (!b_keyboard)
                activate_inline_editing();
        };
        void on_scripts_change()
        {
            if (m_changed) {
                artwork_panel::g_on_repository_change();
                pvt::ng_playlist_view_t::g_on_artwork_repositories_change();
                m_changed = false;
            }
        }
        bool m_changed{ false };

    private:
    } m_source_list;

    void refresh_me(HWND wnd)
    {
        initialising = true;
#if 0
        HWND wnd_edit = GetDlgItem(wnd, IDC_FRONT);
        uSetWindowText(wnd_edit, artwork_panel::cfg_front);
        wnd_edit = GetDlgItem(wnd, IDC_BACK);
        uSetWindowText(wnd_edit, artwork_panel::cfg_back);
        wnd_edit = GetDlgItem(wnd, IDC_DISC);
        uSetWindowText(wnd_edit, artwork_panel::cfg_disc);
        //wnd_edit = GetDlgItem(wnd, IDC_ICON);
        //uSetWindowText(wnd_edit, artwork_panel::cfg_icon);
#endif

        m_source_list.remove_items(pfc::bit_array_true());

        t_size index, indexcount = tabsize(g_artwork_sources);
        for (index = 0; index < indexcount; index++) {
            t_size subindex, subindexcount = g_artwork_sources[index].m_scripts->get_count();
            pfc::array_t<uih::ListView::InsertItem> items;
            items.set_count(subindexcount);
            for (subindex = 0; subindex < subindexcount; subindex++) {
                items[subindex].m_groups.set_size(1);
                items[subindex].m_subitems.set_size(1);
                items[subindex].m_groups[0] = g_artwork_sources[index].m_name;
                items[subindex].m_subitems[0] = (*g_artwork_sources[index].m_scripts)[subindex];
            }
            m_source_list.insert_items(m_source_list.get_item_count(), items.get_count(), items.get_ptr());
        }

        HWND wnd_combo = GetDlgItem(wnd, IDC_FB2KARTWORK);
        ComboBox_AddString(wnd_combo, L"Disabled");
        ComboBox_AddString(wnd_combo, L"Embedded artwork");
        ComboBox_AddString(wnd_combo, L"Embedded and external artwork");
        ComboBox_SetCurSel(wnd_combo, artwork_panel::cfg_fb2k_artwork_mode);

        wnd_combo = GetDlgItem(wnd, IDC_EDGESTYLE);
        ComboBox_AddString(wnd_combo, L"None");
        ComboBox_AddString(wnd_combo, L"Sunken");
        ComboBox_AddString(wnd_combo, L"Grey");
        ComboBox_SetCurSel(wnd_combo, artwork_panel::cfg_edge_style);
        initialising = false;
    }

    static BOOL CALLBACK g_on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        tab_artwork* p_data = nullptr;
        if (msg == WM_INITDIALOG) {
            p_data = reinterpret_cast<tab_artwork*>(lp);
            SetWindowLongPtr(wnd, DWLP_USER, lp);
        } else
            p_data = reinterpret_cast<tab_artwork*>(GetWindowLongPtr(wnd, DWLP_USER));
        return p_data ? p_data->on_message(wnd, msg, wp, lp) : FALSE;
    }

    void on_scripts_change() { m_source_list.on_scripts_change(); }

    BOOL CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg) {
        case WM_INITDIALOG: {
            HWND wnd_fields = m_source_list.create(wnd, uih::WindowPosition(20, 44, 276, 80), true);
            SetWindowPos(wnd_fields, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

            refresh_me(wnd);

            ShowWindow(wnd_fields, SW_SHOWNORMAL);
        } break;
        case WM_DESTROY: {
            on_scripts_change();
        } break;
        case WM_COMMAND:
            switch (wp) {
#if 0
            case (EN_CHANGE<<16)|IDC_FRONT:
                artwork_panel::cfg_front = string_utf8_from_window((HWND)lp);
                m_changed = true;
                break;
            case (EN_CHANGE<<16)|IDC_BACK:
                artwork_panel::cfg_back = string_utf8_from_window((HWND)lp);
                m_changed = true;
                break;
            case (EN_CHANGE<<16)|IDC_DISC:
                artwork_panel::cfg_disc = string_utf8_from_window((HWND)lp);
                m_changed = true;
                break;
            case (EN_KILLFOCUS<<16)|IDC_FRONT:
            case (EN_KILLFOCUS<<16)|IDC_BACK:
            case (EN_KILLFOCUS<<16)|IDC_DISC:
                on_scripts_change();
                break;
#endif
            /*case (EN_CHANGE<<16)|IDC_ICON:
                artwork_panel::cfg_icon = string_utf8_from_window((HWND)lp);
                m_changed = true;
                break;*/
            case IDC_FB2KARTWORK | (CBN_SELCHANGE << 16):
                artwork_panel::cfg_fb2k_artwork_mode = ComboBox_GetCurSel((HWND)lp);
                break;
            case IDC_EDGESTYLE | (CBN_SELCHANGE << 16):
                artwork_panel::cfg_edge_style = ComboBox_GetCurSel((HWND)lp);
                artwork_panel::artwork_panel_t::g_on_edge_style_change();
                break;
            case IDC_ADD: {
                RECT rc;
                GetWindowRect((HWND)lp, &rc);
                HMENU menu = CreatePopupMenu();

                enum { IDM_FRONT = 1 };

                t_size index, indexcount = tabsize(g_artwork_sources);
                for (index = 0; index < indexcount; index++) {
                    AppendMenuW(menu, (MF_STRING), index + 1,
                        pfc::stringcvt::string_wide_from_utf8(g_artwork_sources[index].m_name));
                }

                int cmd = TrackPopupMenu(
                    menu, TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, rc.left, rc.bottom, 0, wnd, nullptr);
                DestroyMenu(menu);
                if (cmd > 0 && (t_size)cmd <= indexcount) {
                    index = cmd - 1;
                    t_size subindex = g_artwork_sources[index].m_scripts->add_item("<enter script>");

                    t_size combined_index = get_combined_index(index, subindex);

                    uih::ListView::InsertItem item(1, 1);
                    item.m_groups[0] = g_artwork_sources[index].m_name;
                    item.m_subitems[0] = "<enter script>";
                    m_source_list.insert_items(combined_index, 1, &item);
                    SetFocus(m_source_list.get_wnd());
                    m_source_list.set_item_selected_single(combined_index);
                    m_source_list.activate_inline_editing();
                    m_source_list.m_changed = true;
                }
            } break;
            case IDC_REMOVE: {
                if (m_source_list.get_selection_count(2) == 1) {
                    pfc::bit_array_bittable mask(m_source_list.get_item_count());
                    m_source_list.get_selection_state(mask);
                    // bool b_found = false;
                    t_size combined_index = 0, count = m_source_list.get_item_count();
                    while (combined_index < count) {
                        if (mask[combined_index])
                            break;
                        combined_index++;
                    }
                    t_size index, subindex;
                    if (combined_index < count && get_separated_index(combined_index, index, subindex)) {
                        g_artwork_sources[index].m_scripts->remove_by_idx(subindex);
                        m_source_list.remove_item(combined_index);
                        m_source_list.m_changed = true;
                        t_size new_count = m_source_list.get_item_count();
                        if (new_count) {
                            if (combined_index < new_count)
                                m_source_list.set_item_selected_single(combined_index);
                            else if (combined_index)
                                m_source_list.set_item_selected_single(combined_index - 1);
                        }
                    }
                }
            } break;
            case IDC_UP: {
                if (m_source_list.get_selection_count(2) == 1) {
                    t_size combined_index = 0;
                    {
                        t_size count = m_source_list.get_item_count();
                        while (!m_source_list.get_item_selected(combined_index) && combined_index < count)
                            combined_index++;
                    }

                    t_size index, subindex, combined_index_start, count;

                    get_group_from_combined_index(combined_index, index, subindex, combined_index_start, count);

                    if (subindex && count) {
                        g_artwork_sources[index].m_scripts->swap_items(subindex, subindex - 1);

                        pfc::list_t<uih::ListView::SizedInsertItem<1, 1>> items;
                        items.set_count(2);

                        items[0].m_groups[0] = g_artwork_sources[index].m_name;
                        items[1].m_groups[0] = g_artwork_sources[index].m_name;
                        items[0].m_subitems[0] = (*g_artwork_sources[index].m_scripts)[subindex - 1];
                        items[1].m_subitems[0] = (*g_artwork_sources[index].m_scripts)[subindex];
                        m_source_list.replace_items(combined_index - 1, items);
                        m_source_list.set_item_selected_single(combined_index - 1);
                        m_source_list.m_changed = true;
                    }
                }
            } break;
            case IDC_DOWN: {
                if (m_source_list.get_selection_count(2) == 1) {
                    t_size combined_index = 0;
                    {
                        t_size count = m_source_list.get_item_count();
                        while (!m_source_list.get_item_selected(combined_index) && combined_index < count)
                            combined_index++;
                    }

                    t_size index, subindex, combined_index_start, count;

                    get_group_from_combined_index(combined_index, index, subindex, combined_index_start, count);

                    if (subindex + 1 < count) {
                        g_artwork_sources[index].m_scripts->swap_items(subindex, subindex + 1);
                        pfc::list_t<uih::ListView::SizedInsertItem<1, 1>> items;
                        items.set_count(2);

                        items[0].m_groups[0] = g_artwork_sources[index].m_name;
                        items[1].m_groups[0] = g_artwork_sources[index].m_name;
                        items[0].m_subitems[0] = (*g_artwork_sources[index].m_scripts)[subindex];
                        items[1].m_subitems[0] = (*g_artwork_sources[index].m_scripts)[subindex + 1];

                        m_source_list.replace_items(combined_index, items);
                        m_source_list.set_item_selected_single(combined_index + 1);
                        m_source_list.m_changed = true;
                    }
                }
            } break;
            }
        }
        return 0;
    }
    void apply() {}
    HWND create(HWND wnd) override { return uCreateDialog(IDD_ARTWORK, wnd, g_on_message, (LPARAM)this); }
    const char* get_name() override { return "Artwork"; }
    bool get_help_url(pfc::string_base& p_out) override
    {
        p_out = "http://yuo.be/wiki/columns_ui:config:artwork";
        return true;
    }

private:
    bool initialising{ false }; //, m_changed;
} g_tab_artwork;

preferences_tab* g_get_tab_artwork()
{
    return &g_tab_artwork;
}
