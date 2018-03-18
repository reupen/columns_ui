#pragma once

namespace cui::splitter_utils {

pfc::array_t<t_uint8> serialise_splitter_item(const uie::splitter_item_full_v3_impl_t* item);
pfc::array_t<t_uint8> serialise_splitter_item(const uie::splitter_item_t* item);
std::unique_ptr<uie::splitter_item_full_v3_impl_t> deserialise_splitter_item(gsl::span<t_uint8> data);

} // namespace cui::splitter_utils
