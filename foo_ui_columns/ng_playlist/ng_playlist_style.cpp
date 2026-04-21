#include "pch.h"

#include "ng_playlist.h"
#include "ng_playlist_style.h"

#include "tf_utils.h"

namespace cui::panels::playlist_view {

namespace {

Colour get_colour_param(titleformat_hook_function_params& params, size_t index)
{
    const auto text = tf::get_param(params, index);
    const auto colour_text = text.size() > 0 && text[0] == 3 ? text.substr(1) : text;
    return Colour(mmh::strtoul_n(colour_text.data(), colour_text.size(), 0x10));
}

std::tuple<Colour, Colour> get_colour_pair_param(titleformat_hook_function_params& params, size_t index)
{
    const auto text = tf::get_param(params, index);
    const auto colour_text = text.size() > 0 && text[0] == 3 ? text.substr(1) : text;
    const auto colour_1 = mmh::strtoul_n(colour_text.data(), colour_text.size(), 0x10);

    const auto colour_2 = [&] {
        if (const auto bar_pos = colour_text.find_first_of('|'); bar_pos != std::string_view::npos) {
            const auto colour_2_text = colour_text.substr(bar_pos + 1);
            return mmh::strtoul_n(colour_2_text.data(), colour_2_text.size(), 0x10);
        }
        return 0xffffff - colour_1;
    }();

    return std::make_tuple(Colour(colour_1), Colour(colour_2));
}

} // namespace

namespace style_cache_manager {

pfc::list_t<SharedCellStyleData*> m_objects;

void g_add_object(const CellStyleData& p_data, SharedCellStyleData::ptr& p_out)
{
    size_t count = m_objects.get_count();
    for (size_t i = 0; i < count; i++)
        if (*static_cast<CellStyleData*>(m_objects[i]) == p_data) {
            p_out = m_objects[i];
            return;
        }
    p_out = new SharedCellStyleData(p_data);
    m_objects.add_item(p_out.get_ptr());
}

void g_remove_object(SharedCellStyleData* p_object)
{
    m_objects.remove_item(p_object);
}

} // namespace style_cache_manager

SharedCellStyleData::~SharedCellStyleData()
{
    style_cache_manager::g_remove_object(this);
}

CellStyleData CellStyleData::create_default()
{
    colours::helper p_helper(ColoursClient::id);
    return CellStyleData{Colour(p_helper.get_colour(colours::colour_text)),
        Colour(p_helper.get_colour(colours::colour_selection_text)),
        Colour(p_helper.get_colour(colours::colour_background)),
        Colour(p_helper.get_colour(colours::colour_selection_background)),
        Colour(p_helper.get_colour(colours::colour_inactive_selection_text)),
        Colour(p_helper.get_colour(colours::colour_inactive_selection_background))};
}

bool StyleTitleformatHook::process_field(
    titleformat_text_out* p_out, const char* p_name, size_t p_name_length, bool& p_found_flag)
{
    p_found_flag = false;
    {
        if (p_name_length > 1 && !stricmp_utf8_ex(p_name, 1, "_", SIZE_MAX)) {
            if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "text", SIZE_MAX)) {
                if (!text.get_ptr()) {
                    text.set_size(33);
                    text.fill(0);
                    _ultoa_s(p_default_colours.text_colour, text.get_ptr(), text.get_size(), 0x10);
                }
                p_out->write(titleformat_inputtypes::unknown, text.get_ptr(), text.get_size());
                p_found_flag = true;
                return true;
            }
            if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "selected_text", SIZE_MAX)) {
                if (!selected_text.get_ptr()) {
                    selected_text.set_size(33);
                    selected_text.fill(0);
                    _ultoa_s(p_default_colours.selected_text_colour, selected_text.get_ptr(), selected_text.get_size(),
                        0x10);
                }
                p_out->write(titleformat_inputtypes::unknown, selected_text.get_ptr(), selected_text.get_size());
                p_found_flag = true;
                return true;
            }
            if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "back", SIZE_MAX)) {
                if (!back.get_ptr()) {
                    back.set_size(33);
                    back.fill(0);
                    _ultoa_s(p_default_colours.background_colour, back.get_ptr(), back.get_size(), 0x10);
                }
                p_out->write(titleformat_inputtypes::unknown, back.get_ptr(), back.get_size());
                p_found_flag = true;
                return true;
            }
            if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "selected_back", SIZE_MAX)) {
                if (!selected_back.get_ptr()) {
                    selected_back.set_size(33);
                    selected_back.fill(0);
                    _ultoa_s(p_default_colours.selected_background_colour, selected_back.get_ptr(),
                        selected_back.get_size(), 0x10);
                }
                p_out->write(titleformat_inputtypes::unknown, selected_back.get_ptr(), selected_back.get_size());
                p_found_flag = true;
                return true;
            }
            if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "selected_back_no_focus", SIZE_MAX)) {
                if (!selected_back_no_focus.get_ptr()) {
                    selected_back_no_focus.set_size(33);
                    selected_back_no_focus.fill(0);
                    _ultoa_s(p_default_colours.selected_background_colour_non_focus, selected_back_no_focus.get_ptr(),
                        selected_back_no_focus.get_size(), 0x10);
                }
                p_out->write(titleformat_inputtypes::unknown, selected_back_no_focus.get_ptr(),
                    selected_back_no_focus.get_size());
                p_found_flag = true;
                return true;
            }
            if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "selected_text_no_focus", SIZE_MAX)) {
                if (!selected_text_no_focus.get_ptr()) {
                    selected_text_no_focus.set_size(33);
                    selected_text_no_focus.fill(0);
                    _ultoa_s(p_default_colours.selected_text_colour_non_focus, selected_text_no_focus.get_ptr(),
                        selected_text_no_focus.get_size(), 0x10);
                }
                p_out->write(titleformat_inputtypes::unknown, selected_text_no_focus.get_ptr(),
                    selected_text_no_focus.get_size());
                p_found_flag = true;
                return true;
            }
            if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "themed", SIZE_MAX)) {
                colours::helper p_helper(ColoursClient::id);
                if (p_helper.get_themed()) {
                    p_out->write(titleformat_inputtypes::unknown, "1", 1);
                    p_found_flag = true;
                }
                return true;
            }
            if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "display_index", SIZE_MAX)) {
                if (!m_index_text) {
                    m_index_text = fmt::format("{}", m_index + 1);
                }
                p_out->write(titleformat_inputtypes::unknown, m_index_text->data(), m_index_text->size());
                p_found_flag = true;
                return true;
            }
            if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "is_group", SIZE_MAX)) {
                if (m_is_group) {
                    p_out->write(titleformat_inputtypes::unknown, "1", 1);
                    p_found_flag = true;
                }
                return true;
            }
        }
    }

    return false;
}

bool StyleTitleformatHook::process_function(titleformat_text_out* p_out, const char* p_name, size_t p_name_length,
    titleformat_hook_function_params* p_params, bool& p_found_flag)
{
    p_found_flag = false;
    if (!stricmp_utf8_ex(p_name, p_name_length, "set_style", SIZE_MAX)) {
        if (p_params->get_param_count() >= 2) {
            const auto name = tf::get_param(*p_params, 0);
            if (!stricmp_utf8_ex(name.data(), name.size(), "text", SIZE_MAX)) {
                if (p_params->get_param_count() == 2) {
                    std::tie(p_colours.text_colour, p_colours.selected_text_colour)
                        = get_colour_pair_param(*p_params, 1);
                } else {
                    p_colours.text_colour = get_colour_param(*p_params, 1);
                    p_colours.selected_text_colour = get_colour_param(*p_params, 2);
                }

                if (p_params->get_param_count() >= 4)
                    p_colours.selected_text_colour_non_focus = get_colour_param(*p_params, 3);
                else
                    p_colours.selected_text_colour_non_focus = p_colours.selected_text_colour;
            } else if (!stricmp_utf8_ex(name.data(), name.size(), "back", SIZE_MAX)) {
                if (p_params->get_param_count() == 2) {
                    std::tie(p_colours.background_colour, p_colours.selected_background_colour)
                        = get_colour_pair_param(*p_params, 1);
                } else {
                    p_colours.background_colour = get_colour_param(*p_params, 1);
                    p_colours.selected_background_colour = get_colour_param(*p_params, 2);
                }

                if (p_params->get_param_count() >= 4)
                    p_colours.selected_background_colour_non_focus = get_colour_param(*p_params, 3);
                else
                    p_colours.selected_background_colour_non_focus = p_colours.selected_background_colour;
            } else if (!stricmp_utf8_ex(name.data(), name.size(), "group-line", SIZE_MAX)) {
                const auto param_1 = tf::get_param(*p_params, 1);
                p_colours.show_group_line = param_1 == "1"sv || param_1 == "true"sv;

                if (p_params->get_param_count() == 3)
                    p_colours.group_line_colour = get_colour_param(*p_params, 2);
            } else if (name.size() >= 6 && !stricmp_utf8_ex(name.data(), 6, "frame-", SIZE_MAX)) {
                const auto side_name = name.substr(6);
                const auto param_1 = tf::get_param(*p_params, 1);
                const auto use_frame = param_1 == "1"sv || param_1 == "true"sv;
                const auto colour = p_params->get_param_count() >= 3
                    ? std::make_optional(get_colour_param(*p_params, 2))
                    : std::nullopt;

                if (!stricmp_utf8_ex(side_name.data(), side_name.size(), "left", SIZE_MAX)) {
                    p_colours.use_frame_left = use_frame;

                    if (colour)
                        p_colours.frame_left = *colour;
                } else if (!stricmp_utf8_ex(side_name.data(), side_name.size(), "top", SIZE_MAX)) {
                    p_colours.use_frame_top = use_frame;

                    if (colour)
                        p_colours.frame_top = *colour;
                } else if (!stricmp_utf8_ex(side_name.data(), side_name.size(), "right", SIZE_MAX)) {
                    p_colours.use_frame_right = use_frame;

                    if (colour)
                        p_colours.frame_right = *colour;
                } else if (!stricmp_utf8_ex(side_name.data(), side_name.size(), "bottom", SIZE_MAX)) {
                    p_colours.use_frame_bottom = use_frame;

                    if (colour)
                        p_colours.frame_bottom = *colour;
                }
            }
            p_found_flag = true;
            return true;
        }
    } else if (!stricmp_utf8_ex(p_name, p_name_length, "calculate_blend_target", SIZE_MAX)) {
        if (p_params->get_param_count() == 1) {
            const char* p_val;
            size_t p_val_length;
            p_params->get_param(0, p_val, p_val_length);

            int colour = mmh::strtoul_n(p_val, p_val_length, 0x10);
            int total = (colour & 0xff) + ((colour & 0xff00) >> 8) + ((colour & 0xff0000) >> 16);

            COLORREF blend_target = total >= 128 * 3 ? 0 : 0xffffff;

            char temp[33]{};
            _ultoa_s(blend_target, temp, 16);
            p_out->write(titleformat_inputtypes::unknown, temp, 33);
            p_found_flag = true;
            return true;
        }
    } else if (!stricmp_utf8_ex(p_name, p_name_length, "offset_colour", SIZE_MAX)) {
        if (p_params->get_param_count() == 3) {
            const char* p_val;
            const char* p_val2;
            size_t p_val_length;
            size_t p_val2_length;
            p_params->get_param(0, p_val, p_val_length);
            int colour = mmh::strtoul_n(p_val, p_val_length, 0x10);
            p_params->get_param(1, p_val2, p_val2_length);
            int target = mmh::strtoul_n(p_val2, p_val2_length, 0x10);
            int amount = gsl::narrow_cast<int>(std::min(p_params->get_param_uint(2), size_t{255u}));

            int rdiff = (target & 0xff) - (colour & 0xff);
            int gdiff = ((target & 0xff00) >> 8) - ((colour & 0xff00) >> 8);
            int bdiff = ((target & 0xff0000) >> 16) - ((colour & 0xff0000) >> 16);

            int totaldiff = abs(rdiff) + abs(gdiff) + abs(bdiff);

            int newr = (colour & 0xff) + (totaldiff ? (rdiff * amount * 3 / totaldiff) : 0);
            if (newr < 0)
                newr = 0;
            if (newr > 255)
                newr = 255;

            int newg = ((colour & 0xff00) >> 8) + (totaldiff ? (gdiff * amount * 3 / totaldiff) : 0);
            if (newg < 0)
                newg = 0;
            if (newg > 255)
                newg = 255;

            int newb = ((colour & 0xff0000) >> 16) + (totaldiff ? (bdiff * amount * 3 / totaldiff) : 0);
            if (newb < 0)
                newb = 0;
            if (newb > 255)
                newb = 255;

            int newrgb = RGB(newr, newg, newb);

            char temp[33]{};

            _ultoa_s(newrgb, temp, 16);
            p_out->write(titleformat_inputtypes::unknown, temp, 33);
            p_found_flag = true;
            return true;
        }
    }
    return false;
}
} // namespace cui::panels::playlist_view
