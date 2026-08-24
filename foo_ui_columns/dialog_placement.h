#pragma once

namespace cui::config {

struct DialogPlacement {
    WINDOWPLACEMENT placement{.length = sizeof(WINDOWPLACEMENT)};
    int dpi{USER_DEFAULT_SCREEN_DPI};
};

class DialogPlacementManager : public cfg_var {
public:
    DialogPlacementManager(const GUID& id) : cfg_var(id) {}

    void get_data_raw(stream_writer* stream, abort_callback& aborter) override;
    void set_data_raw(stream_reader* stream, size_t size, abort_callback& aborter) override;

    void register_window(const GUID& id, HWND wnd);
    void deregister_window(HWND wnd);

private:
    void save_placement(const GUID& id, HWND wnd);

    std::unordered_map<GUID, DialogPlacement, mmh::GUIDHasher> m_dialog_placements{};
    std::unordered_map<HWND, GUID> m_open_windows{};
};

extern DialogPlacementManager dialog_placement_manager;

} // namespace cui::config
