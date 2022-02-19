#include "stdafx.h"
#include "config.h"

namespace cui {

namespace {

class LegacyFontsDataSet : public fcl::dataset {
    enum ItemID {
        font_status,
    };
    void get_name(pfc::string_base& p_out) const override { p_out = "Misc fonts"; }
    const GUID& get_group() const override { return fcl::groups::colours_and_fonts; }
    const GUID& get_guid() const override
    {
        // {0A297BE7-DE43-49da-8D8E-C8D888CF1014}
        static const GUID guid = {0xa297be7, 0xde43, 0x49da, {0x8d, 0x8e, 0xc8, 0xd8, 0x88, 0xcf, 0x10, 0x14}};
        return guid;
    }
    void get_data(stream_writer* p_writer, t_uint32 type, fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);
        // out.write_item(font_status, cfg_status_font);
    }
    void set_data(stream_reader* p_reader, t_size stream_size, t_uint32 type, fcl::t_import_feedback& feedback,
        abort_callback& p_abort) override
    {
        static_api_ptr_t<fonts::manager> api;
        fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
        t_uint32 element_id;
        t_uint32 element_size;
        bool font_read{false};

        while (reader.get_remaining()) {
            reader.read_item(element_id);
            reader.read_item(element_size);

            switch (element_id) {
            case font_status: {
                LOGFONT lf{};
                reader.read_item(lf);
                api->set_font(::fonts::status_bar, lf);
                font_read = true;
                break;
            }
            default:
                reader.skip(element_size);
                break;
            }
        }

        if (font_read)
            refresh_appearance_prefs();
        // on_status_font_change();
    }
};

fcl::dataset_factory<LegacyFontsDataSet> g_export_misc_fonts_t;

} // namespace

} // namespace cui
