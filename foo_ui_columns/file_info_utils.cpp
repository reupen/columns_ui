#include "pch.h"

#include "file_info_utils.h"

#include "string.h"

namespace cui::helpers {

std::vector<std::string> split_meta_value(std::string_view value)
{
    std::vector<std::string> values;

    for (size_t offset{};;) {
        const size_t index = value.find(";"sv, offset);
        const auto substr = value.substr(offset, index - offset);
        const auto trimmed_substr = cui::string::trim(substr, " ");

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

bool EditMetadataFieldValueAggregator::process_file_info(const char* field, const file_info* info)
{
    if (m_truncated)
        return false;

    auto values = get_meta_field_values(*info, field);

    if (!m_first_track_values) {
        m_first_track_values
            = values | ranges::views::transform([](auto&& item) { return std::string{item}; }) | ranges::to_vector;
    } else if (!m_mixed_values) {
        m_mixed_values = !std::ranges::equal(*m_first_track_values, values);
    }

    for (auto& value : values)
        add_value(value);

    return !m_truncated;
}

void EditMetadataFieldValueAggregator::add_value(const std::string_view& value)
{
    if (m_mixed_values && m_values.size() >= max_values)
        return;

    const auto has_value = [&value](auto&& other) { return value == other; };

    if (ranges::any_of(m_values, has_value))
        return;

    m_values.emplace_back(value);

    if (m_mixed_values && m_values.size() == max_values)
        m_truncated = true;
}

} // namespace cui::helpers
