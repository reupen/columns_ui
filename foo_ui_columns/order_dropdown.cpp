#include "stdafx.h"

//#define _WIN32_WINNT 0x500

#define ID_ORDER 2004

#if 0
class playlist_callback_single_order : public playlist_callback_single
{
public:
    virtual void on_items_added(unsigned start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const pfc::bit_array & p_selection){};//inside any of these methods, you can call IPlaylist APIs to get exact info about what happened (but only methods that read playlist state, not those that modify it)
    virtual void on_items_reordered(const unsigned * order,unsigned count){};//changes selection too; doesnt actually change set of items that are selected or item having focus, just changes their order
    virtual void FB2KAPI on_items_removing(const pfc::bit_array & p_mask,unsigned p_old_count,unsigned p_new_count){};//called before actually removing them
    virtual void FB2KAPI on_items_removed(const pfc::bit_array & p_mask,unsigned p_old_count,unsigned p_new_count){};
    virtual void on_items_selection_change(const pfc::bit_array & affected,const pfc::bit_array & state){};
    virtual void on_item_focus_change(unsigned from,unsigned to){};//focus may be -1 when no item has focus; reminder: focus may also change on other callbacks
    virtual void on_items_modified(const pfc::bit_array & p_mask){};
    virtual void FB2KAPI on_items_modified_fromplayback(const pfc::bit_array & p_mask,play_control::t_display_level p_level){};
    virtual void on_items_replaced(const pfc::bit_array & p_mask,const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry> & p_data){};
    virtual void on_item_ensure_visible(unsigned idx){};

    virtual void on_playlist_switch(){};
    virtual void on_playlist_renamed(const char * p_new_name,unsigned p_new_name_len){};
    virtual void on_playlist_locked(bool p_locked){};

    virtual void on_default_format_changed(){};
    virtual void on_playback_order_changed(unsigned p_new_index)
    {
        static_api_ptr_t<playlist_manager> playlist_api;
        const char * name =playlist_api->playback_order_get_name(p_new_index);

        unsigned n, count = combos.get_count();

        for (n = 0; n< count; n++)
        {
            uComboBox_SelectString(combos[n], name);
        }
    }

    mem_block_list_t<HWND> combos;
public:
    void register_callback(HWND combo, bool calloninit)
    {
        combos.add_item(combo);
        if (calloninit)
        {
            static_api_ptr_t<playlist_manager> playlist_api;
            uComboBox_SelectString(combo, playlist_api->playback_order_get_name(playlist_api->playback_order_get_active()));
        }
    }
    unsigned deregister_callback(HWND combo)
    {
        combos.remove_item(combo);
        return combos.get_count();
    }
};

static service_factory_single_transparent_t<playlist_callback_single,playlist_callback_single_order> g_order_modify_callback;
#endif

class order_extension
    : public ui_extension::container_ui_extension
    , public playlist_callback_single {
    static const TCHAR* class_name;

    WNDPROC orderproc{nullptr};

public:
    unsigned min_width{0}, height{0};
    bool initialised{false};
    t_int32 m_mousewheel_delta{0};

    static HFONT font_icon;

    HWND wnd_combo{nullptr};
    LRESULT WINAPI hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    static LRESULT WINAPI main_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;

    order_extension() = default;
    ~order_extension() = default;

    static const GUID extension_guid;

    const GUID& get_extension_guid() const override { return extension_guid; }

    unsigned get_type() const override { return ui_extension::type_toolbar; };

    void get_name(pfc::string_base& out) const override { out.set_string("Playback order"); }
    void get_category(pfc::string_base& out) const override { out.set_string("Toolbars"); }

    class_data& get_class_data() const override { __implement_get_class_data_child_ex(class_name, false, false); }

    void on_items_added(
        unsigned start, const pfc::list_base_const_t<metadb_handle_ptr>& p_data, const pfc::bit_array& p_selection)
        override{}; // inside any of these methods, you can call IPlaylist APIs to get exact info about what happened
                    // (but only methods that read playlist state, not those that modify it)
    void on_items_reordered(const unsigned* order,
        unsigned count) override{}; // changes selection too; doesnt actually change set of items that are selected or
                                    // item having focus, just changes their order
    void FB2KAPI on_items_removing(const pfc::bit_array& p_mask, unsigned p_old_count,
        unsigned p_new_count) override{}; // called before actually removing them
    void FB2KAPI on_items_removed(const pfc::bit_array& p_mask, unsigned p_old_count, unsigned p_new_count) override{};
    void on_items_selection_change(const pfc::bit_array& affected, const pfc::bit_array& state) override{};
    void on_item_focus_change(unsigned from, unsigned to)
        override{}; // focus may be -1 when no item has focus; reminder: focus may also change on other callbacks
    void on_items_modified(const pfc::bit_array& p_mask) override{};
    void FB2KAPI on_items_modified_fromplayback(
        const pfc::bit_array& p_mask, play_control::t_display_level p_level) override{};
    void on_items_replaced(const pfc::bit_array& p_mask,
        const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry>& p_data) override{};
    void on_item_ensure_visible(unsigned idx) override{};

    void on_playlist_switch() override{};
    void on_playlist_renamed(const char* p_new_name, unsigned p_new_name_len) override{};
    void on_playlist_locked(bool p_locked) override{};

    void on_default_format_changed() override{};
    void on_playback_order_changed(unsigned p_new_index) override
    {
        static_api_ptr_t<playlist_manager> playlist_api;
        const char* name = playlist_api->playback_order_get_name(p_new_index);

        uComboBox_SelectString(wnd_combo, name);
    }
};

HFONT order_extension::font_icon = nullptr;

const TCHAR* order_extension::class_name = _T("{ABA09E7E-9C95-443e-BDFC-049D66B324A0}");

LRESULT WINAPI order_extension::main_hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    auto p_this = reinterpret_cast<order_extension*>(GetWindowLongPtr(wnd, GWLP_USERDATA));
    return p_this ? p_this->hook(wnd, msg, wp, lp) : DefWindowProc(wnd, msg, wp, lp);
}

LRESULT order_extension::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_CREATE) {
        if (!font_icon)
            font_icon = uCreateIconFont();

        wnd_combo
            = CreateWindowEx(0, WC_COMBOBOX, nullptr, CBS_DROPDOWNLIST | CBS_SORT | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                0, 0, 100, 300, wnd, (HMENU)ID_ORDER, core_api::get_my_instance(), nullptr);

        initialised = true;

        if (wnd_combo) {
            SetWindowLongPtr(wnd_combo, GWLP_USERDATA, (LPARAM)(this));

            static_api_ptr_t<playlist_manager> playlist_api;

            SendMessage(wnd_combo, WM_SETFONT, (WPARAM)font_icon, MAKELPARAM(1, 0));

            HDC dc = GetDC(wnd_combo);
            HFONT font_old = SelectFont(dc, font_icon);
            unsigned count = playlist_api->playback_order_get_count();

            for (unsigned n = 0; n < count; n++) {
                const char* item = playlist_api->playback_order_get_name(n);
                uSendMessageText(wnd_combo, CB_ADDSTRING, 0, item);
                t_size cx = uih::get_text_width(dc, item, strlen(item));
                min_width = max(min_width, cx);
            }
            SelectFont(dc, font_old);
            ReleaseDC(wnd, dc);

            uComboBox_SelectString(
                wnd_combo, playlist_api->playback_order_get_name(playlist_api->playback_order_get_active()));

            playlist_api->register_callback(this, playlist_callback_single::flag_on_playback_order_changed);

            // g_order_modify_callback.register_callback(wnd_combo, true);

            orderproc = (WNDPROC)SetWindowLongPtr(wnd_combo, GWLP_WNDPROC, (LPARAM)main_hook);

            COMBOBOXINFO cbi;
            memset(&cbi, 0, sizeof(cbi));
            cbi.cbSize = sizeof(cbi);

            GetComboBoxInfo(wnd_combo, &cbi);

            RECT rc_client;
            GetClientRect(wnd_combo, &rc_client);

            min_width += RECT_CX(rc_client) - RECT_CX(cbi.rcItem);
            // min_width+=GetSystemMetrics(SM_CXHTHUMB)+2;//SendMessage(wnd_combo,CB_GETDROPPEDWIDTH ,0,0)+11;
            RECT rc;
            GetWindowRect(wnd_combo, &rc);
            height = rc.bottom - rc.top;
        }
    } else if (msg == WM_WINDOWPOSCHANGED) {
        auto lpwp = (LPWINDOWPOS)lp;
        if (!(lpwp->flags & SWP_NOSIZE)) {
            SetWindowPos(wnd_combo, nullptr, 0, 0, lpwp->cx, 300, SWP_NOZORDER);
        }
    }

    else if (msg == WM_COMMAND && wp == ((CBN_SELCHANGE << 16) | ID_ORDER)) {
        int sel = SendMessage((HWND)lp, CB_GETCURSEL, 0, 0);
        pfc::string8 temp;
        uComboBox_GetText(wnd_combo, sel, temp);
        static_api_ptr_t<playlist_manager> playlist_api;

        unsigned count = playlist_api->playback_order_get_count();

        for (unsigned n = 0; n < count; n++) {
            if (!strcmp(playlist_api->playback_order_get_name(n), temp)) {
                playlist_api->playback_order_set_active(n);
                break;
            }
        }
    } else if (msg == WM_GETMINMAXINFO) {
        auto mmi = LPMINMAXINFO(lp);

        mmi->ptMinTrackSize.x = min_width;
        mmi->ptMinTrackSize.y = height;
        mmi->ptMaxTrackSize.y = height;
        return 0;
    } else if (msg == WM_DESTROY) {
        initialised = false;
        static_api_ptr_t<playlist_manager>()->unregister_callback(this);
        unsigned count = get_class_data().refcount; // g_order_modify_callback.deregister_callback(wnd_combo);
        DestroyWindow(wnd_combo);
        if (count == 1) {
            DeleteFont(font_icon);
            font_icon = nullptr;
        }
    }

    return DefWindowProc(wnd, msg, wp, lp);
}

LRESULT WINAPI order_extension::hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_GETDLGCODE:
        //        return DLGC_WANTARROWS;
        return DLGC_WANTALLKEYS;
    case WM_KEYDOWN:
        if (get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp))
            return 0;
        if (wp == VK_TAB)
            ui_extension::window::g_on_tab(wnd);
        SendMessage(wnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), NULL);
        break;
    case WM_SYSKEYDOWN:
        if (get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp))
            return 0;
        break;
    case WM_UPDATEUISTATE:
        RedrawWindow(wnd, nullptr, nullptr, RDW_INVALIDATE);
        break;
    case WM_MOUSEWHEEL:
        // unsigned scroll_lines = GetNumScrollLines();

        unsigned index = ComboBox_GetCurSel(wnd);
        unsigned count = ComboBox_GetCount(wnd);
        if (count) {
            int zDelta = short(HIWORD(wp));

            // int delta = MulDiv(zDelta, scroll_lines, 120);
            m_mousewheel_delta += zDelta;
            int scroll_lines = 1; // GetNumScrollLines();
            if (scroll_lines == -1)
                scroll_lines = count;

            if (m_mousewheel_delta * scroll_lines >= WHEEL_DELTA) {
                if (index) {
                    ComboBox_SetCurSel(wnd, index - 1);
                    SendMessage(GetAncestor(wnd, GA_PARENT), WM_COMMAND, (CBN_SELCHANGE << 16) | ID_ORDER, (LPARAM)wnd);
                }
                m_mousewheel_delta = 0;
            } else if (m_mousewheel_delta * scroll_lines <= -WHEEL_DELTA) {
                if (index + 1 < count) {
                    ComboBox_SetCurSel(wnd, index + 1);
                    SendMessage(GetAncestor(wnd, GA_PARENT), WM_COMMAND, (CBN_SELCHANGE << 16) | ID_ORDER, (LPARAM)wnd);
                }
                m_mousewheel_delta = 0;
            }
        }

        return 0;
        //    case WM_CHAR:
        //        if (wp == VK_LEFT || wp == VK_RIGHT)
    }
    return uCallWindowProc(orderproc, wnd, msg, wp, lp);
}

// {ABA09E7E-9C95-443e-BDFC-049D66B324A0}
const GUID order_extension::extension_guid
    = {0xaba09e7e, 0x9c95, 0x443e, {0xbd, 0xfc, 0x4, 0x9d, 0x66, 0xb3, 0x24, 0xa0}};

ui_extension::window_factory<order_extension> blah;