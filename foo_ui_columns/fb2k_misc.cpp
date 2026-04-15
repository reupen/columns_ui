#include "pch.h"

#include "fb2k_misc.h"

namespace cui::fb2k_utils {

wil::com_ptr_t<IDataObject> create_data_object_safe(const metadb_handle_list& tracks)
{
    wil::com_ptr_t<IDataObject> data_object;
    const auto ole_api = ole_interaction::get();

    try {
        data_object.attach(ole_api->create_dataobject(tracks).detach());
    } catch (const std::exception& ex) {
        console::print("Columns UI – failed to create data object: ", ex.what());
    }

    return data_object;
}

namespace {

uint32_t get_key_code(WPARAM wp, uint32_t modifiers)
{
    return static_cast<uint32_t>(wp & 0xFF) | ((modifiers & MOD_CONTROL) ? HOTKEYF_CONTROL << 8 : 0)
        | ((modifiers & MOD_SHIFT) ? HOTKEYF_SHIFT << 8 : 0) | ((modifiers & MOD_ALT) ? HOTKEYF_ALT << 8 : 0)
        | ((modifiers & MOD_WIN) ? HOTKEYF_EXT << 8 : 0);
}

} // namespace

bool process_edit_keyboard_shortcuts(WPARAM wp)
{
    if (wp == VK_LEFT || wp == VK_UP || wp == VK_RIGHT || wp == VK_DOWN || wp == VK_BACK || wp == VK_DELETE
        || wp == VK_RETURN || wp == VK_ESCAPE)
        return false;

    const auto modifiers = GetHotkeyModifierFlags();

    if (modifiers == MOD_CONTROL && (wp == 'A' || wp == 'Z' || wp == 'Y' || wp == 'X' || wp == 'C' || wp == 'V'))
        return false;

    if (modifiers == (MOD_CONTROL | MOD_SHIFT) && wp == 'Z')
        return false;

    const auto key_code = get_key_code(wp, modifiers);

    if (keyboard_shortcut_manager::is_typing_key_combo(key_code, modifiers))
        return false;

    return keyboard_shortcut_manager_v2::get()->process_keydown_simple(key_code);
}

} // namespace cui::fb2k_utils
