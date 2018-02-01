#include "stdafx.h"
#include "item_properties.h"

// {8F6069CD-2E36-4ead-B171-93F3DFF0073A}
static const GUID g_guid_selection_properties
    = {0x8f6069cd, 0x2e36, 0x4ead, {0xb1, 0x71, 0x93, 0xf3, 0xdf, 0xf0, 0x7, 0x3a}};

// {02570710-E204-4077-AEAE-5A00D6A2AC08}
static const GUID g_guid_selection_properties_tracking_mode
    = {0x2570710, 0xe204, 0x4077, {0xae, 0xae, 0x5a, 0x0, 0xd6, 0xa2, 0xac, 0x8}};

// {755FBB3D-A8D4-46f3-B0BA-005B0A10A01A}
static const GUID g_guid_selection_properties_items_font_client
    = {0x755fbb3d, 0xa8d4, 0x46f3, {0xb0, 0xba, 0x0, 0x5b, 0xa, 0x10, 0xa0, 0x1a}};

// {7B9DF268-4ECC-4e10-A308-E145DA9692A5}
static const GUID g_guid_selection_properties_header_font_client
    = {0x7b9df268, 0x4ecc, 0x4e10, {0xa3, 0x8, 0xe1, 0x45, 0xda, 0x96, 0x92, 0xa5}};

// {AF5A96A6-96ED-468f-8BA1-C22533C53491}
static const GUID g_guid_selection_properties_group_font_client
    = {0xaf5a96a6, 0x96ed, 0x468f, {0x8b, 0xa1, 0xc2, 0x25, 0x33, 0xc5, 0x34, 0x91}};

// {1D921F23-1708-451a-A02E-C6657F7C4386}
static const GUID g_guid_selection_poperties_edge_style
    = {0x1d921f23, 0x1708, 0x451a, {0xa0, 0x2e, 0xc6, 0x65, 0x7f, 0x7c, 0x43, 0x86}};

// {C314F956-71AD-4d75-8749-5882508F6904}
static const GUID g_guid_selection_poperties_info_sections
    = {0xc314f956, 0x71ad, 0x4d75, {0x87, 0x49, 0x58, 0x82, 0x50, 0x8f, 0x69, 0x4}};

// {9AE7CE9F-7DA8-4115-A895-8D519A039325}
static const GUID g_guid_selection_poperties_show_column_titles
    = {0x9ae7ce9f, 0x7da8, 0x4115, {0xa8, 0x95, 0x8d, 0x51, 0x9a, 0x3, 0x93, 0x25}};

// {B84886A5-4510-42d0-837A-33E55360C24E}
static const GUID g_guid_selection_poperties_show_group_titles
    = {0xb84886a5, 0x4510, 0x42d0, {0x83, 0x7a, 0x33, 0xe5, 0x53, 0x60, 0xc2, 0x4e}};

cfg_uint cfg_selection_properties_tracking_mode(g_guid_selection_properties_tracking_mode, 0);
cfg_uint cfg_selection_properties_edge_style(g_guid_selection_poperties_edge_style, 2);
cfg_uint cfg_selection_properties_info_sections(g_guid_selection_poperties_info_sections, 1 + 2 + 4 + 8 + 16);
cfg_bool cfg_selection_poperties_show_column_titles(g_guid_selection_poperties_show_column_titles, true);
cfg_bool cfg_selection_poperties_show_group_titles(g_guid_selection_poperties_show_group_titles, true);

const info_section_t g_info_sections[] = {info_section_t(0, "Location"), info_section_t(1, "General"),
    info_section_t(2, "ReplayGain"), info_section_t(3, "Playback statistics"), info_section_t(4, "Other")};

t_size g_get_info_secion_index_by_name(const char* p_name)
{
    t_size count = tabsize(g_info_sections);
    for (t_size i = 0; i < count; i++) {
        if (!stricmp_utf8(p_name, g_info_sections[i].name))
            return i;
    }
    return 4;
}

t_size g_get_info_secion_index_by_id(t_size id)
{
    t_size count = tabsize(g_info_sections);
    for (t_size i = 0; i < count; i++) {
        if (g_info_sections[i].id == id)
            return i;
    }
    return pfc_infinite;
}

#if 0
class track_property_callback_getgroups : public track_property_callback_v2 {
public:
    virtual void set_property(const char * p_group,double p_sortpriority,const char * p_name,const char * p_value)
    {
    }
    virtual bool is_group_wanted(const char * p_group)
    {
        m_groups.add_item(p_group);
        return false;
    }
    pfc::list_t< pfc::string8 > m_groups;
};
#endif

selection_properties_t::message_window_t selection_properties_t::g_message_window;

std::vector<selection_properties_t*> selection_properties_t::g_windows;

// {862F8A37-16E0-4a74-B27E-2B73DB567D0F}
const GUID appearance_client_selection_properties_impl::g_guid
    = {0x862f8a37, 0x16e0, 0x4a74, {0xb2, 0x7e, 0x2b, 0x73, 0xdb, 0x56, 0x7d, 0xf}};

namespace {
cui::colours::client::factory<appearance_client_selection_properties_impl> g_appearance_client_impl;
};

void selection_properties_t::g_redraw_all()
{
    for (auto& window : g_windows)
        window->invalidate_all();
}

selection_properties_t::message_window_t::class_data& selection_properties_t::message_window_t::get_class_data() const
{
    __implement_get_class_data_ex(_T("{9D0A0408-59AC-4a96-A3EF-FF26B7B7C118}"), _T(""), false, 0, 0, 0, 0);
}

LRESULT selection_properties_t::message_window_t::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE:
        break;
    case WM_ACTIVATEAPP:
        selection_properties_t::g_on_app_activate(wp != 0);
        break;
    case WM_DESTROY:
        break;
    }
    return DefWindowProc(wnd, msg, wp, lp);
}

void selection_properties_t::g_on_app_activate(bool b_activated)
{
    for (auto& window : g_windows)
        window->on_app_activate(b_activated);
}
void selection_properties_t::on_app_activate(bool b_activated)
{
    if (b_activated) {
        if (GetFocus() != get_wnd())
            register_callback();
    } else {
        deregister_callback();
    }
}

const GUID& selection_properties_t::get_extension_guid() const
{
    return g_guid_selection_properties;
}
void selection_properties_t::get_name(pfc::string_base& p_out) const
{
    p_out = "Item properties";
}
void selection_properties_t::get_category(pfc::string_base& p_out) const
{
    p_out = "Panels";
}
unsigned selection_properties_t::get_type() const
{
    return uie::type_panel;
}

void selection_properties_t::set_config(stream_reader* p_reader, t_size p_size, abort_callback& p_abort)
{
    if (p_size) {
        t_size version;
        p_reader->read_lendian_t(version, p_abort);
        if (version <= config_version_current) {
            t_size field_count;
            p_reader->read_lendian_t(field_count, p_abort);
            m_fields.remove_all();
            m_fields.set_count(field_count);
            for (t_size i = 0; i < field_count; i++) {
                p_reader->read_string(m_fields[i].m_name_friendly, p_abort);
                p_reader->read_string(m_fields[i].m_name, p_abort);
            }
            p_reader->read_lendian_t(m_tracking_mode, p_abort);
            p_reader->read_lendian_t(m_autosizing_columns, p_abort);
            p_reader->read_lendian_t(m_column_name_width, p_abort);
            p_reader->read_lendian_t(m_column_field_width, p_abort);

            if (version >= 3) {
                p_reader->read_lendian_t(m_edge_style, p_abort);
                if (version >= 4) {
                    p_reader->read_lendian_t(m_info_sections_mask, p_abort);
                    p_reader->read_lendian_t(m_show_column_titles, p_abort);
                    p_reader->read_lendian_t(m_show_group_titles, p_abort);
                }
            }
        }
    }
}
void selection_properties_t::get_config(stream_writer* p_writer, abort_callback& p_abort) const
{
    p_writer->write_lendian_t(t_size(config_version_current), p_abort);
    t_size count = m_fields.get_count();
    p_writer->write_lendian_t(count, p_abort);
    for (t_size i = 0; i < count; i++) {
        p_writer->write_string(m_fields[i].m_name_friendly, p_abort);
        p_writer->write_string(m_fields[i].m_name, p_abort);
    }
    p_writer->write_lendian_t(m_tracking_mode, p_abort);
    p_writer->write_lendian_t(m_autosizing_columns, p_abort);
    p_writer->write_lendian_t(m_column_name_width, p_abort);
    p_writer->write_lendian_t(m_column_field_width, p_abort);
    p_writer->write_lendian_t(m_edge_style, p_abort);

    p_writer->write_lendian_t(m_info_sections_mask, p_abort);
    p_writer->write_lendian_t(m_show_column_titles, p_abort);
    p_writer->write_lendian_t(m_show_group_titles, p_abort);
}

void selection_properties_t::notify_on_initialisation()
{
    set_autosize(m_autosizing_columns);
    LOGFONT lf;
    static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_selection_properties_items_font_client, lf);
    set_font(&lf);
    static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_selection_properties_header_font_client, lf);
    set_header_font(&lf);
    static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_selection_properties_group_font_client, lf);
    set_group_font(&lf);
    set_edge_style(m_edge_style);
    set_show_header(m_show_column_titles);
    set_group_level_indentation_enabled(false);
}
void selection_properties_t::notify_on_create()
{
    pfc::list_t<Column> columns;
    columns.add_item(Column("Field", m_column_name_width, 0));
    columns.add_item(Column("Value", m_column_field_width, 1));
    set_columns(columns);
    set_group_count(m_show_group_titles ? 1 : 0);

    register_callback();
    static_api_ptr_t<play_callback_manager>()->register_callback(
        this, play_callback::flag_on_playback_stop | play_callback::flag_on_playback_new_track, true);
    static_api_ptr_t<metadb_io_v3>()->register_callback(this);
    refresh_contents();

    if (g_windows.empty())
        g_message_window.create(nullptr);
    g_windows.push_back(this);
}
void selection_properties_t::notify_on_destroy()
{
    g_windows.erase(std::remove(g_windows.begin(), g_windows.end(), this), g_windows.end());
    if (g_windows.empty())
        g_message_window.destroy();

    static_api_ptr_t<play_callback_manager>()->unregister_callback(this);
    static_api_ptr_t<metadb_io_v3>()->unregister_callback(this);
    deregister_callback();
    m_handles.remove_all();
    m_selection_handles.remove_all();
    m_edit_handles.remove_all();
    m_selection_holder.release();
}

void selection_properties_t::notify_on_set_focus(HWND wnd_lost)
{
    deregister_callback();
    m_selection_holder = static_api_ptr_t<ui_selection_manager>()->acquire();
    m_selection_holder->set_selection(m_handles);
}
void selection_properties_t::notify_on_kill_focus(HWND wnd_receiving)
{
    m_selection_holder.release();
    register_callback();

    /*if (wnd_receiving == NULL)
    register_callback();
    else
    {
    DWORD processid = NULL;
    GetWindowThreadProcessId (wnd_receiving, &processid);
    if (processid == NULL || processid == GetCurrentProcessId())
    register_callback();
    }*/
}

void selection_properties_t::register_callback()
{
    if (!m_callback_registered)
        g_ui_selection_manager_register_callback_no_now_playing_fallback(this);
    m_callback_registered = true;
}
void selection_properties_t::deregister_callback()
{
    if (m_callback_registered)
        static_api_ptr_t<ui_selection_manager>()->unregister_callback(this);
    m_callback_registered = false;
}

class metadata_aggregator_t {
public:
    enum { max_values = 16 };
    class metadata_field_t {
    public:
        pfc::string8 m_name;
        pfc::list_t<pfc::string8> m_values;
        bool m_truncated{false};

        /*static g_compare_entry (const char * str1, const pfc::string8 & str2)
        {
        }*/

        void add_value(const char* p_value)
        {
            t_size index;
            if (!m_values.bsearch_t(stricmp_utf8, p_value, index)) {
                if (m_values.get_count() < max_values)
                    m_values.insert_item(p_value, index);
                else
                    m_truncated = true;
            }
        }

        metadata_field_t(const char* field) : m_name(field){};
        metadata_field_t() = default;
    };

    static int g_compare_field(const metadata_field_t& str2, const char* str1)
    {
        return stricmp_utf8(str2.m_name, str1);
    }

    void add_field(const char* p_field, const char* p_value)
    {
        t_size index;
        if (!m_fields.bsearch_t(g_compare_field, p_field, index)) {
            // m_fields.insert_item(metadata_field_t(p_field), index);
        } else
            m_fields[index].add_value(p_value);
    }

    void process_file_info(const file_info* p_info)
    {
        t_size count_field = p_info->meta_get_count();
        for (t_size index_field = 0; index_field < count_field; index_field++) {
            const char* p_field = p_info->meta_enum_name(index_field);
            t_size value_count = p_info->meta_enum_value_count(index_field);
            for (t_size index_value = 0; index_value < value_count; index_value++) {
                add_field(p_field, p_info->meta_enum_value(index_field, index_value));
            }
        }
    }

#if 0
    void process_track_properties (const metadb_handle_list & tracks)
    {
        {
            track_property_callback_itemproperties props;
            track_property_provider::ptr ptr;
            service_enum_t<track_property_provider> e;
            while (e.next(ptr))
                ptr->enumerate_properties(tracks,props);

            t_size index_field, count_field = m_fields.get_count();
            for (index_field=0; index_field<count_field; index_field++)
            {
                t_size index;
                if (props.find_field(m_fields[index_field].m_name, index))
                {
                    m_fields[index_field].add_value(props.m_values[index].m_value);
                }
            }
        }
    }
#endif

    void process_file_info_v2(const file_info* p_info)
    {
        t_size count_field = m_fields.get_count();
        for (t_size index_field = 0; index_field < count_field; index_field++) {
            t_size index_field_meta = p_info->meta_find(m_fields[index_field].m_name);
            if (index_field_meta != pfc_infinite) {
                t_size value_count = p_info->meta_enum_value_count(index_field_meta);
                for (t_size index_value = 0; index_value < value_count; index_value++) {
                    m_fields[index_field].add_value(p_info->meta_enum_value(index_field_meta, index_value));
                }
            } else {
                /*t_size index_field_info = p_info->info_find(m_fields[index_field].m_name);
                if (index_field_info != pfc_infinite)
                {
                    m_fields[index_field].add_value(p_info->info_enum_value(index_field_info));
                }*/
            }
        }
    }
    void set_fields(pfc::list_t<field_t>& p_source)
    {
        t_size count = p_source.get_count();
        m_fields.set_count(count);
        for (t_size i = 0; i < count; i++)
            m_fields[i].m_name = p_source[i].m_name;
    }
    pfc::list_t<metadata_field_t> m_fields;

private:
};

void selection_properties_t::refresh_contents()
{
    bool b_redraw = disable_redrawing();

    metadata_aggregator_t metadata_aggregator;
    pfc::list_t<uih::ListView::InsertItem> items;
    t_size i, count = m_handles.get_count();
    metadata_aggregator.set_fields(m_fields);
    {
        // metadata_aggregator.process_track_properties(m_handles);
    } {
        for (i = 0; i < count; i++) {
            metadb_info_container::ptr p_info;
            if (m_handles[i]->get_info_ref(p_info)) {
                metadata_aggregator.process_file_info_v2(&p_info->info());
            }
        }
    }
    {
        count = metadata_aggregator.m_fields.get_count();
        for (i = 0; i < count; i++) {
            uih::ListView::InsertItem item(2, 1);
            pfc::string8 temp;
            item.m_subitems[0] = m_fields[i].m_name_friendly;
            temp.reset();
            t_size count_values = metadata_aggregator.m_fields[i].m_values.get_count();
            for (t_size j = 0; j < count_values; j++) {
                temp << metadata_aggregator.m_fields[i].m_values[j];
                if (j + 1 != count_values)
                    temp << "; ";
            }
            if (metadata_aggregator.m_fields[i].m_truncated)
                temp << "; "
                        "\xe2\x80\xa6";
            item.m_subitems[1] = temp;
            item.m_groups[0] = "Metadata";
            items.add_item(item);
        }
    }

    {
        track_property_callback_itemproperties props;
        track_property_provider::ptr ptr;
        if (m_handles.get_count()) {
            service_enum_t<track_property_provider> e;
            while (e.next(ptr))
                ptr->enumerate_properties(m_handles, props);
        }

        t_size count_group = props.m_values.get_size();
        for (t_size index_group = 0; index_group < count_group; index_group++) {
            t_size count_field = props.m_values[index_group].get_count();
            for (t_size index_field = 0; index_field < count_field; index_field++) {
                if (m_info_sections_mask & (1 << (g_info_sections[index_group].id))) {
                    uih::ListView::InsertItem item(2, 1);
                    item.m_subitems[0] = props.m_values[index_group][index_field].m_name;
                    item.m_subitems[1] = props.m_values[index_group][index_field].m_value;
                    item.m_groups[0] = g_info_sections[index_group].name;
                    items.add_item(item);
                }
            }
        }
    }

    t_size old_count = get_item_count(), new_count = items.get_count();

    if (new_count && old_count) {
        pfc::list_t<uih::ListView::InsertItem> items_replace;
        items_replace.add_items_fromptr(items.get_ptr(), min(new_count, old_count));
        uih::ListView::replace_items(0, items_replace, false);
    }

    if (new_count > old_count) {
        uih::ListView::insert_items(old_count, items.get_count() - old_count, items.get_ptr() + old_count, false);
    } else if (new_count < old_count) {
        uih::ListView::remove_items(pfc::bit_array_range(new_count, old_count - new_count), false);
    }

    if (b_redraw)
        enable_redrawing();
}

void selection_properties_t::on_playback_new_track(metadb_handle_ptr p_track)
{
    if (m_tracking_mode == track_nowplaying || m_tracking_mode == track_automatic) {
        m_handles.remove_all();
        m_handles.add_item(p_track);
        refresh_contents();
    }
}

void selection_properties_t::on_playback_stop(play_control::t_stop_reason p_reason)
{
    if (p_reason != play_control::stop_reason_starting_another && p_reason != play_control::stop_reason_shutting_down) {
        if (m_tracking_mode == track_nowplaying || m_tracking_mode == track_automatic) {
            if (m_tracking_mode == track_automatic)
                m_handles = m_selection_handles;
            else
                m_handles.remove_all();
            refresh_contents();
        }
    }
}

void selection_properties_t::on_changed_sorted(metadb_handle_list_cref p_items_sorted, bool p_fromhook)
{
    if (!p_fromhook) {
        bool b_refresh = false;
        t_size count = m_handles.get_count();
        for (t_size i = 0; i < count && !b_refresh; i++) {
            t_size index = pfc_infinite;
            if (p_items_sorted.bsearch_t(pfc::compare_t<metadb_handle_ptr, metadb_handle_ptr>, m_handles[i], index))
                b_refresh = true;
        }
        if (b_refresh) {
            refresh_contents();
        }
    }
}

bool selection_properties_t::check_process_on_selection_changed()
{
    HWND wnd_focus = GetFocus();
    if (wnd_focus == nullptr)
        return false;

    DWORD processid = NULL;
    GetWindowThreadProcessId(wnd_focus, &processid);
    return processid == GetCurrentProcessId();
}

void selection_properties_t::on_selection_changed(const pfc::list_base_const_t<metadb_handle_ptr>& p_selection)
{
    if (check_process_on_selection_changed()) {
        if (g_ui_selection_manager_is_now_playing_fallback())
            m_selection_handles.remove_all();
        else
            m_selection_handles = p_selection;

        if (m_tracking_mode == track_nowplaying
            || (m_tracking_mode == track_automatic && static_api_ptr_t<play_control>()->is_playing()))
            return;

        m_handles = m_selection_handles;
        refresh_contents();
    }

    // pfc::hires_timer timer;
    // timer.start();

    // console::formatter() << "Selection properties panel refreshed in: " << timer.query() << " seconds";
}

void selection_properties_t::on_tracking_mode_change()
{
    m_handles.remove_all();
    if (m_tracking_mode == track_selection
        || (m_tracking_mode == track_automatic && !static_api_ptr_t<play_control>()->is_playing())) {
        m_handles = m_selection_handles;
    } else if (m_tracking_mode == track_nowplaying
        || (m_tracking_mode == track_automatic && static_api_ptr_t<play_control>()->is_playing())) {
        metadb_handle_ptr item;
        if (static_api_ptr_t<playback_control>()->get_now_playing(item))
            m_handles.add_item(item);
    }
    refresh_contents();
}

class font_client_selection_properties : public cui::fonts::client {
public:
    const GUID& get_client_guid() const override { return g_guid_selection_properties_items_font_client; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Item properties: Items"; }

    cui::fonts::font_type_t get_default_font_type() const override { return cui::fonts::font_type_items; }

    void on_font_changed() const override { selection_properties_t::g_on_font_items_change(); }
};

class font_header_client_selection_properties : public cui::fonts::client {
public:
    const GUID& get_client_guid() const override { return g_guid_selection_properties_header_font_client; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Item properties: Column titles"; }

    cui::fonts::font_type_t get_default_font_type() const override { return cui::fonts::font_type_items; }

    void on_font_changed() const override { selection_properties_t::g_on_font_header_change(); }
};

class font_group_client_selection_properties : public cui::fonts::client {
public:
    const GUID& get_client_guid() const override { return g_guid_selection_properties_group_font_client; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Item Properties: Group titles"; }

    cui::fonts::font_type_t get_default_font_type() const override { return cui::fonts::font_type_items; }

    void on_font_changed() const override { selection_properties_t::g_on_font_groups_change(); }
};
void selection_properties_t::g_on_font_items_change()
{
    LOGFONT lf;
    static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_selection_properties_items_font_client, lf);
    for (auto& window : g_windows) {
        window->set_font(&lf);
    }
}

void selection_properties_t::g_on_font_groups_change()
{
    LOGFONT lf;
    static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_selection_properties_group_font_client, lf);
    for (auto& window : g_windows) {
        window->set_group_font(&lf);
    }
}

void selection_properties_t::g_on_font_header_change()
{
    LOGFONT lf;
    static_api_ptr_t<cui::fonts::manager>()->get_font(g_guid_selection_properties_header_font_client, lf);
    for (auto& window : g_windows) {
        window->set_header_font(&lf);
    }
}

selection_properties_t::selection_properties_t()
    : m_tracking_mode(cfg_selection_properties_tracking_mode)
    , m_info_sections_mask(cfg_selection_properties_info_sections)
    , m_show_column_titles(cfg_selection_poperties_show_column_titles)
    , m_show_group_titles(cfg_selection_poperties_show_group_titles)
    , m_edge_style(cfg_selection_properties_edge_style)
    , m_edit_column(pfc_infinite)
    , m_edit_index(pfc_infinite)
{
    m_fields.add_item(field_t("Artist", "ARTIST"));
    m_fields.add_item(field_t("Title", "TITLE"));
    m_fields.add_item(field_t("Album", "ALBUM"));
    m_fields.add_item(field_t("Date", "DATE"));
    m_fields.add_item(field_t("Genre", "GENRE"));
    m_fields.add_item(field_t("Composer", "COMPOSER"));
    m_fields.add_item(field_t("Performer", "PERFORMER"));
    m_fields.add_item(field_t("Album Artist", "ALBUM ARTIST"));
    m_fields.add_item(field_t("Track Number", "TRACKNUMBER"));
    m_fields.add_item(field_t("Total Tracks", "TOTALTRACKS"));
    m_fields.add_item(field_t("Disc Number", "DISCNUMBER"));
    m_fields.add_item(field_t("Total Discs", "TOTALDISCS"));
    m_fields.add_item(field_t("Comment", "COMMENT"));
}

void selection_properties_t::notify_save_inline_edit(const char* value)
{
    static_api_ptr_t<metadb_io_v2> tagger_api;
    if (strcmp(value, "<mixed values>") != 0) {
        pfc::list_t<pfc::string8> values;
        const char *ptr = value, *start = ptr;
        while (*ptr) {
            start = ptr;
            while (*ptr != ';' && *ptr)
                ptr++;
            values.add_item(pfc::string8(start, ptr - start));
            while (*ptr == ' ' || *ptr == ';')
                ptr++;
        }

        t_size value_count = values.get_count();

        metadb_handle_list ptrs(m_edit_handles);
        pfc::list_t<file_info_impl> infos;
        pfc::list_t<bool> mask;
        pfc::list_t<const file_info*> infos_ptr;
        t_size count = ptrs.get_count();
        mask.set_count(count);
        infos.set_count(count);
        // infos.set_count(count);
        for (t_size i = 0; i < count; i++) {
            assert(ptrs[i].is_valid());
            mask[i] = !ptrs[i]->get_info(infos[i]);
            infos_ptr.add_item(&infos[i]);
            if (!mask[i]) {
                pfc::string8 old_value;
                g_print_field(m_edit_field, infos[i], old_value);
                if (!(mask[i] = !((strcmp(old_value, value))))) {
                    infos[i].meta_remove_field(m_edit_field);
                    for (t_size j = 0; j < value_count; j++)
                        infos[i].meta_add(m_edit_field, values[j]);
                }
            }
        }
        infos_ptr.remove_mask(mask.get_ptr());
        ptrs.remove_mask(mask.get_ptr());

        {
            service_ptr_t<file_info_filter_impl> filter = new service_impl_t<file_info_filter_impl>(ptrs, infos_ptr);
            tagger_api->update_info_async(ptrs, filter, GetAncestor(get_wnd(), GA_ROOT),
                metadb_io_v2::op_flag_no_errors | metadb_io_v2::op_flag_background | metadb_io_v2::op_flag_delay_ui,
                nullptr);
        }
    }

    /*if (m_edit_index < m_fields.get_count())
    {
    (m_edit_column ? m_fields[m_edit_index].m_name : m_fields[m_edit_index].m_name_friendly) = value;
    pfc::list_t<uih::ListView:: InsertItem> items;
    items.set_count(1);
    items[0].m_subitems.add_item(m_fields[m_edit_index].m_name_friendly);
    items[0].m_subitems.add_item(m_fields[m_edit_index].m_name);
    replace_items(m_edit_index, items);
    }*/
    m_edit_column = pfc_infinite;
    m_edit_index = pfc_infinite;
    m_edit_field.reset();
    m_edit_handles.remove_all();
}

bool selection_properties_t::notify_create_inline_edit(const pfc::list_base_const_t<t_size>& indices, unsigned column,
    pfc::string_base& p_text, t_size& p_flags, mmh::ComPtr<IUnknown>& pAutocompleteEntries)
{
    t_size indices_count = indices.get_count();
    if (m_handles.get_count() && column == 1 && indices_count == 1 && indices[0] < m_fields.get_count()) {
        m_edit_index = indices[0];
        m_edit_column = column;
        m_edit_field = m_fields[m_edit_index].m_name;
        m_edit_handles = m_handles;

        pfc::string8_fast_aggressive text, temp;
        {
            metadb_info_container::ptr p_info;
            if (m_edit_handles[0]->get_info_ref(p_info))
                g_print_field(m_edit_field, p_info->info(), text);
            t_size count = m_handles.get_count();
            for (t_size i = 1; i < count; i++) {
                temp.reset();
                if (m_edit_handles[i]->get_info_ref(p_info))
                    g_print_field(m_edit_field, p_info->info(), temp);
                if (strcmp(temp, text) != 0) {
                    text = "<mixed values>";
                    break;
                }
            }
        }

        p_text = text;

        return true;
    }
    return false;
}

void selection_properties_t::g_print_field(const char* field, const file_info& p_info, pfc::string_base& p_out)
{
    t_size meta_index = p_info.meta_find(field);
    if (meta_index != pfc_infinite) {
        t_size count = p_info.meta_enum_value_count(meta_index);
        for (t_size i = 0; i < count; i++)
            p_out << p_info.meta_enum_value(meta_index, i) << (i + 1 < count ? "; " : "");
    }
}

bool selection_properties_t::notify_before_create_inline_edit(
    const pfc::list_base_const_t<t_size>& indices, unsigned column, bool b_source_mouse)
{
    return m_handles.get_count() && column == 1 && indices.get_count() == 1 && indices[0] < m_fields.get_count();
}

void selection_properties_t::notify_on_column_size_change(t_size index, int new_width)
{
    if (index == 0)
        m_column_name_width = new_width;
    else
        m_column_field_width = new_width;
}

bool selection_properties_t::notify_on_keyboard_keydown_filter(UINT msg, WPARAM wp, LPARAM lp, bool& b_processed)
{
    uie::window_ptr p_this = this;
    bool ret = get_host()->get_keyboard_shortcuts_enabled() && g_process_keydown_keyboard_shortcuts(wp);
    b_processed = ret;
    return ret;
}

bool selection_properties_t::notify_on_keyboard_keydown_copy()
{
    copy_selected_items_as_text(1);
    return true;
}

void selection_properties_t::get_menu_items(ui_extension::menu_hook_t& p_hook)
{
    ui_extension::menu_node_ptr p_node = new menu_node_source_popup(this);
    p_hook.add_node(p_node);
    p_node = new menu_node_autosize(this);
    p_hook.add_node(p_node);
    p_node = new uie::menu_node_configure(this);
    p_hook.add_node(p_node);
}

bool selection_properties_t::show_config_popup(HWND wnd_parent)
{
    selection_properties_config_t dialog(
        m_fields, m_edge_style, m_info_sections_mask, m_show_column_titles, m_show_group_titles);
    if (dialog.run_modal(wnd_parent)) {
        m_fields = dialog.m_fields;
        if (get_wnd()) {
            m_info_sections_mask = dialog.m_info_sections_mask;
            cfg_selection_properties_info_sections = dialog.m_info_sections_mask;

            m_show_column_titles = dialog.m_show_columns;
            cfg_selection_poperties_show_column_titles = m_show_column_titles;
            set_show_header(m_show_column_titles);

            if (m_show_group_titles != dialog.m_show_groups) {
                m_show_group_titles = dialog.m_show_groups;
                cfg_selection_poperties_show_group_titles = m_show_group_titles;

                remove_items(pfc::bit_array_true(), false);
                set_group_count(m_show_group_titles ? 1 : 0);
            }

            refresh_contents();
            m_edge_style = dialog.m_edge_style;
            cfg_selection_properties_edge_style = m_edge_style;
            set_edge_style(m_edge_style);
        }
        return true;
    }
    return false;
}

bool selection_properties_t::have_config_popup() const
{
    return true;
}

namespace {
font_client_selection_properties::factory<font_client_selection_properties> g_font_client_selection_properties;
font_header_client_selection_properties::factory<font_header_client_selection_properties>
    g_font_header_client_selection_properties;
font_group_client_selection_properties::factory<font_group_client_selection_properties>
    g_font_group_client_selection_properties;
} // namespace
uie::window_factory<selection_properties_t> g_selection_properties;

selection_properties_t::menu_node_source_popup::menu_node_source_popup(selection_properties_t* p_wnd)
{
    m_items.add_item(new menu_node_track_mode(p_wnd, 2));
    m_items.add_item(new menu_node_track_mode(p_wnd, 0));
    // m_items.add_item(new uie::menu_node_separator_t());
    // m_items.add_item(new menu_node_track_mode(p_wnd, 2));
    m_items.add_item(new menu_node_track_mode(p_wnd, 1));
}

void selection_properties_t::menu_node_source_popup::get_child(unsigned p_index, uie::menu_node_ptr& p_out) const
{
    p_out = m_items[p_index].get_ptr();
}

unsigned selection_properties_t::menu_node_source_popup::get_children_count() const
{
    return m_items.get_count();
}

bool selection_properties_t::menu_node_source_popup::get_display_data(
    pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = "Tracking mode";
    p_displayflags = 0;
    return true;
}

selection_properties_t::menu_node_autosize::menu_node_autosize(selection_properties_t* p_wnd) : p_this(p_wnd) {}

void selection_properties_t::menu_node_autosize::execute()
{
    p_this->m_autosizing_columns = !p_this->m_autosizing_columns;
    p_this->set_autosize(p_this->m_autosizing_columns);
}

bool selection_properties_t::menu_node_autosize::get_description(pfc::string_base& p_out) const
{
    return false;
}

bool selection_properties_t::menu_node_autosize::get_display_data(
    pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = "Auto-sizing columns";
    p_displayflags = (p_this->m_autosizing_columns) ? ui_extension::menu_node_t::state_checked : 0;
    return true;
}

selection_properties_t::menu_node_track_mode::menu_node_track_mode(selection_properties_t* p_wnd, t_size p_value)
    : p_this(p_wnd), m_source(p_value)
{
}

void selection_properties_t::menu_node_track_mode::execute()
{
    p_this->m_tracking_mode = m_source;
    cfg_selection_properties_tracking_mode = m_source;
    p_this->on_tracking_mode_change();
}

bool selection_properties_t::menu_node_track_mode::get_description(pfc::string_base& p_out) const
{
    return false;
}

bool selection_properties_t::menu_node_track_mode::get_display_data(
    pfc::string_base& p_out, unsigned& p_displayflags) const
{
    p_out = get_name(m_source);
    p_displayflags = (m_source == p_this->m_tracking_mode) ? ui_extension::menu_node_t::state_radiochecked : 0;
    return true;
}

const char* selection_properties_t::menu_node_track_mode::get_name(t_size source)
{
    if (source == track_nowplaying)
        return "Playing item";
    if (source == track_selection)
        return "Current selection";
    if (source == track_automatic)
        return "Automatic";
    return "";
}

track_property_callback_itemproperties::track_property_callback_itemproperties() : m_values(tabsize(g_info_sections)) {}

void track_property_callback_itemproperties::sort()
{
    t_size count = m_values.get_size();
    for (t_size i = 0; i < count; i++) {
        mmh::Permutation perm(m_values[i].get_count());
        mmh::sort_get_permutation(m_values[i].get_ptr(), perm, track_property_t::g_compare, false);
        m_values[i].reorder(perm.get_ptr());
    }
}

bool track_property_callback_itemproperties::is_group_wanted(const char* p_group)
{
    return true;
}

void track_property_callback_itemproperties::set_property(
    const char* p_group, double p_sortpriority, const char* p_name, const char* p_value)
{
    t_size index = g_get_info_secion_index_by_name(p_group);
    if (index != pfc_infinite)
        m_values[index].add_item(track_property_t(p_sortpriority, p_name, p_value));
}

track_property_callback_itemproperties::track_property_t::track_property_t(
    double p_sortpriority, const char* p_name, const char* p_value)
    : m_name(p_name), m_value(p_value), m_sortpriority(p_sortpriority)
{
}

int track_property_callback_itemproperties::track_property_t::g_compare(self_t const& a, self_t const& b)
{
    int ret = pfc::compare_t(a.m_sortpriority, b.m_sortpriority);
    if (!ret)
        ret = StrCmpLogicalW(
            pfc::stringcvt::string_wide_from_utf8(a.m_name), pfc::stringcvt::string_wide_from_utf8(b.m_name));
    return ret;
}

void appearance_client_selection_properties_impl::on_colour_changed(t_size mask) const
{
    selection_properties_t::g_redraw_all();
}
