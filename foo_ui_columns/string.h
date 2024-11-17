#pragma once

namespace cui::string {

template <typename Char>
std::basic_string_view<Char> trim(const std::basic_string_view<Char>& value, const Char* chars)
{
    const auto start = value.find_first_not_of(chars);
    const auto end = value.find_last_not_of(chars);

    if (start > end || start == std::string_view::npos)
        return {};

    return value.substr(start, end - start + 1);
}

std::optional<float> safe_stof(const std::wstring& value);

} // namespace cui::string
