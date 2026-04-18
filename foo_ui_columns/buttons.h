#pragma once

#include "pch.h"

#include "core_dark_list_view.h"

#include "buttons_button.h"

namespace cui::toolbars::buttons {

enum class IconSize : int32_t {
    Automatic,
    Custom,
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
    I_ICON_SIZE,
    I_WIDTH,
    I_HEIGHT,
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
    I_BUTTON_RESERVED_1,
    I_BUTTON_RESERVED_2,
    I_BUTTON_RESERVED_3,
    I_BUTTON_RESERVED_4,
    I_BUTTON_USE_CUSTOM_TEXT,
    I_BUTTON_TEXT,
    I_BUTTON_SUBCOMMAND
};

class ButtonsToolbar : public uie::container_uie_window_v3 {
public:
    static constexpr GUID extension_guid{0xd8e65660, 0x64ed, 0x42e7, {0x85, 0xb, 0x31, 0xd8, 0x28, 0xc2, 0x52, 0x94}};

    static void s_on_font_change();

    HWND wnd_toolbar{nullptr};
    HWND wnd_host{nullptr};

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;

    ButtonsToolbar();
    ~ButtonsToolbar();

    const GUID& get_extension_guid() const override;

    void get_name(pfc::string_base& out) const override;

    void get_category(pfc::string_base& out) const override;

    class ConfigParam {
    public:
        class ButtonsList : public helpers::CoreDarkListView {
            ConfigParam& m_param;

            void notify_on_initialisation() override;
            void notify_on_create() override;
            void notify_on_destroy() override;
            void notify_on_selection_change(const bit_array& p_affected, const bit_array& p_status,
                notification_source_t p_notification_source) override;
            bool do_drag_drop(WPARAM wp) override;
            void move_selection(int delta) override;

            void move_item(size_t old_index, size_t new_index);

        public:
            explicit ButtonsList(ConfigParam& p_param) : CoreDarkListView(true), m_param(p_param) {}
        } m_button_list;

        void export_to_file(const char* p_path, bool b_paths = false);
        void import_from_file(const char* p_path, bool add);
        void export_to_stream(stream_writer* p_writer, bool b_paths, abort_callback& p_abort);
        void import_from_stream(stream_reader* p_reader, bool add, abort_callback& p_abort);

        INT_PTR on_dialog_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
        void update_size_field_status();

        void on_selection_change(size_t index);
        void populate_buttons_list();
        void refresh_buttons_list_items(size_t index, size_t count, bool b_update_display = true);

        ConfigParam();

        // uih::ListView m_button_list;
        bool m_initialising{};
        Button* m_selection{nullptr};
        HWND m_wnd{nullptr};
        wil::unique_hfont m_h1_font;
        wil::unique_hfont m_h2_font;
        unsigned m_active{0};
        std::vector<Button> m_buttons;
        bool m_text_below{false};
        Appearance m_appearance{APPEARANCE_NORMAL};
        IconSize m_icon_size{IconSize::Automatic};
        uih::IntegerAndDpi<int32_t> m_width{16};
        uih::IntegerAndDpi<int32_t> m_height{16};
    };

    bool have_config_popup() const override { return true; }
    bool show_config_popup(HWND wnd_parent) override;

    template <class List>
    void configure(List&& buttons, bool text_below, Appearance appearance, IconSize icon_size,
        uih::IntegerAndDpi<int32_t> width, uih::IntegerAndDpi<int32_t> height)
    {
        const auto was_initialised = initialised;
        if (was_initialised) {
            destroy_toolbar();
        }
        m_text_below = text_below;
        m_appearance = appearance;
        m_icon_size = icon_size;
        m_width = width;
        m_height = height;
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

private:
    uie::container_window_v3_config get_window_config() override
    {
        uie::container_window_v3_config config(L"columns_ui_buttons_toolbar_XXpeF2WDs-4");
        config.forward_wm_settingchange = false;
        config.invalidate_children_on_move_or_resize = true;
        return config;
    }

    void on_font_change();

    inline static std::vector<ButtonsToolbar*> s_instances;
    inline static wil::unique_hfont s_font;

    WNDPROC menuproc{nullptr};
    bool initialised{false};
    std::vector<Button> m_buttons;
    bool m_text_below{false};
    Appearance m_appearance{APPEARANCE_NORMAL};
    IconSize m_icon_size{IconSize::Automatic};
    uih::IntegerAndDpi<int32_t> m_width{16};
    uih::IntegerAndDpi<int32_t> m_height{16};
    wil::unique_himagelist m_standard_images;
    wil::unique_himagelist m_hot_images;
    std::unique_ptr<colours::dark_mode_notifier> m_dark_mode_notifier;
};

struct CommandPickerData {
    GUID guid{};
    GUID subcommand{};
    int group{TYPE_SEPARATOR};
    int filter{FILTER_ACTIVE_SELECTION};
};

class CommandPickerDialog {
public:
    CommandPickerDialog(CommandPickerData data = {}) : m_data(std::move(data)) {}

    std::tuple<bool, CommandPickerData> open_modal(HWND wnd);

private:
    class CommandData {
    public:
        GUID m_guid{};
        GUID m_subcommand{};
        pfc::string8 m_desc;
    };

    bool __populate_mainmenu_dynamic_recur(
        CommandData& data, const mainmenu_node::ptr& ptr_node, std::list<std::string> name_parts, bool b_root);
    bool __populate_commands_recur(
        CommandData& data, std::list<std::string> name_parts, contextmenu_item_node* p_node, bool b_root);
    void populate_commands();
    void update_description();
    void initialise(HWND wnd);
    void deinitialise(HWND wnd);
    INT_PTR on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    std::vector<std::unique_ptr<CommandData>> m_commands;
    HWND m_wnd{};
    HWND wnd_group{};
    HWND wnd_filter{};
    HWND wnd_command{};
    CommandPickerData m_data;
};

} // namespace cui::toolbars::buttons

namespace pfc {

template <>
class traits_t<cui::toolbars::buttons::ButtonIdentifier> : public traits_rawobject {};
template <>
class traits_t<cui::toolbars::buttons::Identifier> : public traits_rawobject {};
template <>
class traits_t<cui::toolbars::buttons::Appearance> : public traits_rawobject {};
template <>
class traits_t<uie::t_mask> : public traits_rawobject {};

} // namespace pfc
