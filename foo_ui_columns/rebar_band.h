#pragma once

class RebarBandInfo {
public:
    GUID m_guid{};
    // Although we store the DPI, this does virtually nothing as remaining space is automatically
    // distributed among the remaining bands.
    uih::IntegerAndDpi<uint32_t> m_width{};
    bool m_break_before_band{};
    HWND m_wnd{};
    ui_extension::window_ptr m_window;
    mutable pfc::array_t<t_uint8> m_config;

    void export_to_fcl_stream(stream_writer* writer, t_uint32 fcl_type, abort_callback& aborter) const;
    void import_from_fcl_stream(stream_reader* reader, t_uint32 fcl_type, abort_callback& aborter);
    void write_to_stream(stream_writer* writer, abort_callback& aborter) const;
    void read_from_stream(stream_reader* reader, abort_callback& aborter);
    void write_extra(stream_writer* writer, abort_callback& aborter) const;
    void read_extra(stream_reader* reader, abort_callback& aborter);

    RebarBandInfo clone() const;
    RebarBandInfo& operator=(RebarBandInfo&&) = default;
    RebarBandInfo& operator=(const RebarBandInfo& band_info);
    RebarBandInfo() = default;

    RebarBandInfo(
        GUID guid, uih::IntegerAndDpi<uint32_t> width, bool break_before_band = false, uie::window_ptr window = {})
        : m_guid{guid}, m_width(width), m_break_before_band{break_before_band}, m_window(std::move(window))
    {
    }

    RebarBandInfo(RebarBandInfo&&) = default;
    RebarBandInfo(const RebarBandInfo& band_info);
};
