#include "pch.h"

#include "config.h"
#include "config_appearance.h"

namespace cui {

namespace {

class FontsDataSet : public fcl::dataset {
    enum {
        stream_version = 0
    };
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
        identifier_rendering_mode,
        identifier_force_greyscale_antialiasing,

        identifier_client_entry = 0,
    };
    void get_data(stream_writer* p_writer, uint32_t type, fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);
        // p_writer->write_lendian_t(stream_version, p_abort);
        {
            stream_writer_memblock mem;
            g_font_manager_data.m_common_items_entry->_export(&mem, p_abort);
            out.write_item(identifier_global_items, mem.m_data.get_ptr(), gsl::narrow<uint32_t>(mem.m_data.get_size()));
        }
        {
            stream_writer_memblock mem;
            g_font_manager_data.m_common_labels_entry->_export(&mem, p_abort);
            out.write_item(
                identifier_global_labels, mem.m_data.get_ptr(), gsl::narrow<uint32_t>(mem.m_data.get_size()));
        }
        {
            stream_writer_memblock mem;
            fbh::fcl::Writer out2(&mem, p_abort);
            size_t count = g_font_manager_data.m_entries.get_count();
            mem.write_lendian_t(gsl::narrow<uint32_t>(count), p_abort);
            for (size_t i = 0; i < count; i++) {
                stream_writer_memblock mem2;
                g_font_manager_data.m_entries[i]->_export(&mem2, p_abort);
                out2.write_item(
                    identifier_client_entry, mem2.m_data.get_ptr(), gsl::narrow<uint32_t>(mem2.m_data.get_size()));
            }
            out.write_item(
                identifier_client_entries, mem.m_data.get_ptr(), gsl::narrow<uint32_t>(mem.m_data.get_size()));
        }
        out.write_item(identifier_rendering_mode, fonts::rendering_mode);
        out.write_item(identifier_force_greyscale_antialiasing, fonts::force_greyscale_antialiasing);
    }
    void set_data(stream_reader* p_reader, size_t stream_size, uint32_t type, fcl::t_import_feedback& feedback,
        abort_callback& p_abort) override
    {
        fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
        uint32_t element_id;
        uint32_t element_size;

        while (reader.get_remaining()) {
            reader.read_item(element_id);
            reader.read_item(element_size);

            const auto get_data = [&] {
                std::vector<uint8_t> data(element_size);
                reader.read(data.data(), data.size());
                return data;
            };

            switch (element_id) {
            case identifier_global_items: {
                const auto data = get_data();
                stream_reader_memblock_ref data_reader(data.data(), data.size());
                g_font_manager_data.m_common_items_entry->import(&data_reader, data.size(), type, p_abort);
                break;
            }
            case identifier_global_labels: {
                const auto data = get_data();
                stream_reader_memblock_ref data_reader(data.data(), data.size());
                g_font_manager_data.m_common_labels_entry->import(&data_reader, data.size(), type, p_abort);
                break;
            }
            case identifier_client_entries: {
                const auto data = get_data();
                stream_reader_memblock_ref data_reader(data.data(), data.size());
                fbh::fcl::Reader reader2(&data_reader, data.size(), p_abort);

                const auto count = reader2.read_item<uint32_t>();

                g_font_manager_data.m_entries.remove_all();
                g_font_manager_data.m_entries.set_count(count);

                for (size_t i = 0; i < count; i++) {
                    uint32_t element_id2;
                    uint32_t element_size2;
                    reader2.read_item(element_id2);
                    reader2.read_item(element_size2);
                    if (element_id2 == identifier_client_entry) {
                        pfc::array_t<uint8_t> data2;
                        data2.set_size(element_size2);
                        reader2.read(data2.get_ptr(), data2.get_size());
                        stream_reader_memblock_ref element_reader(data2);
                        g_font_manager_data.m_entries[i] = std::make_shared<FontManagerData::Entry>();
                        g_font_manager_data.m_entries[i]->import(&element_reader, data2.get_size(), type, p_abort);
                    } else
                        reader2.skip(element_size2);
                }
                break;
            }
            case identifier_rendering_mode:
                reader.read_item(fonts::rendering_mode);
                break;
            case identifier_force_greyscale_antialiasing:
                reader.read_item(fonts::force_greyscale_antialiasing);
                break;
            default:
                reader.skip(element_size);
                break;
            }
        }
        refresh_appearance_prefs();
        g_font_manager_data.dispatch_all_fonts_changed();
    }
};

service_factory_t<FontsDataSet> g_fcl_fonts_t;

} // namespace

} // namespace cui
