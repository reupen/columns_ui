#pragma once

#include <optional>

#include "dark_mode_dialog.h"

namespace cui::splitter_utils {

pfc::array_t<uint8_t> serialise_splitter_item(const uie::splitter_item_full_v3_impl_t* item);
pfc::array_t<uint8_t> serialise_splitter_item(const uie::splitter_item_t* item);
std::unique_ptr<uie::splitter_item_full_v3_impl_t> deserialise_splitter_item(std::span<uint8_t> data);

CLIPFORMAT get_splitter_item_clipboard_format();

template <typename SplitterItem>
void copy_splitter_item_to_clipboard(const SplitterItem* item)
{
    auto data = serialise_splitter_item(item);
    if (!uih::set_clipboard_data(get_splitter_item_clipboard_format(), {data.get_ptr(), data.get_size()})) {
        auto message = "Error setting clipboard data: "s + helpers::get_last_win32_error_message().get_ptr();
        throw exception_io(message.c_str());
    }
}

template <typename SplitterItem>
bool copy_splitter_item_to_clipboard_safe(HWND wnd, const SplitterItem* item, bool is_cut = false)
{
    try {
        copy_splitter_item_to_clipboard(item);
        return true;
    } catch (const std::exception& ex) {
        dark::modeless_info_box(
            wnd, is_cut ? "Error – Cut panel" : "Error – Copy panel", ex.what(), uih::InfoBoxType::Error);
        return false;
    }
}

bool is_splitter_item_in_clipboard();
std::unique_ptr<uie::splitter_item_full_v3_impl_t> get_splitter_item_from_clipboard_safe(HWND wnd);

template <class T>
std::optional<T> get_config_item(uie::splitter_window::ptr splitter, size_t index, GUID item)
{
    T value;
    if (splitter->get_config_item(index, item, value))
        return {value};
    return {};
}

} // namespace cui::splitter_utils
