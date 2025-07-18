#include "pch.h"

#include "tab_text_rendering.h"

#include "config_appearance.h"
#include "font_manager_data.h"
#include "win32.h"

namespace cui::prefs {

namespace {

constexpr auto rendering_modes = {
    std::make_tuple(fonts::RenderingMode::Automatic, L"Default"),
    std::make_tuple(fonts::RenderingMode::DirectWriteAutomatic, L"Automatic anti-aliasing"),
    std::make_tuple(fonts::RenderingMode::Natural, L"Horizontal anti-aliasing"),
    std::make_tuple(fonts::RenderingMode::NaturalSymmetric, L"Symmetric anti-aliasing"),
    std::make_tuple(fonts::RenderingMode::GdiClassic, L"GDI-compatible, classic"),
    std::make_tuple(fonts::RenderingMode::GdiNatural, L"GDI-compatible, natural"),
    std::make_tuple(fonts::RenderingMode::GdiAliased, L"GDI-compatible, no anti-aliasing"),
};

class TextRenderingTab : public PreferencesTab {
public:
    INT_PTR on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    HWND create(HWND wnd) override
    {
        return m_helper.create(wnd, IDD_PREFS_TEXT_RENDERING,
            [this](auto&&... args) { return on_message(std::forward<decltype(args)>(args)...); });
    }

    const char* get_name() override { return "Text rendering"; }

    bool get_help_url(pfc::string_base& p_out) override { return false; }

private:
    std::optional<ptrdiff_t> find_family_name(std::wstring_view family_name);
    void enable_or_disable_emoji_controls() const;

    HWND m_wnd{};
    HWND m_rendering_mode_combobox{};
    HWND m_use_greyscale_antialiasing_checkbox{};
    HWND m_use_colour_glyphs{};
    HWND m_use_alternative_emoji_font_selection_checkbox{};
    HWND m_colour_emoji_family_combobox{};
    HWND m_monochrome_emoji_family_combobox{};
    bool m_is_font_fallback_available{};
    std::vector<std::wstring> m_emoji_family_names;

    PreferencesTabHelper m_helper{{IDC_TITLE1}};
};

INT_PTR TextRenderingTab::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        m_wnd = wnd;
        m_rendering_mode_combobox = GetDlgItem(wnd, IDC_RENDERING_MODE);
        m_use_greyscale_antialiasing_checkbox = GetDlgItem(wnd, IDC_USE_GREYSCALE_ANTIALIASING);
        m_use_colour_glyphs = GetDlgItem(wnd, IDC_USE_COLOUR_GLYPHS);

        for (auto&& [value, name] : rendering_modes) {
            const auto index = uih::combo_box_add_string_data(m_rendering_mode_combobox, name, WI_EnumValue(value));

            if (WI_EnumValue(value) == fonts::rendering_mode && index != CB_ERR)
                ComboBox_SetCurSel(m_rendering_mode_combobox, index);
        }

        const auto are_colour_glyphs_supported = [] {
            try {
                auto context = uih::direct_write::Context::s_create();
                return static_cast<bool>(context->factory().try_query<IDWriteFactory2>());
            }
            CATCH_LOG()

            return false;
        }();

        if (!are_colour_glyphs_supported) {
            const auto label = uGetWindowText(m_use_colour_glyphs) + " (requires Windows 8.1 or newer)";
            uSetWindowText(m_use_colour_glyphs, label.c_str());

            EnableWindow(m_use_colour_glyphs, FALSE);
        }

        Button_SetCheck(
            m_use_greyscale_antialiasing_checkbox, fonts::use_greyscale_antialiasing ? BST_CHECKED : BST_UNCHECKED);
        Button_SetCheck(
            m_use_colour_glyphs, are_colour_glyphs_supported && fonts::use_colour_glyphs ? BST_CHECKED : BST_UNCHECKED);

        m_use_alternative_emoji_font_selection_checkbox = GetDlgItem(wnd, IDC_USE_ALT_EMOJI_FONT_SELECTION);
        m_colour_emoji_family_combobox = GetDlgItem(wnd, IDC_COLOUR_EMOJI_FAMILY);
        m_monochrome_emoji_family_combobox = GetDlgItem(wnd, IDC_MONOCHROME_EMOJI_FAMILY);

        try {
            const auto direct_write_context = uih::direct_write::Context::s_create();
            m_is_font_fallback_available
                = static_cast<bool>(direct_write_context->factory().try_query<IDWriteFactory2>());

            m_emoji_family_names = direct_write_context->get_emoji_font_families();
        }
        CATCH_LOG()

        if (m_is_font_fallback_available) {
            Button_SetCheck(m_use_alternative_emoji_font_selection_checkbox,
                fonts::use_alternative_emoji_font_selection ? BST_CHECKED : BST_UNCHECKED);

            for (const auto& family_name : m_emoji_family_names) {
                ComboBox_AddString(m_colour_emoji_family_combobox, family_name.c_str());
                ComboBox_AddString(m_monochrome_emoji_family_combobox, family_name.c_str());
            }
        }

        enable_or_disable_emoji_controls();

        if (!m_is_font_fallback_available) {
            const auto checkbox_text
                = uGetWindowText(m_use_alternative_emoji_font_selection_checkbox) + " (requires Window 8.1 or newer)";
            uSetWindowText(m_use_alternative_emoji_font_selection_checkbox, checkbox_text.c_str());
        }

        if (const auto index = find_family_name(mmh::to_utf16(fonts::colour_emoji_font_family.get())))
            ComboBox_SetCurSel(m_colour_emoji_family_combobox, *index);

        if (const auto index = find_family_name(mmh::to_utf16(fonts::monochrome_emoji_font_family.get())))
            ComboBox_SetCurSel(m_monochrome_emoji_family_combobox, *index);

        break;
    }
    case WM_DESTROY: {
        m_wnd = nullptr;
        m_rendering_mode_combobox = nullptr;
        m_emoji_family_names.clear();
        break;
    }
    case WM_COMMAND:
        switch (wp) {
        case IDC_RENDERING_MODE | (CBN_SELCHANGE << 16): {
            const int index = ComboBox_GetCurSel(m_rendering_mode_combobox);

            if (index == CB_ERR)
                break;

            const auto data = ComboBox_GetItemData(m_rendering_mode_combobox, index);

            if (data == CB_ERR)
                break;

            fonts::rendering_mode = gsl::narrow<std::underlying_type_t<fonts::RenderingMode>>(data);
            g_font_manager_data.dispatch_rendering_options_changed();
            break;
        }
        case IDC_COLOUR_EMOJI_FAMILY | (CBN_SELCHANGE << 16): {
            const int index = ComboBox_GetCurSel(m_colour_emoji_family_combobox);

            if (index == CB_ERR)
                break;

            fonts::colour_emoji_font_family = mmh::to_utf8(m_emoji_family_names[index]).c_str();
            g_font_manager_data.dispatch_font_fallback_changed();
            break;
        }
        case IDC_MONOCHROME_EMOJI_FAMILY | (CBN_SELCHANGE << 16): {
            const int index = ComboBox_GetCurSel(m_monochrome_emoji_family_combobox);

            if (index == CB_ERR)
                break;

            fonts::monochrome_emoji_font_family = mmh::to_utf8(m_emoji_family_names[index]).c_str();
            g_font_manager_data.dispatch_font_fallback_changed();
            break;
        }
        case IDC_USE_GREYSCALE_ANTIALIASING: {
            fonts::use_greyscale_antialiasing = Button_GetCheck(m_use_greyscale_antialiasing_checkbox) == BST_CHECKED;
            g_font_manager_data.dispatch_rendering_options_changed();
            break;
        }
        case IDC_USE_COLOUR_GLYPHS: {
            fonts::use_colour_glyphs = Button_GetCheck(m_use_colour_glyphs) == BST_CHECKED;
            g_font_manager_data.dispatch_rendering_options_changed();
            break;
        }
        case IDC_USE_ALT_EMOJI_FONT_SELECTION: {
            fonts::use_alternative_emoji_font_selection
                = Button_GetCheck(m_use_alternative_emoji_font_selection_checkbox) == BST_CHECKED;
            enable_or_disable_emoji_controls();
            g_font_manager_data.dispatch_font_fallback_changed();
            break;
        }
        }
        break;
    }
    return 0;
}

std::optional<ptrdiff_t> TextRenderingTab::find_family_name(std::wstring_view search_family_name)
{
    auto iter = ranges::find_if(
        m_emoji_family_names, [search_family_name](auto&& family_name) { return family_name == search_family_name; });

    if (iter == m_emoji_family_names.end())
        return {};

    return std::distance(m_emoji_family_names.begin(), iter);
}

void TextRenderingTab::enable_or_disable_emoji_controls() const
{
    EnableWindow(m_use_alternative_emoji_font_selection_checkbox, m_is_font_fallback_available);
    EnableWindow(
        m_colour_emoji_family_combobox, m_is_font_fallback_available && fonts::use_alternative_emoji_font_selection);
    EnableWindow(m_monochrome_emoji_family_combobox,
        m_is_font_fallback_available && fonts::use_alternative_emoji_font_selection);
}

} // namespace

PreferencesTab& get_text_rendering_tab()
{
    static TextRenderingTab tab;
    return tab;
}

} // namespace cui::prefs
