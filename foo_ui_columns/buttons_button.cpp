#include "stdafx.h"
#include "buttons.h"

void ButtonsToolbar::Button::ButtonStateCallback::on_button_state_change(unsigned p_new_state) // see t_button_state
{
    unsigned state = SendMessage(m_this->wnd_toolbar, TB_GETSTATE, id, 0);
    if (p_new_state & uie::BUTTON_STATE_ENABLED)
        state |= TBSTATE_ENABLED;
    else
        state = state & ~TBSTATE_ENABLED;
    if (p_new_state & uie::BUTTON_STATE_PRESSED)
        state |= TBSTATE_PRESSED;
    else
        state = state & ~TBSTATE_PRESSED;
    SendMessage(m_this->wnd_toolbar, TB_SETSTATE, id, MAKELONG(state, 0));
};

ButtonsToolbar::Button::ButtonStateCallback& ButtonsToolbar::Button::ButtonStateCallback::operator=(
    const ButtonStateCallback& p_source)
{
    m_this = p_source.m_this;
    return *this;
}
void ButtonsToolbar::Button::ButtonStateCallback::set_wnd(ButtonsToolbar* p_source)
{
    m_this = p_source;
}
void ButtonsToolbar::Button::ButtonStateCallback::set_id(const unsigned i)
{
    id = i;
}

void ButtonsToolbar::Button::set(const ButtonsToolbar::Button& p_source)
{
    m_guid = p_source.m_guid;
    m_subcommand = p_source.m_subcommand;
    m_use_custom = p_source.m_use_custom;
    m_use_custom_hot = p_source.m_use_custom_hot;
    m_custom_image = p_source.m_custom_image;
    m_custom_hot_image = p_source.m_custom_hot_image;
    m_interface = p_source.m_interface;
    m_callback = p_source.m_callback;
    m_type = p_source.m_type;
    m_filter = p_source.m_filter;
    m_show = p_source.m_show;
    m_use_custom_text = p_source.m_use_custom_text;
    m_text = p_source.m_text;
}

void ButtonsToolbar::Button::write(stream_writer* out, abort_callback& p_abort) const
{
    out->write_lendian_t(m_type, p_abort);
    out->write_lendian_t(m_filter, p_abort);
    out->write_lendian_t((GUID&)m_guid, p_abort);
    out->write_lendian_t((GUID&)m_subcommand, p_abort);
    out->write_lendian_t(m_show, p_abort);
    out->write_lendian_t(m_use_custom, p_abort);
    if (m_use_custom) {
        m_custom_image.write(out, p_abort);
    }
    out->write_lendian_t(m_use_custom_hot, p_abort);
    if (m_use_custom_hot) {
        m_custom_hot_image.write(out, p_abort);
    }
    out->write_lendian_t(m_use_custom_text, p_abort);
    if (m_use_custom_text)
        out->write_string(m_text, p_abort);
}

void ButtonsToolbar::Button::read(
    ButtonsToolbar::ConfigVersion p_version, stream_reader* reader, abort_callback& p_abort)
{
    *this = Button{};

    reader->read_lendian_t(m_type, p_abort);
    reader->read_lendian_t(m_filter, p_abort);
    reader->read_lendian_t((GUID&)m_guid, p_abort);
    if (p_version >= VERSION_2)
        reader->read_lendian_t((GUID&)m_subcommand, p_abort);
    reader->read_lendian_t(m_show, p_abort);
    reader->read_lendian_t(m_use_custom, p_abort);
    if (m_use_custom) {
        m_custom_image.read(p_version, reader, p_abort);
    }
    reader->read_lendian_t(m_use_custom_hot, p_abort);
    if (m_use_custom_hot) {
        m_custom_hot_image.read(p_version, reader, p_abort);
    }
    reader->read_lendian_t(m_use_custom_text, p_abort);
    if (m_use_custom_text) {
        pfc::string8 temp;
        reader->read_string(temp, p_abort);
        m_text = temp;
    }
}
void ButtonsToolbar::Button::get_display_text(pfc::string_base& p_out) // display
{
    p_out.reset();
    if (m_use_custom_text)
        p_out = m_text;
    else
        get_short_name(p_out);
}
void ButtonsToolbar::Button::get_short_name(pfc::string_base& p_out) // tooltip
{
    p_out.reset();
    if (m_type == TYPE_BUTTON)
        uie::custom_button::g_button_get_name(m_guid, p_out);
    else if (m_type == TYPE_SEPARATOR)
        p_out = "Separator";
    else if (m_type == TYPE_MENU_ITEM_MAIN)
        menu_helpers::mainpath_from_guid(m_guid, m_subcommand, p_out, true);
    else
        menu_helpers::contextpath_from_guid(m_guid, m_subcommand, p_out, true);
}

void ButtonsToolbar::Button::get_name_type(pfc::string_base& p_out) // config
{
    p_out.reset();
    if (m_type == TYPE_BUTTON) {
        p_out = "Button";
    } else if (m_type == TYPE_SEPARATOR)
        p_out = "Separator";
    else if (m_type == TYPE_MENU_ITEM_MAIN) {
        p_out = "Main menu item";
    } else {
        p_out = "Context menu item";
    }
}

void ButtonsToolbar::Button::get_name_name(pfc::string_base& p_out) // config
{
    p_out.reset();
    if (m_type == TYPE_BUTTON) {
        pfc::string8 temp;
        if (uie::custom_button::g_button_get_name(m_guid, temp)) {
            p_out += temp;
        }
    } else if (m_type == TYPE_SEPARATOR)
        p_out = "-";
    else if (m_type == TYPE_MENU_ITEM_MAIN) {
        pfc::string8 temp;
        menu_helpers::mainpath_from_guid(m_guid, m_subcommand, temp);
        p_out += temp;
    } else {
        pfc::string8 temp;
        menu_helpers::contextpath_from_guid(m_guid, m_subcommand, temp);
        p_out += temp;
    }
}
void ButtonsToolbar::Button::get_name(pfc::string_base& p_out) // config
{
    p_out.reset();
    if (m_type == TYPE_BUTTON) {
        p_out = "[Button] ";
        pfc::string8 temp;
        if (uie::custom_button::g_button_get_name(m_guid, temp)) {
            p_out += temp;
        }
    } else if (m_type == TYPE_SEPARATOR)
        p_out = "[Separator]";
    else if (m_type == TYPE_MENU_ITEM_MAIN) {
        pfc::string8 temp;
        p_out = "[Main menu item] ";
        menu_helpers::mainpath_from_guid(m_guid, m_subcommand, temp);
        p_out += temp;
    } else {
        pfc::string8 temp;
        menu_helpers::contextpath_from_guid(m_guid, m_subcommand, temp);
        p_out = "[Context menu item] ";
        p_out += temp;
    }
}

void ButtonsToolbar::Button::read_from_file(ConfigVersion p_version, const char* p_base, const char* p_name,
    stream_reader* p_file, unsigned p_size, abort_callback& p_abort)
{
    // t_filesize p_start = p_file->get_position(p_abort);
    t_size read = 0;
    while (read < p_size /* && !p_file->is_eof(p_abort)*/) {
        ButtonIdentifier id;
        p_file->read_lendian_t(id, p_abort);
        unsigned size;
        p_file->read_lendian_t(size, p_abort);
        // if (size > p_file->get_size(p_abort) - p_file->get_position(p_abort))
        //    throw exception_io_data();
        read += sizeof(t_uint32) + sizeof(t_uint32) + size;
        switch (id) {
        case I_BUTTON_TYPE:
            p_file->read_lendian_t(m_type, p_abort);
            break;
        case I_BUTTON_FILTER:
            p_file->read_lendian_t(m_filter, p_abort);
            break;
        case I_BUTTON_SHOW:
            p_file->read_lendian_t(m_show, p_abort);
            break;
        case I_BUTTON_GUID:
            p_file->read_lendian_t((GUID&)m_guid, p_abort);
            break;
        case I_BUTTON_SUBCOMMAND:
            p_file->read_lendian_t((GUID&)m_subcommand, p_abort);
            break;
        case I_BUTTON_CUSTOM:
            p_file->read_lendian_t(m_use_custom, p_abort);
            break;
        case I_BUTTON_CUSTOM_HOT:
            p_file->read_lendian_t(m_use_custom_hot, p_abort);
            break;
        case I_BUTTON_CUSTOM_DATA:
            m_custom_image.read_from_file(p_version, p_base, p_name, p_file, size, p_abort);
            break;
        case I_BUTTON_CUSTOM_HOT_DATA:
            m_custom_hot_image.read_from_file(p_version, p_base, p_name, p_file, size, p_abort);
            break;
        case I_BUTTON_USE_CUSTOM_TEXT:
            p_file->read_lendian_t(m_use_custom_text, p_abort);
            break;
        case I_BUTTON_TEXT: {
            pfc::array_t<char> name;
            name.set_size(size);
            p_file->read(name.get_ptr(), name.get_size(), p_abort);
            m_text.set_string(name.get_ptr(), name.get_size());
        } break;
        default:
            if (p_file->skip(size, p_abort) != size)
                throw exception_io_data_truncation();
            break;
        }
    }
}

void ButtonsToolbar::Button::write_to_file(stream_writer& p_file, bool b_paths, abort_callback& p_abort)
{
    p_file.write_lendian_t(I_BUTTON_TYPE, p_abort);
    p_file.write_lendian_t(sizeof(m_type), p_abort);
    p_file.write_lendian_t(m_type, p_abort);

    p_file.write_lendian_t(I_BUTTON_FILTER, p_abort);
    p_file.write_lendian_t(sizeof(m_filter), p_abort);
    p_file.write_lendian_t(m_filter, p_abort);

    p_file.write_lendian_t(I_BUTTON_SHOW, p_abort);
    p_file.write_lendian_t(sizeof(m_show), p_abort);
    p_file.write_lendian_t(m_show, p_abort);

    p_file.write_lendian_t(I_BUTTON_GUID, p_abort);
    p_file.write_lendian_t(sizeof(m_guid), p_abort);
    p_file.write_lendian_t((GUID&)m_guid, p_abort);

    p_file.write_lendian_t(I_BUTTON_SUBCOMMAND, p_abort);
    p_file.write_lendian_t(sizeof(m_subcommand), p_abort);
    p_file.write_lendian_t((GUID&)m_subcommand, p_abort);

    p_file.write_lendian_t(I_BUTTON_CUSTOM, p_abort);
    p_file.write_lendian_t(sizeof(m_use_custom), p_abort);
    p_file.write_lendian_t(m_use_custom, p_abort);

    if (m_use_custom) {
        p_file.write_lendian_t(I_BUTTON_CUSTOM_DATA, p_abort);

        stream_writer_memblock p_write;
        m_custom_image.write_to_file(p_write, b_paths, p_abort);
        p_file.write_lendian_t(p_write.m_data.get_size(), p_abort);
        p_file.write(p_write.m_data.get_ptr(), p_write.m_data.get_size(), p_abort);
    }

    p_file.write_lendian_t(I_BUTTON_CUSTOM_HOT, p_abort);
    p_file.write_lendian_t(sizeof(m_use_custom_hot), p_abort);
    p_file.write_lendian_t(m_use_custom_hot, p_abort);

    if (m_use_custom_hot) {
        p_file.write_lendian_t(I_BUTTON_CUSTOM_HOT_DATA, p_abort);
        stream_writer_memblock p_write;
        m_custom_hot_image.write_to_file(p_write, b_paths, p_abort);
        p_file.write_lendian_t(p_write.m_data.get_size(), p_abort);
        p_file.write(p_write.m_data.get_ptr(), p_write.m_data.get_size(), p_abort);
    }

    p_file.write_lendian_t(I_BUTTON_USE_CUSTOM_TEXT, p_abort);
    p_file.write_lendian_t(sizeof(m_use_custom_text), p_abort);
    p_file.write_lendian_t(m_use_custom_text, p_abort);

    if (m_use_custom_text) {
        p_file.write_lendian_t(I_BUTTON_TEXT, p_abort);
        p_file.write_lendian_t(m_text.length(), p_abort);
        p_file.write(m_text, m_text.length(), p_abort);
    }
}
