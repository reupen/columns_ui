#pragma once

namespace cui::rebar {

class RebarBandState {
public:
    GUID m_guid{};
    // Although we store the DPI, this does virtually nothing as remaining space is automatically
    // distributed among the remaining bands.
    uih::IntegerAndDpi<uint32_t> m_width{};
    bool m_break_before_band{};
    mutable pfc::array_t<t_uint8> m_config{};

    void export_to_fcl_stream(stream_writer* writer, t_uint32 fcl_type, abort_callback& aborter) const;
    void import_from_fcl_stream(stream_reader* reader, t_uint32 fcl_type, abort_callback& aborter);
    void write_to_stream(stream_writer* writer, abort_callback& aborter) const;
    void read_from_stream(stream_reader* reader, abort_callback& aborter);
    void write_extra(stream_writer* writer, abort_callback& aborter) const;
    void read_extra(stream_reader* reader, abort_callback& aborter);

    RebarBandState& operator=(RebarBandState&&) = default;
    RebarBandState& operator=(const RebarBandState& band_state) = default;
    RebarBandState() = default;

    RebarBandState(RebarBandState&&) = default;
    RebarBandState(const RebarBandState& band_state) = default;
};

class RebarBand {
public:
    RebarBandState m_state;
    ui_extension::window_ptr m_window{};
    HWND m_wnd{};

    RebarBand& operator=(RebarBand&&) = default;
    RebarBand& operator=(const RebarBand& band) = default;
    RebarBand() = default;

    RebarBand(RebarBand&&) = default;
    RebarBand(const RebarBand& band) = default;
};

} // namespace cui::rebar
