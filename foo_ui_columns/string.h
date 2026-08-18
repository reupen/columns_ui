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
std::optional<int> parse_int_forgiving(std::wstring_view string, const std::locale& loc = std::locale(""));

inline auto split_into_words(std::wstring_view text)
{
    return text | ranges::views::split_when([](auto&& character) { return std::iswspace(character); })
        | ranges::views::filter([](auto&& word) { return !ranges::empty(word); })
        | ranges::views::transform([](auto&& word) {
              const auto size = ranges::distance(word);
              return size > 0 ? std::wstring_view(&*ranges::begin(word), size) : std::wstring_view{};
          });
}

bool match_string(
    std::wstring_view full_string, std::wstring_view partial_string, bool ignore_symbols, bool starts_with);

} // namespace cui::string
