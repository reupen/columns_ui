#include "stdafx.h"
#include "filter_config_var.h"
#include "filter_search_bar.h"

namespace cui::filter::fcl {

enum class SettingIdentifier : uint32_t {
    Autosend = 1,
    DoubleClickAction,
    EdgeStyle,
    EnableColumnTitleClickSorting,
    FilterPrecedence,
    MiddleClickAction,
    ShowColumnTitles,
    ShowSortIndicators,
    ShowToolbarClearButton,
    ShowUnlabelledItems,
    SortOnAdd,
    SortOnAddScript,
    VerticalItemPadding,
};

enum class FieldIdentifier : uint32_t {
    Name = 1,
    Script,
    LastSortingDirection,
};

enum class FavouriteIdentifier : uint32_t {
    Query = 1,
};

static constexpr GUID filters_group_id = {0xe7cb5097, 0xd092, 0x49c0, {0xa1, 0xdb, 0x3d, 0x7f, 0x6, 0x8, 0x3c, 0x42}};
static constexpr GUID fields_group_id = {0xd5acf25, 0x4c55, 0x41cd, {0x8d, 0xe7, 0x55, 0x42, 0x70, 0x75, 0xec, 0xdf}};
static constexpr GUID favourites_group_id
    = {0x3ff9b75b, 0x7331, 0x4587, {0xa3, 0x87, 0xf5, 0x92, 0x9d, 0x2b, 0x2d, 0x43}};
static constexpr GUID behaviour_appearance_group_id
    = {0x1ff5fbdb, 0xf695, 0x4ebd, {0xa1, 0xe6, 0xc4, 0xa1, 0xba, 0xed, 0x96, 0xc0}};

static constexpr GUID settings_data_set_id
    = {0x2880447d, 0xe58e, 0x4a44, {0xb9, 0xe0, 0x2c, 0xdf, 0x73, 0x71, 0x45, 0xe3}};
static constexpr GUID favourites_data_set_id
    = {0xff1e26f7, 0xfdd0, 0x4d89, {0xb4, 0xc7, 0x83, 0x50, 0xc, 0x20, 0x8c, 0xa2}};
// {4C440D22-CDD6-41CD-899E-782A3526AF14}
static constexpr GUID fields_data_set_id
    = {0x4c440d22, 0xcdd6, 0x41cd, {0x89, 0x9e, 0x78, 0x2a, 0x35, 0x26, 0xaf, 0x14}};

static cui::fcl::group_impl_factory filters_group{filters_group_id, "Filters", "Filter panel settings", GUID{}};

static cui::fcl::group_impl_factory fields_group{
    fields_group_id, "Fields", "Global filter panel field configuration", filters_group_id};

static cui::fcl::group_impl_factory favourites_group{
    favourites_group_id, "Search favourites", "Filter search favourites", filters_group_id};

static cui::fcl::group_impl_factory behaviour_appearance_group{behaviour_appearance_group_id,
    "Appearance and behaviour", "Filter panel appearance and behaviour options", filters_group_id};

class SettingsDataSet : public cui::fcl::dataset {
    void get_name(pfc::string_base& p_out) const override { p_out = "Filter panel settings"; }
    const GUID& get_group() const override { return behaviour_appearance_group_id; }
    const GUID& get_guid() const override { return settings_data_set_id; }
    void get_data(stream_writer* p_writer, t_uint32 type, cui::fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);

        out.write_item(static_cast<uint32_t>(SettingIdentifier::Autosend), filter_panel::cfg_autosend);
        out.write_item(
            static_cast<uint32_t>(SettingIdentifier::DoubleClickAction), filter_panel::cfg_doubleclickaction);
        out.write_item(static_cast<uint32_t>(SettingIdentifier::EdgeStyle), filter_panel::cfg_edgestyle);
        out.write_item(
            static_cast<uint32_t>(SettingIdentifier::EnableColumnTitleClickSorting), filter_panel::cfg_allow_sorting);
        out.write_item(
            static_cast<uint32_t>(SettingIdentifier::FilterPrecedence), filter_panel::cfg_orderedbysplitters);
        out.write_item(
            static_cast<uint32_t>(SettingIdentifier::MiddleClickAction), filter_panel::cfg_middleclickaction);
        out.write_item(
            static_cast<uint32_t>(SettingIdentifier::ShowColumnTitles), filter_panel::cfg_show_column_titles);
        out.write_item(
            static_cast<uint32_t>(SettingIdentifier::ShowSortIndicators), filter_panel::cfg_show_sort_indicators);
        out.write_item(
            static_cast<uint32_t>(SettingIdentifier::ShowToolbarClearButton), filter_panel::cfg_showsearchclearbutton);
        out.write_item(static_cast<uint32_t>(SettingIdentifier::ShowUnlabelledItems), filter_panel::cfg_showemptyitems);
        out.write_item(static_cast<uint32_t>(SettingIdentifier::SortOnAdd), filter_panel::cfg_sort);
        out.write_item(static_cast<uint32_t>(SettingIdentifier::SortOnAddScript), filter_panel::cfg_sort_string);
        out.write_item(
            static_cast<uint32_t>(SettingIdentifier::VerticalItemPadding), filter_panel::cfg_vertical_item_padding);
    }
    void set_data(stream_reader* p_reader, t_size stream_size, t_uint32 type, cui::fcl::t_import_feedback& feedback,
        abort_callback& p_abort) override
    {
        fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
        t_uint32 element_id;
        t_uint32 element_size;

        while (reader.get_remaining()) {
            reader.read_item(element_id);
            reader.read_item(element_size);

            switch (static_cast<SettingIdentifier>(element_id)) {
            case SettingIdentifier::Autosend:
                reader.read_item(filter_panel::cfg_autosend);
                break;
            case SettingIdentifier::DoubleClickAction:
                reader.read_item(filter_panel::cfg_doubleclickaction);
                break;
            case SettingIdentifier::EdgeStyle:
                reader.read_item(filter_panel::cfg_edgestyle);
                break;
            case SettingIdentifier::EnableColumnTitleClickSorting:
                reader.read_item(filter_panel::cfg_allow_sorting);
                break;
            case SettingIdentifier::FilterPrecedence:
                reader.read_item(filter_panel::cfg_orderedbysplitters);
                break;
            case SettingIdentifier::MiddleClickAction:
                reader.read_item(filter_panel::cfg_middleclickaction);
                break;
            case SettingIdentifier::ShowColumnTitles:
                reader.read_item(filter_panel::cfg_show_column_titles);
                break;
            case SettingIdentifier::ShowSortIndicators:
                reader.read_item(filter_panel::cfg_show_sort_indicators);
                break;
            case SettingIdentifier::ShowToolbarClearButton:
                reader.read_item(filter_panel::cfg_showsearchclearbutton);
                break;
            case SettingIdentifier::ShowUnlabelledItems:
                reader.read_item(filter_panel::cfg_showemptyitems);
                break;
            case SettingIdentifier::SortOnAdd:
                reader.read_item(filter_panel::cfg_sort);
                break;
            case SettingIdentifier::SortOnAddScript:
                reader.read_item(filter_panel::cfg_sort_string, element_size);
                break;
            case SettingIdentifier::VerticalItemPadding:
                reader.read_item(filter_panel::cfg_vertical_item_padding);
                break;
            default:
                reader.skip(element_size);
                break;
            }
        }

        filter_panel::filter_panel_t::g_on_show_column_titles_change();
        filter_panel::filter_panel_t::g_on_show_sort_indicators_change();
        filter_panel::filter_panel_t::g_on_vertical_item_padding_change();
        filter_panel::filter_panel_t::g_on_edgestyle_change();
        filter_panel::filter_panel_t::g_on_allow_sorting_change();

        filter_panel::filter_panel_t::g_on_showemptyitems_change(filter_panel::cfg_showemptyitems, false);
        filter_panel::filter_panel_t::g_on_orderedbysplitters_change();
    }
};

cui::fcl::dataset_factory<SettingsDataSet> settings_data_set;

class FavouritesDataSet : public cui::fcl::dataset {
    void get_name(pfc::string_base& p_out) const override { p_out = "Filter search favourites"; }
    const GUID& get_group() const override { return favourites_group_id; }
    const GUID& get_guid() const override { return favourites_data_set_id; }
    void get_data(stream_writer* p_writer, t_uint32 type, cui::fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        const auto count = gsl::narrow<uint32_t>(filter_panel::cfg_favourites.get_count());
        p_writer->write_lendian_t(count, p_abort);

        for (auto i : ranges::view::iota(0, count)) {
            auto& favourite = filter_panel::cfg_favourites[i];
            auto data = write_favourite(favourite, p_abort);

            const auto size = gsl::narrow<uint32_t>(data.get_size());
            p_writer->write_lendian_t(size, p_abort);
            p_writer->write(data.get_ptr(), size, p_abort);
        }
    }
    void set_data(stream_reader* p_reader, t_size stream_size, t_uint32 type, cui::fcl::t_import_feedback& feedback,
        abort_callback& p_abort) override
    {
        uint32_t count{};
        p_reader->read_lendian_t(count, p_abort);
        pfc::array_staticsize_t<pfc::string8> imported_favourites{count};

        for (auto i : ranges::view::iota(0, count)) {
            uint32_t favourite_data_size{};
            p_reader->read_lendian_t(favourite_data_size, p_abort);
            pfc::array_staticsize_t<uint8_t> favourite_data(favourite_data_size);
            p_reader->read(favourite_data.get_ptr(), favourite_data_size, p_abort);
            stream_reader_memblock_ref field_reader(favourite_data);
            imported_favourites[i] = read_favourite(&field_reader, favourite_data_size, p_abort);
        }

        filter_panel::cfg_favourites.remove_all();
        filter_panel::cfg_favourites.add_items(imported_favourites);

        filter_panel::filter_search_bar::s_on_favourites_change();
    }

    pfc::array_t<uint8_t> write_favourite(pfc::string8& favourite, abort_callback& aborter) const
    {
        pfc::array_t<uint8_t> data;
        stream_writer_memblock_ref writer(data);
        fbh::fcl::Writer fcl_writer(&writer, aborter);
        fcl_writer.write_item(static_cast<uint32_t>(FavouriteIdentifier::Query), favourite);
        return data;
    }

    pfc::string8 read_favourite(stream_reader* reader, t_size size, abort_callback& aborter)
    {
        pfc::string8 favourite;
        fbh::fcl::Reader fcl_reader(reader, size, aborter);
        t_uint32 element_id;
        t_uint32 element_size;

        while (fcl_reader.get_remaining()) {
            fcl_reader.read_item(element_id);
            fcl_reader.read_item(element_size);

            switch (static_cast<FavouriteIdentifier>(element_id)) {
            case FavouriteIdentifier::Query:
                fcl_reader.read_item(favourite, element_size);
                break;
            default:
                fcl_reader.skip(element_size);
                break;
            }
        }

        return favourite;
    }
};

cui::fcl::dataset_factory<FavouritesDataSet> favourites_data_set;

class FieldsDataSet : public cui::fcl::dataset {
    void get_name(pfc::string_base& p_out) const override { p_out = "Filter panel fields"; }
    const GUID& get_group() const override { return fields_group_id; }
    const GUID& get_guid() const override { return fields_data_set_id; }

    void get_data(stream_writer* p_writer, t_uint32 type, cui::fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        const auto count = gsl::narrow<uint32_t>(filter_panel::cfg_field_list.get_count());
        p_writer->write_lendian_t(count, p_abort);

        for (auto i : ranges::view::iota(0, count)) {
            auto& field = filter_panel::cfg_field_list[i];
            auto data = write_field(field, p_abort);

            const auto size = gsl::narrow<uint32_t>(data.get_size());
            p_writer->write_lendian_t(size, p_abort);
            p_writer->write(data.get_ptr(), size, p_abort);
        }
    }

    void set_data(stream_reader* p_reader, t_size stream_size, t_uint32 type, cui::fcl::t_import_feedback& feedback,
        abort_callback& p_abort) override
    {
        uint32_t count{};
        p_reader->read_lendian_t(count, p_abort);
        pfc::array_staticsize_t<filter_panel::field_t> imported_fields{count};

        for (auto i : ranges::view::iota(0, count)) {
            uint32_t field_size{};
            p_reader->read_lendian_t(field_size, p_abort);
            pfc::array_staticsize_t<uint8_t> field_data(field_size);
            p_reader->read(field_data.get_ptr(), field_size, p_abort);
            stream_reader_memblock_ref field_reader(field_data);
            imported_fields[i] = read_field(&field_reader, field_size, p_abort);
        }

        filter_panel::cfg_field_list.remove_all();
        filter_panel::cfg_field_list.add_items(imported_fields);
        filter_panel::filter_panel_t::g_on_fields_change();
    }

    pfc::array_t<uint8_t> write_field(filter_panel::field_t& field, abort_callback& aborter) const
    {
        pfc::array_t<uint8_t> data;
        stream_writer_memblock_ref writer(data);
        fbh::fcl::Writer fcl_writer(&writer, aborter);
        fcl_writer.write_item(static_cast<uint32_t>(FieldIdentifier::Name), field.m_name);
        fcl_writer.write_item(static_cast<uint32_t>(FieldIdentifier::Script), field.m_field);
        fcl_writer.write_item(
            static_cast<uint32_t>(FieldIdentifier::LastSortingDirection), field.m_last_sort_direction);
        return data;
    }

    filter_panel::field_t read_field(stream_reader* reader, t_size size, abort_callback& aborter)
    {
        filter_panel::field_t field;
        fbh::fcl::Reader fcl_reader(reader, size, aborter);
        t_uint32 element_id;
        t_uint32 element_size;

        while (fcl_reader.get_remaining()) {
            fcl_reader.read_item(element_id);
            fcl_reader.read_item(element_size);

            switch (static_cast<FieldIdentifier>(element_id)) {
            case FieldIdentifier::Name:
                fcl_reader.read_item(field.m_name, element_size);
                break;
            case FieldIdentifier::Script:
                fcl_reader.read_item(field.m_field, element_size);
                break;
            case FieldIdentifier::LastSortingDirection:
                fcl_reader.read_item(field.m_last_sort_direction);
                break;
            default:
                fcl_reader.skip(element_size);
                break;
            }
        }

        return field;
    }
};

cui::fcl::dataset_factory<FieldsDataSet> fields_data_set;

} // namespace cui::filter::fcl
