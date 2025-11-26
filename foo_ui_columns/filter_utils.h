#pragma once
#include "filter_config_var.h"

namespace cui::panels::filter {

template <class Container>
void sort_tracks(Container&& tracks, const std::optional<SortOverride>& sort_override = {})
{
    if (sort_override) {
        fbh::sort_metadb_handle_list_by_format(
            std::forward<Container>(tracks), sort_override->object, nullptr, false, sort_override->is_reversed);
    } else if (cfg_sort) {
        service_ptr_t<titleformat_object> to;
        titleformat_compiler::get()->compile_safe(to, cfg_sort_string);
        fbh::sort_metadb_handle_list_by_format(
            std::forward<Container>(tracks), to, nullptr, false, cfg_reverse_sort_tracks);
    }
}

} // namespace cui::panels::filter
