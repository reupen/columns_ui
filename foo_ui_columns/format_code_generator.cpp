#include "pch.h"

#include "format_code_generator.h"

#include "dark_mode_dialog.h"
#include "font_picker.h"

namespace cui::utils {

namespace {

std::unordered_map<HWND, HWND> instance_map;

std::wstring create_set_format_snippet(const fonts::FontDescription& font_description)
{
    const auto& desc = font_description;

    if (!desc.wss)
        return L""s;

    const auto font_family
        = desc.typographic_family_name.empty() ? desc.wss->family_name : desc.typographic_family_name;

    const auto font_size = uih::direct_write::dip_to_pt(font_description.dip_size);
    const auto font_style = [style{desc.wss->style}] {
        switch (style) {
        default:
            return L"normal";
        case DWRITE_FONT_STYLE_OBLIQUE:
            return L"oblique";
        case DWRITE_FONT_STYLE_ITALIC:
            return L"italic";
        }
    }();

    const auto snippet = fmt::format(LR"($set_format(
  font-family: {};
  font-size: {};
  font-weight: {};
  font-stretch: {};
  font-style: {};
))",
        font_family, font_size, WI_EnumValue(desc.wss->weight), WI_EnumValue(desc.wss->stretch), font_style);

    return std::regex_replace(snippet, std::wregex(L"\n"), L"\r\n");
}

} // namespace

HWND open_format_code_generator(HWND parent_wnd, GUID initial_font_id)
{
    if (const auto iter = instance_map.find(parent_wnd); iter != instance_map.end()) {
        SetForegroundWindow(iter->second);
        return iter->second;
    }

    return dark::modeless_dialog_box(IDD_FORMAT_CODE_GENERATOR,
        {.button_ids = {IDCANCEL},
            .combo_box_ids = {IDC_FONT_FAMILY, IDC_FONT_FACE},
            .edit_ids = {IDC_FONT_SIZE, IDC_SET_FORMAT_SNIPPET},
            .spin_ids = {IDC_FONT_SIZE_SPIN},
            .last_button_id = IDCANCEL},
        parent_wnd,
        [initial_font_id, parent_wnd, font_picker{DirectWriteFontPicker{}}](
            auto wnd, auto msg, auto wp, auto lp) mutable -> INT_PTR {
            if (const auto result = font_picker.handle_message(wnd, msg, wp, lp); result)
                return *result;

            switch (msg) {
            case WM_INITDIALOG: {
                instance_map[parent_wnd] = wnd;

                const auto font = fonts::get_font(initial_font_id);

                fonts::FontDescription font_description;
                font_description.wss = uih::direct_write::WeightStretchStyle{
                    font->family_name(), font->weight(), font->stretch(), font->style()};
                font_description.set_dip_size(font->size());

                const auto snippet_wnd = GetDlgItem(wnd, IDC_SET_FORMAT_SNIPPET);
                uih::enhance_edit_control(snippet_wnd);

                const auto on_font_changed = [snippet_wnd](const auto& font_description) {
                    SetWindowText(snippet_wnd, create_set_format_snippet(font_description).c_str());
                };
                on_font_changed(font_description);

                font_picker.set_font_description(std::move(font_description));
                font_picker.on_font_changed(std::move(on_font_changed));
                return FALSE;
            }
            case WM_DESTROY:
                instance_map.erase(parent_wnd);
                break;
            case WM_COMMAND:
                if (wp == IDCANCEL)
                    DestroyWindow(wnd);
                return TRUE;
            }

            return FALSE;
        });
}

} // namespace cui::utils
