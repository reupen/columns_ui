#include "pch.h"

#include "config_appearance.h"
#include "font_utils.h"
#include "system_appearance_manager.h"

namespace cui::fonts {

class NOVTABLE Font : public font {
public:
    Font(LOGFONT log_font, uih::direct_write::WeightStretchStyle wss, std::wstring typographic_family_name,
        std::vector<DWRITE_FONT_AXIS_VALUE> axis_values, const float size, DWRITE_RENDERING_MODE rendering_mode,
        bool force_greyscale_antialiasing)
        : m_log_font(std::move(log_font))
        , m_wss(std::move(wss))
        , m_typographic_family_name(std::move(typographic_family_name))
        , m_axis_values(axis_values)
        , m_size(size)
        , m_rendering_mode(rendering_mode)
        , m_force_greyscale_antialiasing(force_greyscale_antialiasing)
    {
    }

    const wchar_t* family_name(FontFamilyModel font_family_model = FontFamilyModel::Automatic) noexcept final
    {
        switch (font_family_model) {
        default:
            return m_typographic_family_name.empty() ? m_wss.family_name.c_str() : m_typographic_family_name.c_str();
        case FontFamilyModel::WeightStretchStyle:
            return m_wss.family_name.c_str();
        case FontFamilyModel::Typographic:
            return m_typographic_family_name.c_str();
        }
    }

    DWRITE_FONT_WEIGHT weight() noexcept final { return m_wss.weight; }
    DWRITE_FONT_STRETCH stretch() noexcept final { return m_wss.stretch; }
    DWRITE_FONT_STYLE style() noexcept final { return m_wss.style; }
    float size() noexcept final { return m_size > 0 ? m_size : 9.0f; }

    size_t axis_count() const noexcept final { return m_axis_values.size(); }

    DWRITE_FONT_AXIS_VALUE axis_value(size_t index) const final { return m_axis_values.at(index); }

    LOGFONT log_font() noexcept override
    {
        return log_font_for_scaling_factor(uih::direct_write::get_default_scaling_factor());
    }

    LOGFONT log_font_for_dpi(unsigned dpi) noexcept override
    {
        const auto scaling_factor = gsl::narrow_cast<float>(dpi) / gsl::narrow_cast<float>(USER_DEFAULT_SCREEN_DPI);
        return log_font_for_scaling_factor(scaling_factor);
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

        try {
            pfc::com_ptr_t<IDWriteTextFormat> text_format;

            if (factory_7 && !m_axis_values.empty()) {
                wil::com_ptr<IDWriteTextFormat3> text_format_3;
                THROW_IF_FAILED(factory_7->CreateTextFormat(family_name(), nullptr, m_axis_values.data(),
                    gsl::narrow<uint32_t>(m_axis_values.size()), size(), L"", &text_format_3));
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

            if (factory_7 && !m_axis_values.empty()) {
                wil::com_ptr<IDWriteTextFormat3> text_format_3;
                THROW_IF_FAILED(factory_7->CreateTextFormat(L"", nullptr, m_axis_values.data(),
                    gsl::narrow<uint32_t>(m_axis_values.size()), size(), L"", &text_format_3));
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
    LOGFONT log_font_for_scaling_factor(float scaling_factor) noexcept
    {
        LOGFONT scaled_log_font{m_log_font};
        scaled_log_font.lfHeight = -gsl::narrow_cast<long>(uih::direct_write::dip_to_px(m_size, scaling_factor) + 0.5f);
        return scaled_log_font;
    }

    LOGFONT m_log_font{};
    uih::direct_write::WeightStretchStyle m_wss{};
    std::wstring m_typographic_family_name;
    std::vector<DWRITE_FONT_AXIS_VALUE> m_axis_values;
    float m_size{};
    DWRITE_RENDERING_MODE m_rendering_mode{};
    bool m_force_greyscale_antialiasing{};
};

class NOVTABLE FontManager3 : public manager_v3 {
public:
    font::ptr get_font(GUID id) const override
    {
        system_appearance_manager::initialise();
        const auto entry = g_font_manager_data.find_by_id(id);

        if (!entry)
            throw exception_font_client_not_found();

        auto font_description = g_font_manager_data.resolve_font_description(entry);

        const auto& log_font = font_description.log_font;
        auto size = font_description.dip_size;
        auto wss = font_description.get_wss_with_fallback();
        auto axis_values = uih::direct_write::axis_values_to_vector(font_description.axis_values);

        return fb2k::service_new<Font>(log_font, wss, font_description.typographic_family_name, std::move(axis_values),
            size, static_cast<DWRITE_RENDERING_MODE>(rendering_mode.get()), force_greyscale_antialiasing.get());
    }

    void set_font_size(GUID id, float size) override
    {
        const auto entry = g_font_manager_data.find_by_id(id);

        if (!entry)
            throw exception_font_client_not_found();

        if (entry->font_mode != FontMode::Custom) {
            entry->font_description = g_font_manager_data.resolve_font_description(entry);
            entry->font_mode = FontMode::Custom;
        }

        entry->font_description.set_dip_size(size);

        client::ptr ptr;
        if (client::create_by_guid(id, ptr))
            g_font_manager_data.dispatch_client_font_changed(ptr);
    }

    [[nodiscard]] callback_token::ptr on_font_changed(GUID id, basic_callback::ptr callback) override
    {
        g_font_manager_data.add_callback(id, callback);

        return fb2k::service_new<lambda_callback_token>(
            [id, callback] { g_font_manager_data.remove_callback(id, callback); });
    }
};

namespace {
service_factory_t<FontManager3> _;
}

} // namespace cui::fonts
