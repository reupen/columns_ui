#include "pch.h"
#include "layout.h"
#include "main_window.h"
#include "rebar.h"

class LayoutDataSet : public cui::fcl::dataset_v2 {
    void get_name(pfc::string_base& p_out) const override { p_out = "Layout"; }
    const GUID& get_guid() const override
    {
        // {2CF00365-F2D7-4e78-9FCA-C1D56E2707D8}
        static const GUID guid = {0x2cf00365, 0xf2d7, 0x4e78, {0x9f, 0xca, 0xc1, 0xd5, 0x6e, 0x27, 0x7, 0xd8}};
        return guid;
    }
    void get_data(stream_writer* p_writer, t_uint32 type, cui::fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        pfc::list_t<GUID> panels;
        g_layout_window.export_config(p_writer, type, panels, p_abort);
        feedback.add_required_panels(panels);
    }
    const GUID& get_group() const override { return cui::fcl::groups::layout; }
    void set_data(stream_reader* p_reader, t_size size, t_uint32 type, cui::fcl::t_import_feedback& feedback,
        abort_callback& p_abort) override
    {
        pfc::list_t<GUID> panels;
        bool missingpanels = false;

        t_uint32 version;
        p_reader->read_lendian_t(version, p_abort);
        if (version > 0)
            throw pfc::exception("Need new columns ui");
        t_uint32 pcount;
        t_uint32 active;
        p_reader->read_lendian_t(active, p_abort);
        p_reader->read_lendian_t(pcount, p_abort);

        pfc::list_t<ConfigLayout::Preset> presets;

        for (t_uint32 j = 0; j < pcount; j++) {
            ConfigLayout::Preset pres;
            if (!g_layout_window.import_config_to_object(p_reader, size, type, pres, panels, p_abort))
                missingpanels = true;
            presets.add_item(pres);
        }

        t_size count = panels.get_count();
        for (t_size i = 0; i < count; i++) {
            uie::window_ptr ptr;
            if (!uie::window::create_by_guid(panels[i], ptr)) {
                missingpanels = true;
                feedback.add_required_panel("", panels[i]);
            }
        }

        if (!missingpanels || type == cui::fcl::type_private) {
            // cfg_layout.
            cfg_layout.set_presets(presets, active);
            // feedback.add_warning("Some panels missing, main layout not imported");
        }
        // else console::print("misspan");
    }

    [[nodiscard]] double get_import_priority() const override { return -100.0; }
};

cui::fcl::dataset_factory<LayoutDataSet> g_export_layout_t;

namespace cui::rebar {

class ToolbarLayoutDataSet : public fcl::dataset_v2 {
    void get_name(pfc::string_base& p_out) const override { p_out = "Toolbars"; }
    const GUID& get_guid() const override
    {
        // {2F802663-0BD1-4d3d-AE7E-0663007A9C2B}
        static const GUID guid = {0x2f802663, 0xbd1, 0x4d3d, {0xae, 0x7e, 0x6, 0x63, 0x0, 0x7a, 0x9c, 0x2b}};
        return guid;
    }
    const GUID& get_group() const override { return fcl::groups::toolbars; }
    void get_data(stream_writer* p_writer, t_uint32 type, fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        g_cfg_rebar.export_config(p_writer, type, feedback, p_abort);
    }
    void set_data(stream_reader* p_reader, t_size size, t_uint32 type, fcl::t_import_feedback& feedback,
        abort_callback& p_abort) override
    {
        pfc::list_t<GUID> panels;
        g_cfg_rebar.import_config(p_reader, size, type, panels, p_abort);
        t_size count = panels.get_count();
        for (t_size i = 0; i < count; i++) {
            feedback.add_required_panel("", panels[i]);
        }
    }

    [[nodiscard]] double get_import_priority() const override { return -100.0; }
};

fcl::dataset_factory<ToolbarLayoutDataSet> g_export_toolbars_t;

} // namespace cui::rebar

class MiscLayoutDataSet : public cui::fcl::dataset {
    enum ItemID { identifier_status, identifier_status_pane, identifier_allow_locked_panel_resizing };
    void get_name(pfc::string_base& p_out) const override { p_out = "Misc layout"; }
    const GUID& get_group() const override { return cui::fcl::groups::layout; }
    const GUID& get_guid() const override
    {
        // {78AA8894-4B2C-477d-B233-E7A7A7663D24}
        static const GUID guid = {0x78aa8894, 0x4b2c, 0x477d, {0xb2, 0x33, 0xe7, 0xa7, 0xa7, 0x66, 0x3d, 0x24}};
        return guid;
    }
    void get_data(stream_writer* p_writer, t_uint32 type, cui::fcl::t_export_feedback& feedback,
        abort_callback& p_abort) const override
    {
        fbh::fcl::Writer out(p_writer, p_abort);
        out.write_item(identifier_status, cfg_status);
        out.write_item(identifier_status_pane, settings::show_status_pane);
        out.write_item(identifier_allow_locked_panel_resizing, settings::allow_locked_panel_resizing.get_value());
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

            switch (element_id) {
            case identifier_status:
                reader.read_item(cfg_status);
                break;
            case identifier_status_pane:
                reader.read_item(settings::show_status_pane);
                break;
            case identifier_allow_locked_panel_resizing:
                settings::allow_locked_panel_resizing = reader.read_raw_item<bool>();
                break;
            default:
                reader.skip(element_size);
                break;
            }
        }

        on_show_status_change();
        on_show_status_pane_change();
    }
};
cui::fcl::dataset_factory<MiscLayoutDataSet> g_export_layout_misc;
