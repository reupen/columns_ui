#pragma once

#include "stdafx.h"

class toolbar_extension : public ui_extension::container_ui_extension {
    static const TCHAR* class_name;
    int width{0};
    int height{0};

    enum t_config_version { VERSION_1, VERSION_2, VERSION_CURRENT = VERSION_2 };

    enum t_filter { FILTER_NONE, FILTER_PLAYING, FILTER_PLAYLIST, FILTER_ACTIVE_SELECTION };

    enum t_type {
        TYPE_SEPARATOR,
        TYPE_BUTTON,
        TYPE_MENU_ITEM_CONTEXT,
        TYPE_MENU_ITEM_MAIN,
    };

    enum t_show { SHOW_IMAGE, SHOW_IMAGE_TEXT, SHOW_TEXT };

    enum t_appearance { APPEARANCE_NORMAL, APPEARANCE_FLAT, APPEARANCE_NOEDGE };

    /** For config dialog */
    enum { MSG_BUTTON_CHANGE = WM_USER + 2, MSG_COMMAND_CHANGE = WM_USER + 3 };

    class_data& get_class_data() const override;

    WNDPROC menuproc{nullptr};
    bool initialised{false}, m_gdiplus_initialised{false};
    ULONG_PTR m_gdiplus_instance;

public:
    class button {
    public:
        t_type m_type{TYPE_SEPARATOR};
        t_filter m_filter{FILTER_ACTIVE_SELECTION};
        t_show m_show{SHOW_IMAGE};
        GUID m_guid{};
        GUID m_subcommand{};
        bool m_use_custom{false};
        bool m_use_custom_hot{false};
        bool m_use_custom_text{false};
        pfc::string_simple m_text;
        class custom_image {
        public:
            pfc::string_simple m_path;
            pfc::string_simple m_mask_path;
            COLORREF m_mask_colour;
            ui_extension::t_mask m_mask_type;
            custom_image& operator=(const custom_image& p_source);
            void get_path(pfc::string8& p_out) const;
            void write(stream_writer* out, abort_callback& p_abort) const;
            void read(t_config_version p_version, stream_reader* reader, abort_callback& p_abort);
            void write_to_file(stream_writer& p_file, bool b_paths, abort_callback& p_abort);
            void read_from_file(t_config_version p_version, const char* p_base, const char* p_name,
                stream_reader* p_file, unsigned p_size, abort_callback& p_abort);

        } m_custom_image, m_custom_hot_image;
        service_ptr_t<uie::button> m_interface;

        class callback_impl : public uie::button_callback {
            void on_button_state_change(unsigned p_new_state) override; // see t_button_state

            void on_command_state_change(unsigned p_new_state) override{};

            service_ptr_t<toolbar_extension> m_this;
            unsigned id{0};

        public:
            callback_impl& operator=(const callback_impl& p_source);
            void set_wnd(toolbar_extension* p_source);
            void set_id(const unsigned i);
            callback_impl() = default;
        } m_callback;

        void set(const button& p_source);

        button(GUID p_guid, bool p_custom, const char* p_custom_bitmap_path, const char* p_custom_bitmap_mask_path,
            COLORREF p_custom_bitmap_colour_mask, ui_extension::t_mask p_custom_bitmap_mask_type);
        button() = default;
        button& operator=(const button& p_source);
        void write(stream_writer* out, abort_callback& p_abort) const;

        void read(t_config_version p_version, stream_reader* reader, abort_callback& p_abort);
        void get_display_text(pfc::string_base& p_out); // display
        void get_short_name(pfc::string_base& p_out); // tooltip

        void get_name_type(pfc::string_base& p_out); // config
        void get_name_name(pfc::string_base& p_out); // config
        void get_name(pfc::string_base& p_out); // config
        void write_to_file(stream_writer& p_file, bool b_paths, abort_callback& p_abort);
        void read_from_file(t_config_version p_version, const char* p_base, const char* p_name, stream_reader* p_file,
            unsigned p_size, abort_callback& p_abort);
    };

    class button_image {
        HBITMAP m_bm{nullptr};
        HICON m_icon{nullptr};
        ui_extension::t_mask m_mask_type{uie::MASK_NONE};
        HBITMAP m_bm_mask{nullptr};
        COLORREF m_mask_colour{0};

    public:
        button_image() = default;
        ~button_image();
        bool is_valid();
        void load(const button::custom_image& p_image);
        void load(const service_ptr_t<uie::button>& p_in, COLORREF colour_btnface, unsigned cx, unsigned cy);
        unsigned add_to_imagelist(HIMAGELIST iml);
        void get_size(SIZE& p_out);
    };

    static const button g_button_null;

    HWND wnd_toolbar{nullptr};
    HWND wnd_host{nullptr};

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;

    toolbar_extension();
    ~toolbar_extension();

    static const GUID extension_guid;

    const GUID& get_extension_guid() const override;

    void get_name(pfc::string_base& out) const override;

    void get_category(pfc::string_base& out) const override;

    class config_param {
    public:
        // service_ptr_t<toolbar_extension> m_this;
        class t_button_list_view : public uih::ListView {
            config_param& m_param;
            static CLIPFORMAT g_clipformat();
            struct DDData {
                t_uint32 version;
                HWND wnd;
            };
            class IDropTarget_buttons_list : public IDropTarget {
                long drop_ref_count;
                bool last_rmb;
                t_button_list_view* m_button_list_view;
                mmh::ComPtr<IDataObject> m_DataObject;
                mmh::ComPtr<IDropTargetHelper> m_DropTargetHelper;
                // pfc::string
            public:
                HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID FAR* ppvObject) override;
                ULONG STDMETHODCALLTYPE AddRef() override;
                ULONG STDMETHODCALLTYPE Release() override;
                bool check_do(IDataObject* pDO);
                HRESULT STDMETHODCALLTYPE DragEnter(
                    IDataObject* pDataObj, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect) override;
                HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect) override;
                HRESULT STDMETHODCALLTYPE DragLeave() override;
                HRESULT STDMETHODCALLTYPE Drop(
                    IDataObject* pDataObj, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect) override;
                IDropTarget_buttons_list(t_button_list_view* p_blv);
            };
            void notify_on_initialisation() override;
            void notify_on_create() override;
            void notify_on_destroy() override;
            void notify_on_selection_change(const pfc::bit_array& p_affected, const pfc::bit_array& p_status,
                notification_source_t p_notification_source) override;
            bool do_drag_drop(WPARAM wp) override;

        public:
            t_button_list_view(config_param& p_param) : m_param(p_param){};
        } m_button_list;

        modal_dialog_scope m_scope;
        // uih::ListView m_button_list;
        button* m_selection{nullptr};
        HWND m_wnd{nullptr}, m_child{nullptr};
        unsigned m_active{0};
        button::custom_image* m_image{nullptr};
        pfc::list_t<button> m_buttons;
        bool m_text_below{false};
        t_appearance m_appearance{APPEARANCE_NORMAL};
        void export_to_file(const char* p_path, bool b_paths = false);
        void import_from_file(const char* p_path, bool add);
        void export_to_stream(stream_writer* p_writer, bool b_paths, abort_callback& p_abort);
        void import_from_stream(stream_reader* p_reader, bool add, abort_callback& p_abort);

        static BOOL CALLBACK g_ConfigPopupProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
        BOOL ConfigPopupProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

        void on_selection_change(t_size index);
        void populate_buttons_list();
        void refresh_buttons_list_items(t_size index, t_size count, bool b_update_display = true);

        config_param();
    };

    static BOOL CALLBACK ConfigChildProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
    static BOOL CALLBACK ConfigCommandProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    bool have_config_popup() const override { return true; }

    bool show_config_popup(HWND wnd_parent) override;

    void create_toolbar();
    void destroy_toolbar();

    void get_menu_items(ui_extension::menu_hook_t& p_hook) override;

    unsigned get_type() const override;

    pfc::list_t<button> m_buttons;
    pfc::list_t<button> m_buttons_config;

    bool m_text_below{false};
    t_appearance m_appearance{APPEARANCE_NORMAL};

    static void reset_buttons(pfc::list_base_t<button>& p_buttons);

    void get_config(stream_writer* data, abort_callback& p_abort) const override;
    void set_config(stream_reader* p_reader, t_size p_sizehint, abort_callback& p_abort) override;
    void import_config(stream_reader* p_reader, t_size p_size, abort_callback& p_abort) override;
    void export_config(stream_writer* p_writer, abort_callback& p_abort) const override;

    //    virtual void write_to_file(stream_writer * out);

    static const GUID g_guid_fcb;

    enum t_identifier {
        I_TEXT_BELOW,
        I_APPEARANCE,
        I_BUTTONS,
    };
    enum t_identifier_button {
        I_BUTTON_TYPE,
        I_BUTTON_FILTER,
        I_BUTTON_SHOW,
        I_BUTTON_GUID,
        I_BUTTON_CUSTOM,
        I_BUTTON_CUSTOM_DATA,
        I_BUTTON_CUSTOM_HOT,
        I_BUTTON_CUSTOM_HOT_DATA,
        I_BUTTON_MASK_TYPE,
        I_BUTTON_CUSTOM_IMAGE_DATA,
        I_BUTTON_CUSTOM_IMAGE_MASK_DATA,
        I_BUTTON_MASK_COLOUR,
        I_BUTTON_USE_CUSTOM_TEXT,
        I_BUTTON_TEXT,
        I_BUTTON_SUBCOMMAND
    };

    enum t_custom_image_identifiers {
        I_CUSTOM_BUTTON_PATH,
        I_CUSTOM_BUTTON_MASK_PATH,
        // I_BUTTON_MASK_TYPE=8
    };

    enum t_image_identifiers { IMAGE_NAME, IMAGE_DATA, IMAGE_PATH };
};

class command_picker_param {
public:
    GUID m_guid{};
    GUID m_subcommand{};
    unsigned m_group;
    unsigned m_filter;
    command_picker_param() = delete;
    command_picker_param(GUID p_guid, GUID p_subcommand, unsigned p_group, unsigned p_filter)
        : m_guid(p_guid), m_subcommand(p_subcommand), m_group(p_group), m_filter(p_filter){};
};

class command_picker_data {
    modal_dialog_scope m_scope;
    class command_data {
    public:
        GUID m_guid{};
        GUID m_subcommand{};
        pfc::string8 m_desc;
    };
    std::vector<std::unique_ptr<command_data>> m_data;
    HWND m_wnd;
    HWND wnd_group;
    HWND wnd_filter;
    HWND wnd_command;
    unsigned m_group;
    GUID m_guid{};
    GUID m_subcommand{};
    unsigned m_filter;

    bool __populate_mainmenu_dynamic_recur(
        command_data& data, const mainmenu_node::ptr& ptr_node, pfc::string_base& full, bool b_root);
    bool __populate_commands_recur(
        command_data& data, pfc::string_base& full, contextmenu_item_node* p_node, bool b_root);
    void populate_commands();
    void update_description();

public:
    void set_data(const command_picker_param& p_data);
    void get_data(command_picker_param& p_data) const;
    void initialise(HWND wnd);
    void deinitialise(HWND wnd);
    BOOL on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
};

namespace pfc {
template <>
class traits_t<toolbar_extension::t_image_identifiers> : public traits_rawobject {
};
template <>
class traits_t<toolbar_extension::t_custom_image_identifiers> : public traits_rawobject {
};
template <>
class traits_t<toolbar_extension::t_identifier_button> : public traits_rawobject {
};
template <>
class traits_t<toolbar_extension::t_identifier> : public traits_rawobject {
};
template <>
class traits_t<toolbar_extension::t_appearance> : public traits_rawobject {
};
template <>
class traits_t<toolbar_extension::t_config_version> : public traits_rawobject {
};
template <>
class traits_t<toolbar_extension::t_show> : public traits_rawobject {
};
template <>
class traits_t<toolbar_extension::t_filter> : public traits_rawobject {
};
template <>
class traits_t<toolbar_extension::t_type> : public traits_rawobject {
};
template <>
class traits_t<uie::t_mask> : public traits_rawobject {
};
} // namespace pfc
