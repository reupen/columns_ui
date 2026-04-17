#pragma once

#include "buttons_config.h"

namespace cui::toolbars::buttons {

enum Show : uint32_t {
    SHOW_IMAGE,
    SHOW_IMAGE_TEXT,
    SHOW_TEXT,
};

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

enum CustomImageIdentifier {
    I_CUSTOM_BUTTON_PATH,
    I_CUSTOM_BUTTON_MASK_PATH,
    I_CUSTOM_BUTTON_MASK_TYPE = 8,
    I_CUSTOM_BUTTON_IMAGE_DATA = 9,
    I_CUSTOM_BUTTON_MASK_DATA = 10,
    I_CUSTOM_BUTTON_MASK_COLOUR = 11
};

enum ImageIdentifier {
    IMAGE_NAME,
    IMAGE_DATA,
    IMAGE_PATH
};

enum class CustomImageContentType {
    Ico,
    Svg,
    Other,
};

class ButtonStateCallback : public uie::button_callback {
public:
    ButtonStateCallback(uie::button::ptr button_instance, HWND toolbar_wnd, int button_id)
        : m_button_instance(std::move(button_instance))
        , m_toolbar_wnd(toolbar_wnd)
        , m_button_id(button_id)
    {
        m_button_instance->register_callback(*this);
    }

    ~ButtonStateCallback() { m_button_instance->deregister_callback(*this); }

private:
    void on_button_state_change(unsigned p_new_state) final;

    void on_command_state_change(unsigned p_new_state) final {}

    uie::button::ptr m_button_instance;
    HWND m_toolbar_wnd{};
    int m_button_id{};
};

class MainMenuStateCallback : public mainmenu_commands_v3::state_callback {
public:
    MainMenuStateCallback(mainmenu_commands_v3::ptr mainmenu_commands_v3_instance, uint32_t command_index,
        HWND toolbar_wnd, int button_id)
        : m_mainmenu_commands_v3(std::move(mainmenu_commands_v3_instance))
        , m_command_index(command_index)
        , m_toolbar_wnd(toolbar_wnd)
        , m_button_id(button_id)
    {
        m_mainmenu_commands_v3->add_state_callback(this);
    }

    ~MainMenuStateCallback() { m_mainmenu_commands_v3->remove_state_callback(this); }

private:
    void menu_state_changed(const GUID& main, const GUID& sub) final;

    mainmenu_commands_v3::ptr m_mainmenu_commands_v3;
    uint32_t m_command_index{};
    HWND m_toolbar_wnd{};
    int m_button_id{};
};

class CustomImage {
public:
    pfc::string_simple m_path;
    pfc::string_simple m_mask_path;
    COLORREF m_mask_colour{};
    ui_extension::t_mask m_mask_type{uie::MASK_NONE};

    [[nodiscard]] CustomImageContentType content_type() const;
    [[nodiscard]] pfc::string8 get_path() const;

    void write(stream_writer* out, abort_callback& p_abort) const;
    void read(ConfigVersion p_version, stream_reader* reader, abort_callback& p_abort);
    void write_to_file(stream_writer& p_file, bool b_paths, abort_callback& p_abort);
    void read_from_file(FCBVersion p_version, const char* p_base, const char* p_name, stream_reader* p_file,
        unsigned p_size, abort_callback& p_abort);
};

class Button {
public:
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

    uie::button::ptr m_interface;
    mainmenu_commands_v3::ptr m_mainmenu_commands_v3;
    std::optional<uint32_t> m_mainmenu_commands_index{};
    std::shared_ptr<ButtonStateCallback> m_button_state_callback;
    std::shared_ptr<MainMenuStateCallback> m_mainmenu_state_callback;

    void reset_state();

    void write(stream_writer* out, abort_callback& p_abort) const;

    void read(ConfigVersion p_version, stream_reader* reader, abort_callback& p_abort);
    std::string get_display_text() const;

    std::string get_type_desc() const;
    std::string get_name(bool short_form = false) const;
    std::string get_name_with_type() const;
    void write_to_file(stream_writer& p_file, bool b_paths, abort_callback& p_abort);
    void read_from_file(FCBVersion p_version, const char* p_base, const char* p_name, stream_reader* p_file,
        unsigned p_size, abort_callback& p_abort);
};

} // namespace cui::toolbars::buttons

namespace pfc {

template <>
class traits_t<cui::toolbars::buttons::ImageIdentifier> : public traits_rawobject {};
template <>
class traits_t<cui::toolbars::buttons::CustomImageIdentifier> : public traits_rawobject {};
template <>
class traits_t<cui::toolbars::buttons::Show> : public traits_rawobject {};
template <>
class traits_t<cui::toolbars::buttons::Filter> : public traits_rawobject {};
template <>
class traits_t<cui::toolbars::buttons::Type> : public traits_rawobject {};

} // namespace pfc
