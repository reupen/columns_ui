#pragma once

namespace cui::string {

template <typename Char>
struct TrimChars {};

template <>
struct TrimChars<wchar_t> {
    inline static const wchar_t* whitespace = L" \u00a0\u200b\u202f\ufeff\r\n";
};

template <typename Char>
std::basic_string_view<Char> trim(
    const std::basic_string_view<Char>& value, const Char* chars = TrimChars<Char>::whitespace)
{
    const auto start = value.find_first_not_of(chars);
    const auto end = value.find_last_not_of(chars);

    if (start > end || start == std::string_view::npos)
        return {};

    return value.substr(start, end - start + 1);
}

std::optional<float> safe_stof(const std::wstring& value);
std::optional<int> safe_stoi(const std::wstring& value);

} // namespace cui::string
