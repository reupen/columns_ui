#pragma once

namespace cui {

pfc::string8 format_playlist_title(
    size_t index, const titleformat_object::ptr& tf_object, std::optional<float> default_font_size);

}
