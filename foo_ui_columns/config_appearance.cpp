#include "stdafx.h"

#include "playlist_view.h"
#include "config_appearance.h"
#include "config_host.h"
#include "tab_colours.h"
#include "tab_fonts.h"

/*
* Fonts: 
*  Tabs - playlist tabs, tab stack
*  Lists - Playlist switcher, Playlist items, Playlist items group header, playlist column titles, filter panel
*  Other - Status bar, , , , 
* 
* Colours:
*  [[inactive] selected] text, [[inactive] selected] item background, 
* 
*/

#if 1

class appearance_message_window_t : public ui_helpers::container_window_autorelease_t
{
public:
    class_data & get_class_data() const override 
    {
        __implement_get_class_data_ex(_T("{BDCEC7A3-7230-4671-A5F7-B19A989DCA81}"), _T(""), false, 0, 0, 0, 0);
    }
    static void g_initialise();
    static bool g_initialised;
    LRESULT on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp) override;
};
bool appearance_message_window_t::g_initialised = false;
//pfc::rcptr_t<appearance_message_window_t> g_appearance_message_window;

void appearance_message_window_t::g_initialise()
    {
        if (!g_initialised)
        {
            auto  ptr = new appearance_message_window_t;
            ptr->create(HWND_MESSAGE);
            g_initialised = true;
        }
    }


colours_manager_data g_colours_manager_data;
fonts_manager_data g_fonts_manager_data;
tab_appearance g_tab_appearance;
tab_appearance_fonts g_tab_appearance_fonts;

class colours_manager_instance_impl : public cui::colours::manager_instance
{
public:
    colours_manager_instance_impl(const GUID & p_client_guid)
    {
        g_colours_manager_data.find_by_guid(p_client_guid, m_entry);
        g_colours_manager_data.find_by_guid(pfc::guid_null, m_global_entry);
    }
    COLORREF get_colour(const cui::colours::colour_identifier_t & p_identifier) const override
    {
        appearance_message_window_t::g_initialise();
        colours_manager_data::entry_ptr_t p_entry = m_entry->colour_mode == cui::colours::colour_mode_global ? m_global_entry : m_entry;
        if (p_entry->colour_mode == cui::colours::colour_mode_system || p_entry->colour_mode == cui::colours::colour_mode_themed)
            return g_get_system_color(p_identifier);
        switch (p_identifier)
        {
        case cui::colours::colour_text:
            return p_entry->text;
        case cui::colours::colour_selection_text:
            return p_entry->selection_text;
        case cui::colours::colour_background:
            return p_entry->background;
        case cui::colours::colour_selection_background:
            return p_entry->selection_background;
        case cui::colours::colour_inactive_selection_text:
            return p_entry->inactive_selection_text;
        case cui::colours::colour_inactive_selection_background:
            return p_entry->inactive_selection_background;
        case cui::colours::colour_active_item_frame:
            return p_entry->active_item_frame;
        default:
            return 0;
        }
    }
    bool get_bool(const cui::colours::bool_identifier_t & p_identifier) const override
    {
        colours_manager_data::entry_ptr_t p_entry = m_entry->colour_mode == cui::colours::colour_mode_global ? m_global_entry : m_entry;
        switch (p_identifier)
        {
        case cui::colours::bool_use_custom_active_item_frame:
            return p_entry->use_custom_active_item_frame;
        default:
            return false;
        }
    }
    bool get_themed() const override 
    {
        return m_entry->colour_mode == cui::colours::colour_mode_themed
            || (m_entry->colour_mode == cui::colours::colour_mode_global && m_global_entry->colour_mode == cui::colours::colour_mode_themed);
    }
private:
    colours_manager_data::entry_ptr_t m_entry;
    colours_manager_data::entry_ptr_t m_global_entry;
};

class colours_manager_impl : public cui::colours::manager
{
public:
    void create_instance (const GUID & p_client_guid, cui::colours::manager_instance::ptr & p_out) override
    {
        p_out = new service_impl_t<colours_manager_instance_impl>(p_client_guid);
    }
    void register_common_callback (cui::colours::common_callback * p_callback) override 
    {
        g_colours_manager_data.register_common_callback(p_callback);
    };
    void deregister_common_callback (cui::colours::common_callback * p_callback) override 
    {
        g_colours_manager_data.deregister_common_callback(p_callback);
    };
private:
};

class fonts_manager_impl : public cui::fonts::manager
{
public:
    void get_font(const GUID & p_guid, LOGFONT & p_out) const override
    {
        appearance_message_window_t::g_initialise();
        fonts_manager_data::entry_ptr_t p_entry;
        g_fonts_manager_data.find_by_guid(p_guid, p_entry);
        if (p_entry->font_mode == cui::fonts::font_mode_common_items)
            get_font(cui::fonts::font_type_items, p_out);
        else if (p_entry->font_mode == cui::fonts::font_mode_common_labels)
            get_font(cui::fonts::font_type_labels, p_out);
        else
        {
            p_out = p_entry->font;
        }
    }
    void get_font(const cui::fonts::font_type_t p_type, LOGFONT & p_out) const override
    {
        fonts_manager_data::entry_ptr_t p_entry;
        if (p_type == cui::fonts::font_type_items)
            p_entry = g_fonts_manager_data.m_common_items_entry;
        else
            p_entry = g_fonts_manager_data.m_common_labels_entry;

        if (p_entry->font_mode == cui::fonts::font_mode_system)
        {
            if (p_type == cui::fonts::font_type_items)
                uGetIconFont(&p_out);
            else
                uGetMenuFont(&p_out);
        }
        else
        {
            p_out = p_entry->font;
        }
    }
    void set_font(const GUID & p_guid, const LOGFONT & p_font) override
    {
        fonts_manager_data::entry_ptr_t p_entry;
        g_fonts_manager_data.find_by_guid(p_guid, p_entry);
        p_entry->font_mode = cui::fonts::font_mode_custom;
        p_entry->font = p_font;
        cui::fonts::client::ptr ptr;
        if (cui::fonts::client::create_by_guid(p_guid, ptr))
            ptr->on_font_changed();
    }
    void register_common_callback (cui::fonts::common_callback * p_callback) override 
    {
        g_fonts_manager_data.register_common_callback(p_callback);
    };
    void deregister_common_callback (cui::fonts::common_callback * p_callback) override 
    {
        g_fonts_manager_data.deregister_common_callback(p_callback);
    };
private:
};

namespace {
    service_factory_single_t<colours_manager_impl> g_colours_manager;
service_factory_t<fonts_manager_impl> g_fonts_manager;
};




cui::colours::colour_mode_t g_get_global_colour_mode() 
{    
    colours_manager_data::entry_ptr_t ptr;
    g_colours_manager_data.find_by_guid(pfc::guid_null, ptr);
    return ptr->colour_mode;
}

void g_set_global_colour_mode(cui::colours::colour_mode_t mode) 
{    
    colours_manager_data::entry_ptr_t ptr;
    g_colours_manager_data.find_by_guid(pfc::guid_null, ptr);
    if (ptr->colour_mode != mode)
    {
        ptr->colour_mode = mode;
        g_colours_manager_data.g_on_common_colour_changed(cui::colours::colour_flag_all);
        if (g_tab_appearance.is_active())
        {
            g_tab_appearance.update_mode_combobox();
            g_tab_appearance.update_fills();
        }
        colours_client_list_t m_colours_client_list;
        m_colours_client_list.g_get_list(m_colours_client_list);
        t_size i, count = m_colours_client_list.get_count();
        for (i=0; i<count; i++)
        {
            colours_manager_data::entry_ptr_t p_data;
            g_colours_manager_data.find_by_guid(m_colours_client_list[i].m_guid, p_data);
            if (p_data->colour_mode == cui::colours::colour_mode_global)
                m_colours_client_list[i].m_ptr->on_colour_changed(cui::colours::colour_flag_all);
        }
    }
}

void on_global_colours_change()
{
    if (g_tab_appearance.is_active())
    {
        g_tab_appearance.update_mode_combobox();
        g_tab_appearance.update_fills();
    }
    g_colours_manager_data.g_on_common_colour_changed(cui::colours::colour_flag_all);
    colours_client_list_t m_colours_client_list;
    m_colours_client_list.g_get_list(m_colours_client_list);
    t_size i, count = m_colours_client_list.get_count();
    for (i=0; i<count; i++)
    {
        colours_manager_data::entry_ptr_t p_data;
        g_colours_manager_data.find_by_guid(m_colours_client_list[i].m_guid, p_data);
        if (p_data->colour_mode == cui::colours::colour_mode_global)
            m_colours_client_list[i].m_ptr->on_colour_changed(cui::colours::colour_flag_all);
    }
}

namespace fonts
{
// {82196D79-69BC-4041-8E2A-E3B4406BB6FC}
    const GUID columns_playlist_items = 
    { 0x82196d79, 0x69bc, 0x4041, { 0x8e, 0x2a, 0xe3, 0xb4, 0x40, 0x6b, 0xb6, 0xfc } };

    // {B9D5EA18-5827-40be-A896-302A71BCAA9C}
    const GUID status_bar = 
    { 0xb9d5ea18, 0x5827, 0x40be, { 0xa8, 0x96, 0x30, 0x2a, 0x71, 0xbc, 0xaa, 0x9c } };

    // {C0D3B76C-324D-46d3-BB3C-E81C7D3BCB85}
    const GUID columns_playlist_header = 
    { 0xc0d3b76c, 0x324d, 0x46d3, { 0xbb, 0x3c, 0xe8, 0x1c, 0x7d, 0x3b, 0xcb, 0x85 } };

    // {19F8E0B3-E822-4f07-B200-D4A67E4872F9}
    const GUID ng_playlist_items = 
    { 0x19f8e0b3, 0xe822, 0x4f07, { 0xb2, 0x0, 0xd4, 0xa6, 0x7e, 0x48, 0x72, 0xf9 } };

    // {30FBD64C-2031-4f0b-A937-F21671A2E195}
    const GUID ng_playlist_header = 
    { 0x30fbd64c, 0x2031, 0x4f0b, { 0xa9, 0x37, 0xf2, 0x16, 0x71, 0xa2, 0xe1, 0x95 } };

    // {6F000FC4-3F86-4fc5-80EA-F7AA4D9551E6}
    const GUID splitter_tabs = 
    { 0x6f000fc4, 0x3f86, 0x4fc5, { 0x80, 0xea, 0xf7, 0xaa, 0x4d, 0x95, 0x51, 0xe6 } };

    // {942C36A4-4E28-4cea-9644-F223C9A838EC}
    const GUID playlist_tabs = 
    { 0x942c36a4, 0x4e28, 0x4cea, { 0x96, 0x44, 0xf2, 0x23, 0xc9, 0xa8, 0x38, 0xec } };

    // {70A5C273-67AB-4bb6-B61C-F7975A6871FD}
    const GUID playlist_switcher = 
    { 0x70a5c273, 0x67ab, 0x4bb6, { 0xb6, 0x1c, 0xf7, 0x97, 0x5a, 0x68, 0x71, 0xfd } };

    // {D93F1EF3-4AEE-4632-B5BF-0220CEC76DED}
    const GUID filter_items = 
    { 0xd93f1ef3, 0x4aee, 0x4632, { 0xb5, 0xbf, 0x2, 0x20, 0xce, 0xc7, 0x6d, 0xed } };

    // {FCA8752B-C064-41c4-9BE3-E125C7C7FC34}
    const GUID filter_header = 
    { 0xfca8752b, 0xc064, 0x41c4, { 0x9b, 0xe3, 0xe1, 0x25, 0xc7, 0xc7, 0xfc, 0x34 } };
}

void refresh_appearance_prefs()
{
    if (g_tab_appearance_fonts.is_active())
    {
        g_tab_appearance_fonts.update_mode_combobox();
        g_tab_appearance_fonts.update_font_desc();
        g_tab_appearance_fonts.update_change();
    }
}

static preferences_tab * g_tabs_appearance[] = 
{
    &g_tab_appearance, &g_tab_appearance_fonts
};

// {FA25D859-C808-485d-8AB7-FCC10F29ECE5}
const GUID g_guid_cfg_child_appearance = 
{ 0xfa25d859, 0xc808, 0x485d, { 0x8a, 0xb7, 0xfc, 0xc1, 0xf, 0x29, 0xec, 0xe5 } };

cfg_int cfg_child_appearance(g_guid_cfg_child_appearance,0);

// {41E6D7ED-A1DC-4d84-9BC9-352DAF7788B0}
constexpr const GUID g_guid_colour_preferences =
{ 0x41e6d7ed, 0xa1dc, 0x4d84,{ 0x9b, 0xc9, 0x35, 0x2d, 0xaf, 0x77, 0x88, 0xb0 } };

static service_factory_single_t<config_host_generic> g_config_tabs("Colours and Fonts", g_tabs_appearance, tabsize(g_tabs_appearance), g_guid_colour_preferences, g_guid_columns_ui_preferences_page, &cfg_child_appearance);


class fcl_colours_t : public cui::fcl::dataset
{
    enum {stream_version=0};
    void get_name (pfc::string_base & p_out) const override
    {
        p_out = "Colours (unified)";
    }
    const GUID & get_group () const override
    {
        return cui::fcl::groups::colours_and_fonts;
    }
    const GUID & get_guid () const override
    {
        // {165946E7-6165-4680-A08E-84B5768458E8}
        static const GUID guid = 
        { 0x165946e7, 0x6165, 0x4680, { 0xa0, 0x8e, 0x84, 0xb5, 0x76, 0x84, 0x58, 0xe8 } };
        return guid;
    }
    enum identifiers_t 
    {
        identifier_global_entry,
        identifier_client_entries,
        identifier_client_entry=0,
    };
    void get_data (stream_writer * p_writer, t_uint32 type, cui::fcl::t_export_feedback & feedback, abort_callback & p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);
        //p_writer->write_lendian_t(stream_version, p_abort);
        {
            stream_writer_memblock mem;
            g_colours_manager_data.m_global_entry->_export(&mem, p_abort);
            out.write_item(identifier_global_entry, mem.m_data.get_ptr(), mem.m_data.get_size());
        }
        {
            stream_writer_memblock mem;
            fbh::fcl::Writer out2(&mem, p_abort);
            t_size i, count = g_colours_manager_data.m_entries.get_count();
            mem.write_lendian_t(count, p_abort);
            for (i=0; i<count; i++)
            {
                stream_writer_memblock mem2;
                g_colours_manager_data.m_entries[i]->_export(&mem2, p_abort);
                out2.write_item(identifier_client_entry, mem2.m_data.get_ptr(), mem2.m_data.get_size());
            }
            out.write_item(identifier_client_entries, mem.m_data.get_ptr(), mem.m_data.get_size());
        }
    }
    void set_data (stream_reader * p_reader, t_size stream_size, t_uint32 type, cui::fcl::t_import_feedback & feedback, abort_callback & p_abort) override
    {
        fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
        t_uint32 element_id;
        t_uint32 element_size;

        while (reader.get_remaining())
        {
            reader.read_item(element_id);
            reader.read_item(element_size);

            pfc::array_t<t_uint8> data;
            data.set_size(element_size);
            reader.read(data.get_ptr(), data.get_size());
            

            switch (element_id)
            {
            case identifier_global_entry:
                {
                    stream_reader_memblock_ref colour_reader(data);
                    g_colours_manager_data.m_global_entry->import(&colour_reader, data.get_size(), type, p_abort);
                }
                break;
            case identifier_client_entries:
                {
                    stream_reader_memblock_ref stream2(data);
                    fbh::fcl::Reader reader2(&stream2, data.get_size(), p_abort);

                    t_size count, i;
                    reader2.read_item(count);

                    g_colours_manager_data.m_entries.remove_all();
                    g_colours_manager_data.m_entries.set_count(count);

                    for (i=0; i<count; i++)
                    {
                        t_uint32 element_id2;
                        t_uint32 element_size2;
                        reader2.read_item(element_id2);
                        reader2.read_item(element_size2);
                        if (element_id2 == identifier_client_entry)
                        {
                            pfc::array_t<t_uint8> data2;
                            data2.set_size(element_size2);
                            reader2.read(data2.get_ptr(), data2.get_size());
                            stream_reader_memblock_ref colour_reader(data2);
                            g_colours_manager_data.m_entries[i] = new colours_manager_data::entry_t;
                            g_colours_manager_data.m_entries[i]->import(&colour_reader, data2.get_size(), type, p_abort);
                        }
                        else
                            reader2.skip(element_size2);
                    }
                }
                break;
            default:
                reader.skip(element_size);
                break;
            };
        }
        if (g_tab_appearance.is_active())
        {
            g_tab_appearance.update_mode_combobox();
            g_tab_appearance.update_fills();
        }
        g_colours_manager_data.g_on_common_colour_changed(cui::colours::colour_flag_all);
        service_enum_t<cui::colours::client> colour_enum;
        cui::colours::client::ptr ptr;
        while (colour_enum.next(ptr))
            ptr->on_colour_changed(cui::colours::colour_flag_all);
    }
};

namespace {
service_factory_t<fcl_colours_t> g_fcl_colours_t;
};

class fcl_fonts_t : public cui::fcl::dataset
{
    enum {stream_version=0};
    void get_name (pfc::string_base & p_out) const override
    {
        p_out = "Fonts (unified)";
    }
    const GUID & get_group () const override
    {
        return cui::fcl::groups::colours_and_fonts;
    }
    const GUID & get_guid () const override
    {
        // {A806A9CD-4117-43da-805E-FE4EB348C90C}
        static const GUID guid = 
        { 0xa806a9cd, 0x4117, 0x43da, { 0x80, 0x5e, 0xfe, 0x4e, 0xb3, 0x48, 0xc9, 0xc } };
        return guid;
    }
    enum identifiers_t 
    {
        identifier_global_items,
        identifier_global_labels,
        identifier_client_entries,
        identifier_client_entry=0,
    };
    void get_data (stream_writer * p_writer, t_uint32 type, cui::fcl::t_export_feedback & feedback, abort_callback & p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);
        //p_writer->write_lendian_t(stream_version, p_abort);
        {
            stream_writer_memblock mem;
            g_fonts_manager_data.m_common_items_entry->_export(&mem, p_abort);
            out.write_item(identifier_global_items, mem.m_data.get_ptr(), mem.m_data.get_size());
        }
        {
            stream_writer_memblock mem;
            g_fonts_manager_data.m_common_labels_entry->_export(&mem, p_abort);
            out.write_item(identifier_global_labels, mem.m_data.get_ptr(), mem.m_data.get_size());
        }
        {
            stream_writer_memblock mem;
            fbh::fcl::Writer out2(&mem, p_abort);
            t_size i, count = g_fonts_manager_data.m_entries.get_count();
            mem.write_lendian_t(count, p_abort);
            for (i=0; i<count; i++)
            {
                stream_writer_memblock mem2;
                g_fonts_manager_data.m_entries[i]->_export(&mem2, p_abort);
                out2.write_item(identifier_client_entry, mem2.m_data.get_ptr(), mem2.m_data.get_size());
            }
            out.write_item(identifier_client_entries, mem.m_data.get_ptr(), mem.m_data.get_size());
        }
    }
    void set_data (stream_reader * p_reader, t_size stream_size, t_uint32 type, cui::fcl::t_import_feedback & feedback, abort_callback & p_abort) override
    {
        fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
        t_uint32 element_id;
        t_uint32 element_size;

        while (reader.get_remaining())
        {
            reader.read_item(element_id);
            reader.read_item(element_size);

            pfc::array_t<t_uint8> data;
            data.set_size(element_size);
            reader.read(data.get_ptr(), data.get_size());

            stream_reader_memblock_ref data_reader(data);

            switch (element_id)
            {
            case identifier_global_items:
                g_fonts_manager_data.m_common_items_entry->import(&data_reader, data.get_size(), type, p_abort);
                break;
            case identifier_global_labels:
                g_fonts_manager_data.m_common_labels_entry->import(&data_reader, data.get_size(), type, p_abort);
                break;
            case identifier_client_entries:
                {
                    fbh::fcl::Reader reader2(&data_reader, data.get_size(), p_abort);

                    t_size count, i;
                    reader2.read_item(count);

                    g_fonts_manager_data.m_entries.remove_all();
                    g_fonts_manager_data.m_entries.set_count(count);

                    for (i=0; i<count; i++)
                    {
                        t_uint32 element_id2;
                        t_uint32 element_size2;
                        reader2.read_item(element_id2);
                        reader2.read_item(element_size2);
                        if (element_id2 == identifier_client_entry)
                        {
                            pfc::array_t<t_uint8> data2;
                            data2.set_size(element_size2);
                            reader2.read(data2.get_ptr(), data2.get_size());
                            stream_reader_memblock_ref element_reader(data2);
                            g_fonts_manager_data.m_entries[i] = new fonts_manager_data::entry_t;
                            g_fonts_manager_data.m_entries[i]->import(&element_reader, data2.get_size(), type, p_abort);
                        }
                        else
                            reader2.skip(element_size2);
                    }
                }
                break;
            default:
                reader.skip(element_size);
                break;
            };
        }
        refresh_appearance_prefs();
        g_fonts_manager_data.g_on_common_font_changed(pfc_infinite);
        service_enum_t<cui::fonts::client> font_enum;
        cui::fonts::client::ptr ptr;
        while (font_enum.next(ptr))
            ptr->on_font_changed();
    }
};

namespace {
service_factory_t<fcl_fonts_t> g_fcl_fonts_t;
};

// {15FD4FF9-0622-4077-BFBB-DF0102B6A068}
const GUID colours_manager_data::g_cfg_guid = 
{ 0x15fd4ff9, 0x622, 0x4077, { 0xbf, 0xbb, 0xdf, 0x1, 0x2, 0xb6, 0xa0, 0x68 } };

// {6B71F91C-6B7E-4dbe-B27B-C493AA513FD0}
const GUID fonts_manager_data::g_cfg_guid = 
{ 0x6b71f91c, 0x6b7e, 0x4dbe, { 0xb2, 0x7b, 0xc4, 0x93, 0xaa, 0x51, 0x3f, 0xd0 } };

LRESULT appearance_message_window_t::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
    {
        switch (msg)
        {
        case WM_SYSCOLORCHANGE:
            {
                colours_client_list_t m_colours_client_list;
                m_colours_client_list.g_get_list(m_colours_client_list);
                t_size i, count = m_colours_client_list.get_count();
                bool b_global_custom = g_colours_manager_data.m_global_entry->colour_mode == cui::colours::colour_mode_custom;
                if (!b_global_custom)
                    g_colours_manager_data.g_on_common_colour_changed(cui::colours::colour_flag_all);
                for (i=0; i<count; i++)
                {
                    colours_manager_data::entry_ptr_t p_data;
                    g_colours_manager_data.find_by_guid(m_colours_client_list[i].m_guid, p_data);
                    if (p_data->colour_mode == cui::colours::colour_mode_system 
                            || p_data->colour_mode == cui::colours::colour_mode_themed
                            || (p_data->colour_mode == cui::colours::colour_mode_global && !b_global_custom)) {
                        m_colours_client_list[i].m_ptr->on_colour_changed(cui::colours::colour_flag_all);
                    }
                }
            }
            break;
        case WM_SETTINGCHANGE:
            if (
                (wp == SPI_GETICONTITLELOGFONT && g_fonts_manager_data.m_common_items_entry.is_valid() && g_fonts_manager_data.m_common_items_entry->font_mode == cui::fonts::font_mode_system)
                || (wp == SPI_GETNONCLIENTMETRICS && g_fonts_manager_data.m_common_labels_entry.is_valid() && g_fonts_manager_data.m_common_labels_entry->font_mode == cui::fonts::font_mode_system))
            {

                fonts_client_list_t m_fonts_client_list;
                m_fonts_client_list.g_get_list(m_fonts_client_list);
                t_size i, count = m_fonts_client_list.get_count();
                g_fonts_manager_data.g_on_common_font_changed(wp == SPI_GETICONTITLELOGFONT ? cui::fonts::font_type_flag_items : cui::fonts::font_type_flag_labels);
                for (i=0; i<count; i++)
                {
                    fonts_manager_data::entry_ptr_t p_data;
                    g_fonts_manager_data.find_by_guid(m_fonts_client_list[i].m_guid, p_data);
                    if (wp == SPI_GETNONCLIENTMETRICS && p_data->font_mode == cui::fonts::font_mode_common_items)
                        m_fonts_client_list[i].m_ptr->on_font_changed();
                    else if (wp == SPI_GETICONTITLELOGFONT && p_data->font_mode == cui::fonts::font_mode_common_labels)
                        m_fonts_client_list[i].m_ptr->on_font_changed();
                }
            }
            break;
        case WM_CLOSE:
            destroy();
            delete this;
            return 0;
        };
        return DefWindowProc(wnd, msg, wp, lp);
    }

#endif