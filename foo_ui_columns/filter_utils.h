#pragma once
#include <filter_config_var.h>

namespace filter_panel {

template <class Container>
void sort_tracks(Container&& tracks)
{
    if (cfg_sort) {
        service_ptr_t<titleformat_object> to;
        static_api_ptr_t<titleformat_compiler>()->compile_safe(to, cfg_sort_string);
        fbh::sort_metadb_handle_list_by_format(
            std::forward<Container>(tracks), to, nullptr, false, cfg_reverse_sort_tracks);
    }
}

} // namespace filter_panel
