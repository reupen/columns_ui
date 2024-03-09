#pragma once

namespace cui::helpers {

std::vector<std::string> split_meta_value(std::string_view value);

std::vector<std::string_view> get_meta_field_values(const file_info& info, std::string_view field);

class SingleFieldFileInfoFilter : public file_info_filter {
public:
    SingleFieldFileInfoFilter(std::string field, std::vector<std::string> new_values)
        : m_field(std::move(field))
        , m_new_values(std::move(new_values))
    {
    }

    bool apply_filter(metadb_handle_ptr p_location, t_filestats p_stats, file_info& p_info) override;

    std::string m_field;
    std::vector<std::string> m_new_values;
};

} // namespace cui::helpers
