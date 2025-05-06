#include "pch.h"

#include "config_appearance.h"
#include "font_utils.h"
#include "system_appearance_manager.h"
#include "ui_helpers/direct_write_emoji.h"

namespace cui::fonts {

class NOVTABLE Font : public font {
public:
    Font(LOGFONT log_font, uih::direct_write::WeightStretchStyle wss, std::wstring typographic_family_name,
        std::vector<DWRITE_FONT_AXIS_VALUE> axis_values, const float size, rendering_options::ptr rendering_opts,
        std::optional<uih::direct_write::EmojiFontSelectionConfig> emoji_font_selection_config)
        : m_log_font(std::move(log_font))
        , m_wss(std::move(wss))
        , m_typographic_family_name(std::move(typographic_family_name))
        , m_axis_values(axis_values)
        , m_size(size)
        , m_rendering_options(rendering_opts)
        , m_emoji_font_selection_config(emoji_font_selection_config)
    {
    }

    const wchar_t* family_name(font_family_model font_family_model = font_family_model::automatic) noexcept final
    {
        switch (font_family_model) {
        default:
            return m_typographic_family_name.empty() ? m_wss.family_name.c_str() : m_typographic_family_name.c_str();
        case font_family_model::weight_stretch_style:
            return m_wss.family_name.c_str();
        case font_family_model::typographic:
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

        const auto set_font_fallback = [&](auto&& text_format) {
            if (!m_emoji_font_selection_config)
                return;

            try {
                if (auto text_format_1 = text_format.template try_query<IDWriteTextFormat1>()) {
                    const auto font_fallback = context->create_emoji_font_fallback(*m_emoji_font_selection_config);
                    THROW_IF_FAILED(text_format_1->SetFontFallback(font_fallback.get()));
                }
            }
            CATCH_LOG();
        };

        const auto create_text_format = [&](auto&& create_family_name) {
            wil::com_ptr_t<IDWriteTextFormat> text_format;

            try {
                if (factory_7 && !m_axis_values.empty()) {
                    wil::com_ptr<IDWriteTextFormat3> text_format_3;
                    THROW_IF_FAILED(factory_7->CreateTextFormat(create_family_name, nullptr, m_axis_values.data(),
                        gsl::narrow<uint32_t>(m_axis_values.size()), size(), locale_name, &text_format_3));
                    text_format = std::move(text_format_3);
                } else {
                    THROW_IF_FAILED(context->factory()->CreateTextFormat(
                        create_family_name, nullptr, weight(), style(), stretch(), size(), locale_name, &text_format));
                }

                set_font_fallback(text_format);

                return text_format;
            }
            CATCH_LOG()

            return wil::com_ptr<IDWriteTextFormat>();
        };

        auto text_format = create_text_format(family_name());

        if (!text_format)
            text_format = create_text_format(L"");

        pfc::com_ptr_t<IDWriteTextFormat> pfc_text_format;

        pfc_text_format.attach(text_format.detach());

        return pfc_text_format;
    }

    [[nodiscard]] pfc::com_ptr_t<IDWriteFontFallback> font_fallback() noexcept override
    {
        try {
            const auto context = uih::direct_write::Context::s_create();
            auto font_fallback = context->create_emoji_font_fallback(*m_emoji_font_selection_config);

            pfc::com_ptr_t<IDWriteFontFallback> pfc_font_fallback;
            pfc_font_fallback.attach(font_fallback.detach());
            return pfc_font_fallback;
        }
        CATCH_LOG()

        return {};
    }

    rendering_options::ptr rendering_options() noexcept override { return m_rendering_options; }

private:
    LOGFONT log_font_for_scaling_factor(float scaling_factor) const noexcept
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
    rendering_options::ptr m_rendering_options;
    std::optional<uih::direct_write::EmojiFontSelectionConfig> m_emoji_font_selection_config;
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

        auto rendering_opts = fb2k::service_new<rendering_options_impl>(
            get_rendering_mode(), force_greyscale_antialiasing.get(), use_colour_glyphs.get());

        auto emoji_font_selection_config = use_alternative_emoji_font_selection
            ? std::make_optional(uih::direct_write::EmojiFontSelectionConfig{
                  mmh::to_utf16(colour_emoji_font_family.get()), mmh::to_utf16(monochrome_emoji_font_family.get())})
            : std::nullopt;

        return fb2k::service_new<Font>(log_font, wss, font_description.typographic_family_name, std::move(axis_values),
            size, std::move(rendering_opts), std::move(emoji_font_selection_config));
    }

    void set_font_size(GUID id, float size) override
    {
        system_appearance_manager::initialise();
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
        system_appearance_manager::initialise();
        g_font_manager_data.add_font_callback(id, callback);

        return fb2k::service_new<lambda_callback_token>(
            [id, callback] { g_font_manager_data.remove_font_callback(id, callback); });
    }

    [[nodiscard]] callback_token::ptr on_default_rendering_options_changed(basic_callback::ptr callback) override
    {
        system_appearance_manager::initialise();
        g_font_manager_data.add_rendering_options_callback(callback);

        return fb2k::service_new<lambda_callback_token>(
            [callback] { g_font_manager_data.remove_rendering_options_callback(callback); });
    }

    [[nodiscard]] callback_token::ptr on_default_font_fallback_changed(basic_callback::ptr callback) override
    {
        system_appearance_manager::initialise();
        g_font_manager_data.add_font_fallback_callback(callback);

        return fb2k::service_new<lambda_callback_token>(
            [callback] { g_font_manager_data.remove_font_fallback_callback(callback); });
    }

    [[nodiscard]] rendering_options::ptr get_default_rendering_options() noexcept override
    {
        system_appearance_manager::initialise();
        return fb2k::service_new<rendering_options_impl>(
            get_rendering_mode(), force_greyscale_antialiasing.get(), use_colour_glyphs.get());
    }

    [[nodiscard]] pfc::com_ptr_t<IDWriteFontFallback> get_default_font_fallback() noexcept override
    {
        if (!use_alternative_emoji_font_selection)
            return {};

        const auto emoji_font_selection_config = uih::direct_write::EmojiFontSelectionConfig{
            mmh::to_utf16(colour_emoji_font_family.get()), mmh::to_utf16(monochrome_emoji_font_family.get())};

        try {
            const auto context = uih::direct_write::Context::s_create();
            auto font_fallback = context->create_emoji_font_fallback(emoji_font_selection_config);

            pfc::com_ptr_t<IDWriteFontFallback> pfc_font_fallback;
            pfc_font_fallback.attach(font_fallback.detach());
            return pfc_font_fallback;
        }
        CATCH_LOG()

        return {};
    }
};

namespace {
service_factory_t<FontManager3> _;
}

} // namespace cui::fonts
