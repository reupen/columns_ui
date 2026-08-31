#pragma once
#include "config_utils.h"

namespace cui::config {

class DialogPlacementManager : public cfg_var {
public:
    DialogPlacementManager(const GUID& id) : cfg_var(id) {}

    void get_data_raw(stream_writer* stream, abort_callback& aborter) override;
    void set_data_raw(stream_reader* stream, size_t size, abort_callback& aborter) override;

    void register_window(const GUID& id, HWND wnd);
    void deregister_window(HWND wnd);

private:
    void save_placement(const GUID& id, HWND wnd);

    std::unordered_map<GUID, WindowPlacementAndDpi, mmh::GUIDHasher> m_placements_and_dpis{};
    std::unordered_map<HWND, GUID> m_open_windows{};
};

extern DialogPlacementManager dialog_placement_manager;

} // namespace cui::config
