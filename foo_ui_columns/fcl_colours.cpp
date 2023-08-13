#include "pch.h"

#include "config_appearance.h"
#include "tab_colours.h"
#include "tab_dark_mode.h"

namespace cui {

namespace {

class ColoursDataSet : public fcl::dataset {
    enum {
        stream_version = 0
    };
    void get_name(pfc::string_base& p_out) const override { p_out = "Colours (unified)"; }
    const GUID& get_group() const override { return fcl::groups::colours_and_fonts; }
    const GUID& get_guid() const override
    {
        // {165946E7-6165-4680-A08E-84B5768458E8}
        static const GUID guid = {0x165946e7, 0x6165, 0x4680, {0xa0, 0x8e, 0x84, 0xb5, 0x76, 0x84, 0x58, 0xe8}};
        return guid;
    }
    enum Identifier {
        identifier_global_light_entry,
        identifier_light_entries,
        identifier_global_dark_entry,
        identifier_dark_entries,
        identifier_mode,
        identifier_client_entry = 0,
    };
    void get_data(stream_writer* p_writer, uint32_t type, fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);

        out.write_item(identifier_mode, cui::colours::dark_mode_status.get());

        std::initializer_list<std::tuple<Identifier, colours::Entry::Ptr>> global_identifiers_and_entries
            = {{identifier_global_light_entry, g_colour_manager_data.m_global_light_entry},
                {identifier_global_dark_entry, g_colour_manager_data.m_global_dark_entry}};

        for (auto&& [id, entry] : global_identifiers_and_entries) {
            stream_writer_memblock mem;
            entry->_export(&mem, p_abort);
            out.write_item(id, mem.m_data.get_ptr(), gsl::narrow<uint32_t>(mem.m_data.get_size()));
        }

        std::initializer_list<std::tuple<Identifier, std::vector<colours::Entry::Ptr>&>> identifiers_and_entries
            = {{identifier_light_entries, g_colour_manager_data.m_light_entries},
                {identifier_dark_entries, g_colour_manager_data.m_dark_entries}};

        for (auto&& [id, entries] : identifiers_and_entries) {
            stream_writer_memblock mem;
            fbh::fcl::Writer out2(&mem, p_abort);
            const size_t count = entries.size();
            mem.write_lendian_t(gsl::narrow<uint32_t>(count), p_abort);
            for (auto&& entry : entries) {
                stream_writer_memblock mem2;
                entry->_export(&mem2, p_abort);
                out2.write_item(
                    identifier_client_entry, mem2.m_data.get_ptr(), gsl::narrow<uint32_t>(mem2.m_data.get_size()));
            }
            out.write_item(id, mem.m_data.get_ptr(), gsl::narrow<uint32_t>(mem.m_data.get_size()));
        }
    }
    void set_data(stream_reader* p_reader, size_t stream_size, uint32_t type, fcl::t_import_feedback& feedback,
        abort_callback& p_abort) override
    {
        fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
        uint32_t element_id;
        uint32_t element_size;

        const auto old_is_dark = cui::colours::is_dark_mode_active();
        bool mode_read{};

        while (reader.get_remaining()) {
            reader.read_item(element_id);
            reader.read_item(element_size);

            switch (element_id) {
            case identifier_mode:
                colours::dark_mode_status.set(reader.read_item<int32_t>());
                mode_read = true;
                break;
            case identifier_global_light_entry:
            case identifier_global_dark_entry: {
                std::vector<uint8_t> data(element_size);
                reader.read(data.data(), data.size());

                stream_reader_memblock_ref colour_reader(data.data(), data.size());
                const auto entry = g_colour_manager_data.get_global_entry(element_id == identifier_global_dark_entry);
                entry->import(&colour_reader, data.size(), type, p_abort);
            } break;
            case identifier_light_entries:
            case identifier_dark_entries: {
                std::vector<uint8_t> data(element_size);
                reader.read(data.data(), data.size());

                stream_reader_memblock_ref stream2(data.data(), data.size());
                fbh::fcl::Reader sub_reader(&stream2, data.size(), p_abort);

                const auto count = sub_reader.read_item<uint32_t>();

                auto& entries = element_id == identifier_dark_entries ? g_colour_manager_data.m_dark_entries
                                                                      : g_colour_manager_data.m_light_entries;

                entries.clear();
                entries.reserve(count);

                for (auto _ [[maybe_unused]] : ranges::views::iota(0u, count)) {
                    const auto sub_element_id = sub_reader.read_item<uint32_t>();
                    const auto sub_element_size = sub_reader.read_item<uint32_t>();
                    if (sub_element_id == identifier_client_entry) {
                        std::vector<uint8_t> data2(sub_element_size);
                        sub_reader.read(data2.data(), data2.size());

                        stream_reader_memblock_ref colour_reader(data2.data(), data2.size());
                        auto entry = std::make_shared<colours::Entry>(false);
                        entry->import(&colour_reader, data2.size(), type, p_abort);
                        entries.emplace_back(std::move(entry));
                    } else
                        sub_reader.skip(sub_element_size);
                }
            } break;
            default:
                reader.skip(element_size);
                break;
            }
        }

        if (!mode_read)
            colours::dark_mode_status.set(WI_EnumValue(cui::colours::DarkModeStatus::Disabled));

        g_tab_dark_mode.refresh();

        if (old_is_dark != colours::is_dark_mode_active())
            return;

        g_tab_appearance.handle_external_configuration_change();

        colours::common_colour_callback_manager.s_on_common_colour_changed(colours::colour_flag_all);

        for (auto enumerator = colours::client::enumerate(); !enumerator.finished(); ++enumerator)
            (*enumerator)->on_colour_changed(colours::colour_flag_all);
    }
};

service_factory_t<ColoursDataSet> g_fcl_colours_t;

} // namespace

} // namespace cui
