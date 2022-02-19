#include "stdafx.h"

#include "config.h"
#include "config_appearance.h"

namespace cui {

namespace {

class FontsDataSet : public fcl::dataset {
    enum { stream_version = 0 };
    void get_name(pfc::string_base& p_out) const override { p_out = "Fonts (unified)"; }
    const GUID& get_group() const override { return fcl::groups::colours_and_fonts; }
    const GUID& get_guid() const override
    {
        // {A806A9CD-4117-43da-805E-FE4EB348C90C}
        static const GUID guid = {0xa806a9cd, 0x4117, 0x43da, {0x80, 0x5e, 0xfe, 0x4e, 0xb3, 0x48, 0xc9, 0xc}};
        return guid;
    }
    enum Identifier {
        identifier_global_items,
        identifier_global_labels,
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
            t_size count = g_fonts_manager_data.m_entries.get_count();
            mem.write_lendian_t(count, p_abort);
            for (t_size i = 0; i < count; i++) {
                stream_writer_memblock mem2;
                g_fonts_manager_data.m_entries[i]->_export(&mem2, p_abort);
                out2.write_item(identifier_client_entry, mem2.m_data.get_ptr(), mem2.m_data.get_size());
            }
            out.write_item(identifier_client_entries, mem.m_data.get_ptr(), mem.m_data.get_size());
        }
    }
    void set_data(stream_reader* p_reader, t_size stream_size, t_uint32 type, fcl::t_import_feedback& feedback,
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

            stream_reader_memblock_ref data_reader(data);

            switch (element_id) {
            case identifier_global_items:
                g_fonts_manager_data.m_common_items_entry->import(&data_reader, data.get_size(), type, p_abort);
                break;
            case identifier_global_labels:
                g_fonts_manager_data.m_common_labels_entry->import(&data_reader, data.get_size(), type, p_abort);
                break;
            case identifier_client_entries: {
                fbh::fcl::Reader reader2(&data_reader, data.get_size(), p_abort);

                t_size count;
                reader2.read_item(count);

                g_fonts_manager_data.m_entries.remove_all();
                g_fonts_manager_data.m_entries.set_count(count);

                for (t_size i = 0; i < count; i++) {
                    t_uint32 element_id2;
                    t_uint32 element_size2;
                    reader2.read_item(element_id2);
                    reader2.read_item(element_size2);
                    if (element_id2 == identifier_client_entry) {
                        pfc::array_t<t_uint8> data2;
                        data2.set_size(element_size2);
                        reader2.read(data2.get_ptr(), data2.get_size());
                        stream_reader_memblock_ref element_reader(data2);
                        g_fonts_manager_data.m_entries[i] = std::make_shared<FontsManagerData::Entry>();
                        g_fonts_manager_data.m_entries[i]->import(&element_reader, data2.get_size(), type, p_abort);
                    } else
                        reader2.skip(element_size2);
                }
            } break;
            default:
                reader.skip(element_size);
                break;
            }
        }
        refresh_appearance_prefs();
        g_fonts_manager_data.g_on_common_font_changed(pfc_infinite);
        service_enum_t<fonts::client> font_enum;
        fonts::client::ptr ptr;
        while (font_enum.next(ptr))
            ptr->on_font_changed();
    }
};

service_factory_t<FontsDataSet> g_fcl_fonts_t;

} // namespace

} // namespace cui
