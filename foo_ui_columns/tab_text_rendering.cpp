#include "pch.h"

#include "tab_text_rendering.h"

#include "config_appearance.h"
#include "font_manager_data.h"

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
    HWND m_wnd{};
    HWND m_rendering_mode_combobox{};
    HWND m_force_greyscale_antialiasing_checkbox{};
    HWND m_use_colour_glyphs{};

    PreferencesTabHelper m_helper{{IDC_TITLE1}};
};

INT_PTR TextRenderingTab::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {
        m_wnd = wnd;
        m_rendering_mode_combobox = GetDlgItem(wnd, IDC_RENDERING_MODE);
        m_force_greyscale_antialiasing_checkbox = GetDlgItem(wnd, IDC_FORCE_GREYSCALE_ANTIALIASING);
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
            const auto label = uGetWindowText(m_use_colour_glyphs) + " (requires Windows 8.1 or later)";
            uSetWindowText(m_use_colour_glyphs, label.c_str());

            EnableWindow(m_use_colour_glyphs, FALSE);
        }

        Button_SetCheck(
            m_force_greyscale_antialiasing_checkbox, fonts::force_greyscale_antialiasing ? BST_CHECKED : BST_UNCHECKED);
        Button_SetCheck(
            m_use_colour_glyphs, are_colour_glyphs_supported && fonts::use_colour_glyphs ? BST_CHECKED : BST_UNCHECKED);

        break;
    }
    case WM_DESTROY: {
        m_wnd = nullptr;
        m_rendering_mode_combobox = nullptr;
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
            g_font_manager_data.dispatch_all_fonts_changed();
            break;
        }
        case IDC_FORCE_GREYSCALE_ANTIALIASING: {
            fonts::force_greyscale_antialiasing
                = Button_GetCheck(m_force_greyscale_antialiasing_checkbox) == BST_CHECKED;
            g_font_manager_data.dispatch_all_fonts_changed();
            break;
        }
        case IDC_USE_COLOUR_GLYPHS: {
            fonts::use_colour_glyphs = Button_GetCheck(m_use_colour_glyphs) == BST_CHECKED;
            g_font_manager_data.dispatch_all_fonts_changed();
            break;
        }
        }
        break;
    }
    return 0;
}

} // namespace

PreferencesTab& get_text_rendering_tab()
{
    static TextRenderingTab tab;
    return tab;
}

} // namespace cui::prefs
