#include "pch.h"

#include "string.h"

namespace cui::string {

std::optional<float> safe_stof(const std::wstring& value)
{
    try {
        return std::stof(value);
    } catch (const std::exception&) {
        return {};
    }
}

std::optional<int> parse_int_forgiving(std::wstring_view string, const std::locale& loc)
{
    const auto& numpunct = std::use_facet<std::numpunct<wchar_t>>(loc);
    const auto decimal_point = numpunct.decimal_point();
    const auto thousands_sep = numpunct.thousands_sep();

    std::wstring cleaned_string;
    cleaned_string.reserve(string.size());

    for (const wchar_t chr : string) {
        if (chr == thousands_sep)
            continue;

        if (chr == decimal_point) {
            break;
        }

        cleaned_string.push_back(chr);
    }

    try {
        return std::stoi(cleaned_string);
    } catch (const std::exception&) {
        return {};
    }
}

} // namespace cui::string
