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
    Font(LOGFONT log_font, WeightStretchStyle wss, const float size)
        : m_log_font(std::move(log_font))
        , m_wss(std::move(wss))
        , m_size(size) {};

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

        try {
            pfc::com_ptr_t<IDWriteTextFormat> text_format;
            THROW_IF_FAILED(context->factory()->CreateTextFormat(
                family_name(), nullptr, weight(), style(), stretch(), size(), locale_name, text_format.receive_ptr()));

            return text_format;
        }
        CATCH_LOG()

        try {
            pfc::com_ptr_t<IDWriteTextFormat> text_format;
            THROW_IF_FAILED(context->factory()->CreateTextFormat(
                L"", nullptr, weight(), style(), stretch(), size(), locale_name, text_format.receive_ptr()));

            return text_format;
        }
        CATCH_LOG()

        return {};
    }

private:
    LOGFONT m_log_font;
    WeightStretchStyle m_wss;
    float m_size{};
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
        return fb2k::service_new<Font>(log_font, wss, size);
    }
};

service_factory_t<FontManager3> _;

} // namespace cui::fonts
