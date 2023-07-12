#pragma once

namespace cui::colours {

enum ColourScheme { ColourSchemeGlobal, ColourSchemeSystem, ColourSchemeThemed, ColourSchemeCustom };

struct ColourSet {
    ColourScheme colour_scheme{};
    COLORREF text{};
    COLORREF selection_text{};
    COLORREF inactive_selection_text{};
    COLORREF background{};
    COLORREF selection_background{};
    COLORREF inactive_selection_background{};
    COLORREF active_item_frame{};
    bool use_custom_active_item_frame{};

    bool operator==(const ColourSet&) const = default;

    void read(uint32_t version, stream_reader* stream, abort_callback& aborter);
    void write(stream_writer* stream, abort_callback& aborter) const;
};

ColourSet create_default_colour_set(bool is_dark, ColourScheme scheme = ColourSchemeGlobal);

class Entry {
public:
    using Ptr = std::shared_ptr<Entry>;

    enum ExportItemID {
        identifier_id,
        identifier_scheme,
        identifier_background,
        identifier_selection_background,
        identifier_inactive_selection_background,
        identifier_text,
        identifier_selection_text,
        identifier_inactive_selection_text,
        identifier_custom_active_item_frame,
        identifier_use_custom_active_item_frame,
    };
    GUID id{};
    ColourSet colour_set{};

    void write(stream_writer* stream, abort_callback& aborter);
    void _export(stream_writer* p_stream, abort_callback& p_abort);
    void import(stream_reader* p_reader, size_t stream_size, uint32_t type, abort_callback& p_abort);
    void read(uint32_t version, stream_reader* stream, abort_callback& aborter);
    explicit Entry(bool is_dark, bool b_global = false);
};

class ColourManagerData : public cfg_var {
public:
    static const GUID g_cfg_guid;
    enum { cfg_version = 0 };
    void get_data_raw(stream_writer* p_stream, abort_callback& p_abort) override;
    void set_data_raw(stream_reader* p_stream, size_t p_sizehint, abort_callback& p_abort) override;

    Entry::Ptr m_global_light_entry;
    Entry::Ptr m_global_dark_entry;
    std::vector<Entry::Ptr> m_light_entries;
    std::vector<Entry::Ptr> m_dark_entries;

    Entry::Ptr get_entry(GUID id, bool is_dark = is_dark_mode_active());
    Entry::Ptr get_global_entry(bool is_dark = is_dark_mode_active()) const;

    ColourManagerData();
};

class CommonColourCallbackManager {
public:
    void register_common_callback(common_callback* p_callback);
    void deregister_common_callback(common_callback* p_callback);

    void s_on_common_colour_changed(uint32_t mask);

    void s_on_common_bool_changed(uint32_t mask);

private:
    std::set<common_callback*> m_callbacks;
};

extern CommonColourCallbackManager common_colour_callback_manager;

} // namespace cui::colours
