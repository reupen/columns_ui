#include "../stdafx.h"

#include "ng_playlist.h"
#include "ng_playlist_style.h"

namespace cui::panels::playlist_view {
namespace style_cache_manager {
pfc::list_t<SharedCellStyleData*> m_objects;
void g_add_object(const CellStyleData& p_data, SharedCellStyleData::ptr& p_out)
{
    t_size count = m_objects.get_count();
    for (t_size i = 0; i < count; i++)
        if (m_objects[i]->is_equal(p_data)) {
            p_out = m_objects[i];
            return;
        }
    p_out = new SharedCellStyleData(p_data);
    // console::formatter() << "added style obj: " << m_objects.add_item(p_out.get_ptr());
    m_objects.add_item(p_out.get_ptr());
}
void g_remove_object(SharedCellStyleData* p_object)
{
    m_objects.remove_item(p_object);
    // console::formatter() << "removed style obj";
}
} // namespace style_cache_manager

SharedCellStyleData::~SharedCellStyleData()
{
    style_cache_manager::g_remove_object(this);
}

CellStyleData CellStyleData::g_create_default()
{
    cui::colours::helper p_helper(ColoursClient::g_guid);
    return CellStyleData(p_helper.get_colour(cui::colours::colour_text),
        p_helper.get_colour(cui::colours::colour_selection_text), p_helper.get_colour(cui::colours::colour_background),
        p_helper.get_colour(cui::colours::colour_selection_background),
        p_helper.get_colour(cui::colours::colour_inactive_selection_text),
        p_helper.get_colour(cui::colours::colour_inactive_selection_background));
}

bool StyleTitleformatHook::process_field(
    titleformat_text_out* p_out, const char* p_name, unsigned p_name_length, bool& p_found_flag)
{
    p_found_flag = false;
    {
        if (p_name_length > 1 && !stricmp_utf8_ex(p_name, 1, "_", pfc_infinite)) {
            if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "text", pfc_infinite)) {
                if (!text.get_ptr()) {
                    text.set_size(33);
                    text.fill(0);
                    _ultoa_s(p_default_colours.text_colour, text.get_ptr(), text.get_size(), 0x10);
                }
                p_out->write(titleformat_inputtypes::unknown, text.get_ptr(), text.get_size());
                p_found_flag = true;
                return true;
            }
            if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "selected_text", pfc_infinite)) {
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
            if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "back", pfc_infinite)) {
                if (!back.get_ptr()) {
                    back.set_size(33);
                    back.fill(0);
                    _ultoa_s(p_default_colours.background_colour, back.get_ptr(), back.get_size(), 0x10);
                }
                p_out->write(titleformat_inputtypes::unknown, back.get_ptr(), back.get_size());
                p_found_flag = true;
                return true;
            }
            if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "selected_back", pfc_infinite)) {
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
            if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "selected_back_no_focus", pfc_infinite)) {
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
            if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "selected_text_no_focus", pfc_infinite)) {
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
            if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "themed", pfc_infinite)) {
                cui::colours::helper p_helper(ColoursClient::g_guid);
                if (p_helper.get_themed()) {
                    p_out->write(titleformat_inputtypes::unknown, "1", 1);
                    p_found_flag = true;
                }
                return true;
            }
            if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "display_index", pfc_infinite)) {
                if (!m_index_text.get_ptr()) {
                    m_index_text.set_size(33);
                    m_index_text.fill(0);
                    _ultoa_s(m_index + 1, m_index_text.get_ptr(), m_index_text.get_size(), 10);
                }
                p_out->write(titleformat_inputtypes::unknown, m_index_text.get_ptr(), m_index_text.get_size());
                p_found_flag = true;
                return true;
            }
            if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "is_group", pfc_infinite)) {
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

bool StyleTitleformatHook::process_function(titleformat_text_out* p_out, const char* p_name, unsigned p_name_length,
    titleformat_hook_function_params* p_params, bool& p_found_flag)
{
    p_found_flag = false;
    if (!stricmp_utf8_ex(p_name, p_name_length, "set_style", pfc_infinite)) {
        if (p_params->get_param_count() >= 2) {
            const char* name;
            unsigned name_length;
            p_params->get_param(0, name, name_length);
            if (!stricmp_utf8_ex(name, name_length, "text", pfc_infinite)) {
                {
                    const char* value;
                    unsigned value_length;
                    p_params->get_param(1, value, value_length);
                    if (value_length && *value == 3) {
                        value++;
                        value_length--;
                    }
                    p_colours.text_colour.set(mmh::strtoul_n(value, value_length, 0x10));
                    if (value_length == 6 * 2 + 2 && value[6] == '|') {
                        value += 7;
                        value_length -= 7;
                        p_colours.selected_text_colour.set(mmh::strtoul_n(value, value_length, 0x10));
                    } else
                        p_colours.selected_text_colour.set(0xffffff - p_colours.text_colour);
                }
                if (p_params->get_param_count() >= 3) {
                    {
                        const char* value;
                        unsigned value_length;
                        p_params->get_param(2, value, value_length);
                        if (value_length && *value == 3) {
                            value++;
                            value_length--;
                        }
                        p_colours.selected_text_colour.set(mmh::strtoul_n(value, value_length, 0x10));
                    }
                }
                if (p_params->get_param_count() >= 4) {
                    const char* value;
                    unsigned value_length;
                    p_params->get_param(3, value, value_length);
                    if (value_length && *value == 3) {
                        value++;
                        value_length--;
                    }
                    p_colours.selected_text_colour_non_focus.set(mmh::strtoul_n(value, value_length, 0x10));
                } else
                    p_colours.selected_text_colour_non_focus.set(p_colours.selected_text_colour);
            } else if (!stricmp_utf8_ex(name, name_length, "back", pfc_infinite)) {
                {
                    const char* value;
                    unsigned value_length;
                    p_params->get_param(1, value, value_length);
                    if (value_length && *value == 3) {
                        value++;
                        value_length--;
                    }
                    p_colours.background_colour.set(mmh::strtoul_n(value, value_length, 0x10));

                    if (value_length == 6 * 2 + 2 && value[6] == '|') {
                        value += 7;
                        value_length -= 7;
                        p_colours.selected_background_colour.set(mmh::strtoul_n(value, value_length, 0x10));
                    } else
                        p_colours.selected_background_colour.set(0xffffff - p_colours.background_colour);
                }
                if (p_params->get_param_count() >= 3) {
                    {
                        const char* value;
                        unsigned value_length;
                        p_params->get_param(2, value, value_length);
                        if (value_length && *value == 3) {
                            value++;
                            value_length--;
                        }
                        p_colours.selected_background_colour.set(mmh::strtoul_n(value, value_length, 0x10));
                    }
                    if (p_params->get_param_count() >= 4) {
                        const char* value;
                        unsigned value_length;
                        p_params->get_param(3, value, value_length);
                        if (value_length && *value == 3) {
                            value++;
                            value_length--;
                        }
                        p_colours.selected_background_colour_non_focus.set(mmh::strtoul_n(value, value_length, 0x10));
                    } else
                        p_colours.selected_background_colour_non_focus.set(p_colours.selected_background_colour);
                } else
                    p_colours.selected_background_colour_non_focus.set(p_colours.selected_background_colour);
            } else if (name_length >= 6 && !stricmp_utf8_ex(name, 6, "frame-", pfc_infinite)) {
                const char* p_side = name;
                p_side += 6;
                if (!stricmp_utf8_ex(p_side, name_length - 6, "left", pfc_infinite)) {
                    const char* value;
                    unsigned value_length;
                    p_params->get_param(1, value, value_length);
                    p_colours.use_frame_left = (value_length == 1 && *value == '1');
                    if (p_params->get_param_count() >= 3) {
                        {
                            const char* value;
                            unsigned value_length;
                            p_params->get_param(2, value, value_length);
                            if (value_length && *value == 3) {
                                value++;
                                value_length--;
                            }
                            p_colours.frame_left.set(mmh::strtoul_n(value, value_length, 0x10));
                        }
                    }
                } else if (!stricmp_utf8_ex(p_side, name_length - 6, "top", pfc_infinite)) {
                    const char* value;
                    unsigned value_length;
                    p_params->get_param(1, value, value_length);
                    p_colours.use_frame_top = (value_length == 1 && *value == '1');
                    if (p_params->get_param_count() >= 3) {
                        {
                            const char* value;
                            unsigned value_length;
                            p_params->get_param(2, value, value_length);
                            if (value_length && *value == 3) {
                                value++;
                                value_length--;
                            }
                            p_colours.frame_top.set(mmh::strtoul_n(value, value_length, 0x10));
                        }
                    }
                } else if (!stricmp_utf8_ex(p_side, name_length - 6, "right", pfc_infinite)) {
                    const char* value;
                    unsigned value_length;
                    p_params->get_param(1, value, value_length);
                    p_colours.use_frame_right = (value_length == 1 && *value == '1');
                    if (p_params->get_param_count() >= 3) {
                        {
                            const char* value;
                            unsigned value_length;
                            p_params->get_param(2, value, value_length);
                            if (value_length && *value == 3) {
                                value++;
                                value_length--;
                            }
                            p_colours.frame_right.set(mmh::strtoul_n(value, value_length, 0x10));
                        }
                    }
                } else if (!stricmp_utf8_ex(p_side, name_length - 6, "bottom", pfc_infinite)) {
                    const char* value;
                    unsigned value_length;
                    p_params->get_param(1, value, value_length);
                    p_colours.use_frame_bottom = (value_length == 1 && *value == '1');
                    if (p_params->get_param_count() >= 3) {
                        {
                            const char* value;
                            unsigned value_length;
                            p_params->get_param(2, value, value_length);
                            if (value_length && *value == 3) {
                                value++;
                                value_length--;
                            }
                            p_colours.frame_bottom.set(mmh::strtoul_n(value, value_length, 0x10));
                        }
                    }
                }
            }
            p_found_flag = true;
            return true;
        }
    } else if (!stricmp_utf8_ex(p_name, p_name_length, "calculate_blend_target", pfc_infinite)) {
        if (p_params->get_param_count() == 1) {
            const char* p_val;
            unsigned p_val_length;
            p_params->get_param(0, p_val, p_val_length);

            int colour = mmh::strtoul_n(p_val, p_val_length, 0x10);
            int total = (colour & 0xff) + ((colour & 0xff00) >> 8) + ((colour & 0xff0000) >> 16);

            COLORREF blend_target = total >= 128 * 3 ? 0 : 0xffffff;

            char temp[33];
            memset(temp, 0, 33);
            _ultoa_s(blend_target, temp, 16);
            p_out->write(titleformat_inputtypes::unknown, temp, 33);
            p_found_flag = true;
            return true;
        }
    } else if (!stricmp_utf8_ex(p_name, p_name_length, "offset_colour", pfc_infinite)) {
        if (p_params->get_param_count() == 3) {
            const char* p_val;
            const char* p_val2;
            unsigned p_val_length;
            unsigned p_val2_length;
            p_params->get_param(0, p_val, p_val_length);
            int colour = mmh::strtoul_n(p_val, p_val_length, 0x10);
            p_params->get_param(1, p_val2, p_val2_length);
            int target = mmh::strtoul_n(p_val2, p_val2_length, 0x10);
            int amount = p_params->get_param_uint(2);

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

            char temp[33];
            memset(temp, 0, 33);

            _ultoa_s(newrgb, temp, 16);
            p_out->write(titleformat_inputtypes::unknown, temp, 33);
            p_found_flag = true;
            return true;
        }
    }
    return false;
}
} // namespace cui::panels::playlist_view
