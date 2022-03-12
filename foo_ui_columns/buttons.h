#pragma once

#include "pch.h"

namespace cui::toolbars::buttons {

class ButtonsToolbar : public ui_extension::container_ui_extension {
public:
    enum ConfigVersion { VERSION_1, VERSION_2, VERSION_CURRENT = VERSION_2 };

    /** For config dialog */
    enum { MSG_BUTTON_CHANGE = WM_USER + 2, MSG_COMMAND_CHANGE = WM_USER + 3 };

    enum Filter : int32_t {
        FILTER_NONE,
        FILTER_PLAYING,
        FILTER_PLAYLIST,
        FILTER_ACTIVE_SELECTION,
    };

    enum Type : int32_t {
        TYPE_SEPARATOR,
        TYPE_BUTTON,
        TYPE_MENU_ITEM_CONTEXT,
        TYPE_MENU_ITEM_MAIN,
    };

    enum Show : uint32_t {
        SHOW_IMAGE,
        SHOW_IMAGE_TEXT,
        SHOW_TEXT,
    };

    enum Appearance : uint32_t {
        APPEARANCE_NORMAL,
        APPEARANCE_FLAT,
        APPEARANCE_NOEDGE,
    };

    enum Identifier {
        I_TEXT_BELOW,
        I_APPEARANCE,
        I_BUTTONS,
    };
    enum ButtonIdentifier {
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

    enum CustomImageIdentifier {
        I_CUSTOM_BUTTON_PATH,
        I_CUSTOM_BUTTON_MASK_PATH,
        // I_BUTTON_MASK_TYPE=8
    };

    enum ImageIdentifier { IMAGE_NAME, IMAGE_DATA, IMAGE_PATH };

    class Button {
    public:
        class CustomImage {
        public:
            pfc::string_simple m_path;
            pfc::string_simple m_mask_path;
            COLORREF m_mask_colour{};
            ui_extension::t_mask m_mask_type{uie::MASK_NONE};

            void get_path(pfc::string8& p_out) const;
            void write(stream_writer* out, abort_callback& p_abort) const;
            void read(ConfigVersion p_version, stream_reader* reader, abort_callback& p_abort);
            void write_to_file(stream_writer& p_file, bool b_paths, abort_callback& p_abort);
            void read_from_file(ConfigVersion p_version, const char* p_base, const char* p_name, stream_reader* p_file,
                unsigned p_size, abort_callback& p_abort);
        };

        Type m_type{TYPE_SEPARATOR};
        Filter m_filter{FILTER_ACTIVE_SELECTION};
        Show m_show{SHOW_IMAGE};
        GUID m_guid{};
        GUID m_subcommand{};
        bool m_use_custom{false};
        bool m_use_custom_hot{false};
        bool m_use_custom_text{false};
        pfc::string_simple m_text;
        CustomImage m_custom_image;
        CustomImage m_custom_hot_image;
        service_ptr_t<uie::button> m_interface;

        class ButtonStateCallback : public uie::button_callback {
            void on_button_state_change(unsigned p_new_state) override; // see t_button_state

            void on_command_state_change(unsigned p_new_state) override {}

            service_ptr_t<ButtonsToolbar> m_this;
            int id{};

        public:
            ButtonStateCallback& operator=(const ButtonStateCallback& p_source);
            void set_wnd(ButtonsToolbar* p_source);
            void set_id(int i);
            ButtonStateCallback() = default;
        } m_callback;

        void set(const Button& p_source);

        void write(stream_writer* out, abort_callback& p_abort) const;

        void read(ConfigVersion p_version, stream_reader* reader, abort_callback& p_abort);
        std::string get_display_text() const;

        std::string get_type_desc() const;
        std::string get_name(bool short_form = false) const;
        std::string get_name_with_type() const;
        void write_to_file(stream_writer& p_file, bool b_paths, abort_callback& p_abort);
        void read_from_file(ConfigVersion p_version, const char* p_base, const char* p_name, stream_reader* p_file,
            unsigned p_size, abort_callback& p_abort);
    };

    class ButtonImage {
        HBITMAP m_bm{nullptr};
        HICON m_icon{nullptr};
        ui_extension::t_mask m_mask_type{uie::MASK_NONE};
        HBITMAP m_bm_mask{nullptr};
        COLORREF m_mask_colour{0};

    public:
        ButtonImage() = default;
        ButtonImage(const ButtonImage&) = delete;
        ButtonImage& operator=(const ButtonImage&) = delete;
        ButtonImage(ButtonImage&&) = delete;
        ButtonImage& operator=(ButtonImage&&) = delete;
        ~ButtonImage();
        bool is_valid();
        void load(const Button::CustomImage& p_image);
        void load(const service_ptr_t<uie::button>& p_in, COLORREF colour_btnface, unsigned cx, unsigned cy);
        unsigned add_to_imagelist(HIMAGELIST iml);
        void get_size(SIZE& p_out);
    };

    HWND wnd_toolbar{nullptr};
    HWND wnd_host{nullptr};

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;

    ButtonsToolbar();
    ~ButtonsToolbar();

    static const GUID extension_guid;

    const GUID& get_extension_guid() const override;

    void get_name(pfc::string_base& out) const override;

    void get_category(pfc::string_base& out) const override;

    class ConfigParam {
    public:
        // service_ptr_t<toolbar_extension> m_this;
        class ButtonsList : public uih::ListView {
            ConfigParam& m_param;
            static CLIPFORMAT g_clipformat();
            struct DDData {
                t_uint32 version;
                HWND wnd;
            };
            class ButtonsListDropTarget : public IDropTarget {
                long drop_ref_count;
                bool last_rmb;
                ButtonsList* m_button_list_view;
                wil::com_ptr_t<IDataObject> m_DataObject;
                wil::com_ptr_t<IDropTargetHelper> m_DropTargetHelper;
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
                explicit ButtonsListDropTarget(ButtonsList* p_blv);
            };
            void notify_on_initialisation() override;
            void notify_on_create() override;
            void notify_on_destroy() override;
            void notify_on_selection_change(const bit_array& p_affected, const bit_array& p_status,
                notification_source_t p_notification_source) override;
            bool do_drag_drop(WPARAM wp) override;

        public:
            explicit ButtonsList(ConfigParam& p_param) : m_param(p_param) {}
        } m_button_list;

        modal_dialog_scope m_scope;
        // uih::ListView m_button_list;
        Button* m_selection{nullptr};
        HWND m_wnd{nullptr}, m_child{nullptr};
        unsigned m_active{0};
        Button::CustomImage* m_image{nullptr};
        std::vector<Button> m_buttons;
        bool m_text_below{false};
        Appearance m_appearance{APPEARANCE_NORMAL};
        void export_to_file(const char* p_path, bool b_paths = false);
        void import_from_file(const char* p_path, bool add);
        void export_to_stream(stream_writer* p_writer, bool b_paths, abort_callback& p_abort);
        void import_from_stream(stream_reader* p_reader, bool add, abort_callback& p_abort);

        static INT_PTR CALLBACK g_ConfigPopupProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
        BOOL ConfigPopupProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

        void on_selection_change(size_t index);
        void populate_buttons_list();
        void refresh_buttons_list_items(size_t index, size_t count, bool b_update_display = true);

        ConfigParam();
    };

    static INT_PTR CALLBACK ConfigChildProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    bool have_config_popup() const override { return true; }
    bool show_config_popup(HWND wnd_parent) override;

    template <class List>
    void configure(List&& buttons, bool text_below, Appearance appearance)
    {
        const auto was_initialised = initialised;
        if (was_initialised) {
            destroy_toolbar();
        }
        m_text_below = text_below;
        m_appearance = appearance;
        m_buttons = buttons;
        if (was_initialised) {
            create_toolbar();
            get_host()->on_size_limit_change(wnd_host, ui_extension::size_limit_minimum_width);
        }
    }

    void create_toolbar();
    void destroy_toolbar();
    void set_window_theme() const;

    void get_menu_items(ui_extension::menu_hook_t& p_hook) override;

    unsigned get_type() const override;

    static void reset_buttons(std::vector<Button>& p_buttons);

    void get_config(stream_writer* data, abort_callback& p_abort) const override;
    void set_config(stream_reader* p_reader, size_t p_sizehint, abort_callback& p_abort) override;
    void import_config(stream_reader* p_reader, size_t p_size, abort_callback& p_abort) override;
    void export_config(stream_writer* p_writer, abort_callback& p_abort) const override;

    //    virtual void write_to_file(stream_writer * out);

    static const GUID g_guid_fcb;

private:
    class_data& get_class_data() const override;

    static const TCHAR* class_name;
    int width{0};
    int height{0};

    WNDPROC menuproc{nullptr};
    bool initialised{false}, m_gdiplus_initialised{false};
    ULONG_PTR m_gdiplus_instance{};
    std::vector<Button> m_buttons;
    bool m_text_below{false};
    Appearance m_appearance{APPEARANCE_NORMAL};
    wil::unique_himagelist m_standard_images;
    wil::unique_himagelist m_hot_images;
    std::unique_ptr<colours::dark_mode_notifier> m_dark_mode_notifier;
};

class CommandPickerParam {
public:
    GUID m_guid{};
    GUID m_subcommand{};
    int m_group{ButtonsToolbar::TYPE_SEPARATOR};
    int m_filter{ButtonsToolbar::FILTER_ACTIVE_SELECTION};
};

class CommandPickerData {
    modal_dialog_scope m_scope;
    class CommandData {
    public:
        GUID m_guid{};
        GUID m_subcommand{};
        pfc::string8 m_desc;
    };
    std::vector<std::unique_ptr<CommandData>> m_data;
    HWND m_wnd{};
    HWND wnd_group{};
    HWND wnd_filter{};
    HWND wnd_command{};
    int m_group{ButtonsToolbar::TYPE_SEPARATOR};
    GUID m_guid{};
    GUID m_subcommand{};
    int m_filter{ButtonsToolbar::FILTER_ACTIVE_SELECTION};

    bool __populate_mainmenu_dynamic_recur(
        CommandData& data, const mainmenu_node::ptr& ptr_node, std::list<std::string> name_parts, bool b_root);
    bool __populate_commands_recur(
        CommandData& data, std::list<std::string> name_parts, contextmenu_item_node* p_node, bool b_root);
    void populate_commands();
    void update_description();

public:
    void set_data(const CommandPickerParam& p_data);
    void get_data(CommandPickerParam& p_data) const;
    void initialise(HWND wnd);
    void deinitialise(HWND wnd);
    INT_PTR on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
};

} // namespace cui::toolbars::buttons

namespace pfc {
template <>
class traits_t<cui::toolbars::buttons::ButtonsToolbar::ImageIdentifier> : public traits_rawobject {
};
template <>
class traits_t<cui::toolbars::buttons::ButtonsToolbar::CustomImageIdentifier> : public traits_rawobject {
};
template <>
class traits_t<cui::toolbars::buttons::ButtonsToolbar::ButtonIdentifier> : public traits_rawobject {
};
template <>
class traits_t<cui::toolbars::buttons::ButtonsToolbar::Identifier> : public traits_rawobject {
};
template <>
class traits_t<cui::toolbars::buttons::ButtonsToolbar::Appearance> : public traits_rawobject {
};
template <>
class traits_t<cui::toolbars::buttons::ButtonsToolbar::ConfigVersion> : public traits_rawobject {
};
template <>
class traits_t<cui::toolbars::buttons::ButtonsToolbar::Show> : public traits_rawobject {
};
template <>
class traits_t<cui::toolbars::buttons::ButtonsToolbar::Filter> : public traits_rawobject {
};
template <>
class traits_t<cui::toolbars::buttons::ButtonsToolbar::Type> : public traits_rawobject {
};
template <>
class traits_t<uie::t_mask> : public traits_rawobject {
};
} // namespace pfc
