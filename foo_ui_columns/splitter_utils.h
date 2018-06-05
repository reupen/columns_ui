#pragma once

#include <optional>

namespace cui::splitter_utils {

pfc::array_t<t_uint8> serialise_splitter_item(const uie::splitter_item_full_v3_impl_t* item);
pfc::array_t<t_uint8> serialise_splitter_item(const uie::splitter_item_t* item);
std::unique_ptr<uie::splitter_item_full_v3_impl_t> deserialise_splitter_item(gsl::span<t_uint8> data);

CLIPFORMAT get_splitter_item_clipboard_format();

template <typename SplitterItem>
void copy_splitter_item_to_clipboard(const SplitterItem* item)
{
    auto data = serialise_splitter_item(item);
    if (!SetClipboardDataBlock(get_splitter_item_clipboard_format(), data)) {
        auto message = "Error setting clipboard data: "s + helpers::get_last_win32_error_message().get_ptr();
        throw exception_io(message.c_str());
    }
}

bool is_splitter_item_in_clipboard();
std::unique_ptr<uie::splitter_item_full_v3_impl_t> get_splitter_item_from_clipboard();

template <class T>
std::optional<T> get_config_item(uie::splitter_window::ptr splitter, size_t index, GUID item)
{
    T value;
    if (splitter->get_config_item(index, item, value))
        return {value};
    return {};
}

} // namespace cui::splitter_utils
