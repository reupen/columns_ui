#include "pch.h"

#include "config_appearance.h"
#include "tab_colours.h"

namespace cui {

namespace {

class ColoursDataSet : public fcl::dataset {
    enum { stream_version = 0 };
    void get_name(pfc::string_base& p_out) const override { p_out = "Colours (unified)"; }
    const GUID& get_group() const override { return fcl::groups::colours_and_fonts; }
    const GUID& get_guid() const override
    {
        // {165946E7-6165-4680-A08E-84B5768458E8}
        static const GUID guid = {0x165946e7, 0x6165, 0x4680, {0xa0, 0x8e, 0x84, 0xb5, 0x76, 0x84, 0x58, 0xe8}};
        return guid;
    }
    enum Identifier {
        identifier_global_entry,
        identifier_client_entries,
        identifier_client_entry = 0,
    };
    void get_data(stream_writer* p_writer, t_uint32 type, fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);
        // p_writer->write_lendian_t(stream_version, p_abort);
        {
            stream_writer_memblock mem;
            g_colour_manager_data.m_global_entry->_export(&mem, p_abort);
            out.write_item(identifier_global_entry, mem.m_data.get_ptr(), gsl::narrow<uint32_t>(mem.m_data.get_size()));
        }
        {
            stream_writer_memblock mem;
            fbh::fcl::Writer out2(&mem, p_abort);
            size_t count = g_colour_manager_data.m_entries.get_count();
            mem.write_lendian_t(gsl::narrow<uint32_t>(count), p_abort);
            for (size_t i = 0; i < count; i++) {
                stream_writer_memblock mem2;
                g_colour_manager_data.m_entries[i]->_export(&mem2, p_abort);
                out2.write_item(
                    identifier_client_entry, mem2.m_data.get_ptr(), gsl::narrow<uint32_t>(mem2.m_data.get_size()));
            }
            out.write_item(
                identifier_client_entries, mem.m_data.get_ptr(), gsl::narrow<uint32_t>(mem.m_data.get_size()));
        }
    }
    void set_data(stream_reader* p_reader, size_t stream_size, t_uint32 type, fcl::t_import_feedback& feedback,
        abort_callback& p_abort) override
    {
        fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
        t_uint32 element_id;
        t_uint32 element_size;

        while (reader.get_remaining()) {
            reader.read_item(element_id);
            reader.read_item(element_size);

            pfc::array_t<t_uint8> data;
            data.set_size(element_size);
            reader.read(data.get_ptr(), data.get_size());

            switch (element_id) {
            case identifier_global_entry: {
                stream_reader_memblock_ref colour_reader(data);
                g_colour_manager_data.m_global_entry->import(&colour_reader, data.get_size(), type, p_abort);
            } break;
            case identifier_client_entries: {
                stream_reader_memblock_ref stream2(data);
                fbh::fcl::Reader reader2(&stream2, data.get_size(), p_abort);

                const auto count = reader2.read_item<uint32_t>();

                g_colour_manager_data.m_entries.remove_all();
                g_colour_manager_data.m_entries.set_count(count);

                for (size_t i = 0; i < count; i++) {
                    t_uint32 element_id2;
                    t_uint32 element_size2;
                    reader2.read_item(element_id2);
                    reader2.read_item(element_size2);
                    if (element_id2 == identifier_client_entry) {
                        pfc::array_t<t_uint8> data2;
                        data2.set_size(element_size2);
                        reader2.read(data2.get_ptr(), data2.get_size());
                        stream_reader_memblock_ref colour_reader(data2);
                        g_colour_manager_data.m_entries[i] = std::make_shared<ColourManagerData::Entry>();
                        g_colour_manager_data.m_entries[i]->import(&colour_reader, data2.get_size(), type, p_abort);
                    } else
                        reader2.skip(element_size2);
                }
            } break;
            default:
                reader.skip(element_size);
                break;
            }
        }
        if (g_tab_appearance.is_active()) {
            g_tab_appearance.update_mode_combobox();
            g_tab_appearance.update_fills();
        }
        g_colour_manager_data.g_on_common_colour_changed(colours::colour_flag_all);
        service_enum_t<colours::client> colour_enum;
        colours::client::ptr ptr;
        while (colour_enum.next(ptr))
            ptr->on_colour_changed(colours::colour_flag_all);
    }
};

service_factory_t<ColoursDataSet> g_fcl_colours_t;

} // namespace

} // namespace cui
