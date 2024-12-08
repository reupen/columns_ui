#include "pch.h"

#include "item_details_format_parser.h"

#ifdef __clang__
#pragma clang diagnostic warning "-Wshift-op-parentheses"
#endif

namespace cui::panels::item_details {

namespace {

namespace dsl = lexy::dsl;

struct Initial {
    static constexpr auto rule = dsl::lit<"initial">;
    static constexpr auto value = lexy::constant(InitialPropertyValue{});
};

struct Float {
    static constexpr auto rule = [] {
        constexpr auto whole_part = dsl::digits<>;
        constexpr auto fractional_part = dsl::period >> dsl::digits<>;
        constexpr auto decimal = dsl::token(whole_part + dsl::if_(fractional_part));
        return dsl::capture(decimal);
    }();

    static constexpr auto value = lexy::as_string<std::wstring, lexy::utf16_encoding>
        | lexy::callback<float>([](std::wstring&& str) { return std::stof(str); });
};

struct FontFamilyValue {
    struct String : lexy::token_production {
        static constexpr auto rule = dsl::list(dsl::capture(dsl::unicode::character - dsl::lit_c<';'>));
        static constexpr auto value = lexy::as_string<std::wstring, lexy::utf16_encoding>;
    };

    static constexpr auto rule = dsl::p<Initial> | dsl::else_ >> dsl::p<String>;
    static constexpr auto value = lexy::forward<decltype(FormatProperties::font_family)>;
};

struct FontSizeValue {
    static constexpr auto rule = dsl::p<Initial> | dsl::p<Float>;
    static constexpr auto value = lexy::forward<decltype(FormatProperties::font_size)>;
};

struct FontWeightValue {
    struct Weight {
        static constexpr auto rule = dsl::integer<int>;
        static constexpr auto value = lexy::callback<DWRITE_FONT_WEIGHT>(
            [](int value) { return static_cast<DWRITE_FONT_WEIGHT>(std::clamp(value, 1, 999)); });
    };

    static constexpr auto rule = dsl::p<Initial> | dsl::p<Weight>;
    static constexpr auto value = lexy::forward<decltype(FormatProperties::font_weight)>;
};

struct FontStretchValue {
    struct StretchClass {
        static constexpr auto rule = dsl::integer<int>;
        static constexpr auto value = lexy::callback<DWRITE_FONT_STRETCH>(
            [](int value) { return static_cast<DWRITE_FONT_STRETCH>(std::clamp(value, 1, 9)); });
    };

    struct Percentage {
        static constexpr auto rule = dsl::p<Float> + (dsl::lit<"pc"> | dsl::lit_c<'%'>);
        static constexpr auto value = lexy::forward<float>;
    };

    static constexpr auto rule
        = dsl::p<Initial> | dsl::peek(dsl::p<Percentage>) >> dsl::p<Percentage> | dsl::p<StretchClass>;
    static constexpr auto value = lexy::forward<decltype(FormatProperties::font_stretch)>;
};

struct FontStyleValue {
    struct Normal {
        static constexpr auto rule = dsl::lit<"normal">;
        static constexpr auto value = lexy::constant(DWRITE_FONT_STYLE_NORMAL);
    };

    struct Italic {
        static constexpr auto rule = dsl::lit<"italic">;
        static constexpr auto value = lexy::constant(DWRITE_FONT_STYLE_ITALIC);
    };

    struct Oblique {
        static constexpr auto rule = dsl::lit<"oblique">;
        static constexpr auto value = lexy::constant(DWRITE_FONT_STYLE_OBLIQUE);
    };

    static constexpr auto rule = dsl::p<Initial> | dsl::p<Normal> | dsl::p<Italic> | dsl::p<Oblique>;
    static constexpr auto value = lexy::forward<decltype(FormatProperties::font_style)>;
};

struct TextDecorationValue {
    struct None {
        static constexpr auto rule = dsl::lit<"none">;
        static constexpr auto value = lexy::constant(TextDecorationType::None);
    };

    struct Underline {
        static constexpr auto rule = dsl::lit<"underline">;
        static constexpr auto value = lexy::constant(TextDecorationType::Underline);
    };

    static constexpr auto rule = dsl::p<Initial> | dsl::p<None> | dsl::p<Underline>;
    static constexpr auto value = lexy::forward<decltype(FormatProperties::text_decoration)>;
};

struct UnknownProperty {
    static constexpr auto rule = dsl::until(dsl::lit_c<';'>);
};

template <typename Name, typename ValueRule, typename Member>
constexpr auto make_property(Name name, ValueRule value_rule, Member member)
{
    return name >> dsl::try_(dsl::lit_c<':'> + (member = value_rule) + (dsl::lit_c<';'> | dsl::eof),
               dsl::until(dsl::lit_c<';'>).or_eof());
}

struct FormatPropertiesProduction {
    static constexpr auto whitespace = dsl::unicode::space;

    static constexpr auto rule = dsl::partial_combination(
        make_property(dsl::lit<"font-family">, dsl::p<FontFamilyValue>, dsl::member<&FormatProperties::font_family>),
        make_property(dsl::lit<"font-size">, dsl::p<FontSizeValue>, dsl::member<&FormatProperties::font_size>),
        make_property(dsl::lit<"font-weight">, dsl::p<FontWeightValue>, dsl::member<&FormatProperties::font_weight>),
        make_property(dsl::lit<"font-stretch">, dsl::p<FontStretchValue>, dsl::member<&FormatProperties::font_stretch>),
        make_property(dsl::lit<"font-style">, dsl::p<FontStyleValue>, dsl::member<&FormatProperties::font_style>),
        make_property(
            dsl::lit<"text-decoration">, dsl::p<TextDecorationValue>, dsl::member<&FormatProperties::text_decoration>),
        dsl::inline_<UnknownProperty>);

    static constexpr auto value = lexy::as_aggregate<FormatProperties>;
};

} // namespace

std::optional<FormatProperties> parse_format_properties(std::wstring_view input)
{
    auto input_buffer = lexy::string_input<lexy::utf16_encoding>(input);

#ifdef _DEBUG
    std::ostringstream stream;
    auto result = lexy::parse<FormatPropertiesProduction>(
        input_buffer, lexy_ext::report_error.to(std::ostream_iterator<const char>(stream)));

    if (const auto errors = stream.view(); !errors.empty())
        console::print(fmt::format("Item details: $set_format() error: {}", errors).c_str());
#else
    auto result = lexy::parse<FormatPropertiesProduction>(input_buffer, lexy::noop);
#endif

    if (result.has_value())
        return result.value();

    return {};
}

} // namespace cui::panels::item_details
