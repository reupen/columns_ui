#include "pch.h"

#include "font_manager_v3.h"

#include "config_appearance.h"
#include "font_utils.h"
#include "system_appearance_manager.h"

namespace cui::fonts {

const GUID font::class_guid{0x6e7708d5, 0x4799, 0x45e5, {0x8f, 0x41, 0x2d, 0x68, 0xf1, 0x5d, 0x08, 0x50}};
const GUID manager_v3::class_guid{0x471a234d, 0xb81a, 0x4f6a, {0x84, 0x94, 0x5c, 0x9f, 0xff, 0x2c, 0xf6, 0x1f}};

class Font : public font {
public:
    Font(LOGFONT log_font, WeightStretchStyle wss, std::wstring typographic_family_name,
        std::unordered_map<uint32_t, float> axis_values, const float size, DWRITE_RENDERING_MODE rendering_mode,
        bool force_greyscale_antialiasing)
        : m_log_font(std::move(log_font))
        , m_wss(std::move(wss))
        , m_typographic_family_name(typographic_family_name)
        , m_axis_values(axis_values)
        , m_size(size)
        , m_rendering_mode(rendering_mode)
        , m_force_greyscale_antialiasing(force_greyscale_antialiasing)
    {
    }

    const wchar_t* family_name() noexcept final { return m_wss.family_name.c_str(); }
    DWRITE_FONT_WEIGHT weight() noexcept final { return m_wss.weight; }
    DWRITE_FONT_STRETCH stretch() noexcept final { return m_wss.stretch; }
    DWRITE_FONT_STYLE style() noexcept final { return m_wss.style; }
    float size() noexcept final { return m_size > 0 ? m_size : 9.0f; }

    LOGFONT log_font() noexcept override
    {
        LOGFONT scaled_log_font{m_log_font};
        scaled_log_font.lfHeight = -gsl::narrow_cast<long>(uih::direct_write::dip_to_px(m_size) + 0.5f);
        return scaled_log_font;
    }

    pfc::com_ptr_t<IDWriteTextFormat> create_text_format(const wchar_t* locale_name = L"") noexcept final
    {
        uih::direct_write::Context::Ptr context;

        try {
            context = uih::direct_write::Context::s_create();
        } catch (...) {
            LOG_CAUGHT_EXCEPTION();
            return {};
        }

        const auto factory_7 = context->factory().try_query<IDWriteFactory7>();
        const auto axis_values = m_axis_values | ranges::views::transform([](auto pair) {
            return DWRITE_FONT_AXIS_VALUE{static_cast<DWRITE_FONT_AXIS_TAG>(pair.first), pair.second};
        }) | ranges::to<std::vector>;

        try {
            pfc::com_ptr_t<IDWriteTextFormat> text_format;

            if (factory_7 && !axis_values.empty()) {
                wil::com_ptr_t<IDWriteTextFormat3> text_format_3;
                THROW_IF_FAILED(factory_7->CreateTextFormat(m_typographic_family_name.c_str(), nullptr,
                    axis_values.data(), gsl::narrow<uint32_t>(axis_values.size()), size(), L"", &text_format_3));
                text_format.attach(text_format_3.detach());
            } else {
                THROW_IF_FAILED(context->factory()->CreateTextFormat(family_name(), nullptr, weight(), style(),
                    stretch(), size(), locale_name, text_format.receive_ptr()));
            }

            return text_format;
        }
        CATCH_LOG()

        try {
            pfc::com_ptr_t<IDWriteTextFormat> text_format;

            if (factory_7 && !axis_values.empty()) {
                wil::com_ptr_t<IDWriteTextFormat3> text_format_3;
                THROW_IF_FAILED(factory_7->CreateTextFormat(L"", nullptr, axis_values.data(),
                    gsl::narrow<uint32_t>(axis_values.size()), size(), L"", &text_format_3));
                text_format.attach(text_format_3.detach());
            } else {
                THROW_IF_FAILED(context->factory()->CreateTextFormat(
                    L"", nullptr, weight(), style(), stretch(), size(), locale_name, text_format.receive_ptr()));
            }

            return text_format;
        }
        CATCH_LOG()

        return {};
    }

    DWRITE_RENDERING_MODE rendering_mode() noexcept override { return m_rendering_mode; }
    bool force_greyscale_antialiasing() noexcept override { return m_force_greyscale_antialiasing; }

private:
    LOGFONT m_log_font{};
    WeightStretchStyle m_wss{};
    std::wstring m_typographic_family_name;
    std::unordered_map<uint32_t, float> m_axis_values;
    float m_size{};
    DWRITE_RENDERING_MODE m_rendering_mode{};
    bool m_force_greyscale_antialiasing{};
};

class FontManager3 : public manager_v3 {
public:
    font::ptr get_client_font(GUID id) const override
    {
        system_appearance_manager::initialise();
        const auto entry = g_font_manager_data.find_by_guid(id);
        auto font_description = g_font_manager_data.resolve_font_description(entry);

        const auto& log_font = font_description.log_font;
        auto size = font_description.dip_size;
        auto wss = font_description.get_wss_with_fallback();
        const auto& axis_values = font_description.axis_values;

        auto typographic_family_name = font_description.typographic_family_name.empty() && !axis_values.empty()
            ? font_description.wss->family_name
            : font_description.typographic_family_name;

        return fb2k::service_new<Font>(log_font, wss, std::move(typographic_family_name), axis_values, size,
            static_cast<DWRITE_RENDERING_MODE>(rendering_mode.get()), force_greyscale_antialiasing.get());
    }

    void set_client_font_size(GUID id, float size) override
    {
        const auto entry = g_font_manager_data.find_by_guid(id);

        if (entry->font_mode != font_mode_custom) {
            entry->font_description = g_font_manager_data.resolve_font_description(entry);
            entry->font_mode = font_mode_custom;
        }

        entry->font_description.set_dip_size(size);

        client::ptr ptr;
        if (client::create_by_guid(id, ptr))
            ptr->on_font_changed();
    }
};

service_factory_t<FontManager3> _;

} // namespace cui::fonts
