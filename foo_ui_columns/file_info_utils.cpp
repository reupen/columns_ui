#include "pch.h"

#include "file_info_utils.h"

namespace cui::helpers {

namespace {

std::string_view trim_string(std::string_view value)
{
    const auto start = value.find_first_not_of(' ');
    const auto end = value.find_last_not_of(' ');

    if (start > end || start == std::string_view::npos)
        return ""sv;

    return value.substr(start, end - start + 1);
}

} // namespace

std::vector<std::string> split_meta_value(std::string_view value)
{
    std::vector<std::string> values;

    for (size_t offset{};;) {
        const size_t index = value.find(";"sv, offset);
        const auto substr = value.substr(offset, index - offset);
        const auto trimmed_substr = trim_string(substr);

        if (trimmed_substr.length() > 0)
            values.emplace_back(trimmed_substr);

        if (index == std::string_view::npos)
            break;

        offset = index + 1;
    }

    return values;
}

std::vector<std::string_view> get_meta_field_values(const file_info& info, std::string_view field)
{
    std::vector<std::string_view> values;

    for (const auto index : ranges::views::iota(size_t{}, info.meta_get_count_by_name(field.data()))) {
        values.emplace_back(info.meta_get(field.data(), index));
    }

    return values;
}

bool SingleFieldFileInfoFilter::apply_filter(metadb_handle_ptr p_location, t_filestats p_stats, file_info& p_info)
{
    auto old_values = get_meta_field_values(p_info, m_field);
    std::vector<std::string_view> new_values;
    ranges::push_back(new_values, m_new_values);

    if (old_values == new_values)
        return false;

    p_info.meta_remove_field(m_field.data());

    for (auto&& value : m_new_values)
        p_info.meta_add_ex(m_field.data(), m_field.length(), value.data(), value.length());

    return true;
}

} // namespace cui::helpers
