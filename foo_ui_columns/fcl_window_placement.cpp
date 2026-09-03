#include "pch.h"
#include "main_window.h"

namespace cui::fcl {
namespace {

namespace groups {

constexpr GUID main_window_placement_id{0x9d612b71, 0x14f7, 0x4864, {0xa0, 0xb2, 0x17, 0x78, 0x31, 0x3a, 0x6f, 0xda}};

group_impl_factory _main_window_placement_group(main_window_placement_id, "Main window size and position",
    "The main window size and position (excluding layout preset-specific sizes and positions)");

} // namespace groups

class MainWindowPlacementDataSet : public dataset {
    static constexpr auto main_window_placement_id = 1;

    void get_name(pfc::string_base& p_out) const override { p_out = "Main window size and position"; }

    const GUID& get_group() const override { return groups::main_window_placement_id; }

    const GUID& get_guid() const override
    {
        static constexpr GUID id{0xab22bbed, 0x68ab, 0x4af8, {0xb7, 0x3e, 0x7c, 0xf5, 0x71, 0x96, 0xf6, 0x21}};
        return id;
    }

    void get_data(
        stream_writer* p_writer, uint32_t type, t_export_feedback& feedback, abort_callback& aborter) const override
    {
        stream_writer_memblock placement_writer;
        cfg_window_placement_columns.get_data_raw(&placement_writer, aborter);

        if (placement_writer.m_data.size() == 0)
            return;

        fbh::fcl::Writer fcl_writer(p_writer, aborter);
        fcl_writer.write_item(main_window_placement_id, placement_writer.m_data.get_ptr(),
            gsl::narrow<uint32_t>(placement_writer.m_data.size()));
    }

    void set_data(stream_reader* p_reader, size_t stream_size, uint32_t type, t_import_feedback& feedback,
        abort_callback& aborter) override
    {
        fbh::fcl::Reader reader(p_reader, stream_size, aborter);
        uint32_t element_id{};
        uint32_t element_size{};

        bool is_imported{};

        while (reader.get_remaining()) {
            reader.read_item(element_id);
            reader.read_item(element_size);

            switch (element_id) {
            case main_window_placement_id: {
                std::vector<uint8_t> data(element_size);
                reader.read(data.data(), element_size);
                stream_reader_memblock_ref placement_reader(data.data(), element_size);
                cfg_window_placement_columns.set_data_raw(&placement_reader, element_size, aborter);
                is_imported = true;
                break;
            }
            default:
                reader.skip(element_size);
                break;
            }
        }

        if (is_imported)
            main_window.on_window_placement_imported();
    }
};

dataset_factory<MainWindowPlacementDataSet> _main_window_placement_dataset;

} // namespace

} // namespace cui::fcl
