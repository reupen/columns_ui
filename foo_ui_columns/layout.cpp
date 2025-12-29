#include "pch.h"
#include "layout.h"

#include "dark_mode_dialog.h"
#include "splitter_utils.h"
#include "main_window.h"
#include "panel_utils.h"

namespace {

bool does_splitter_have_panel(const uie::splitter_window::ptr& splitter, GUID id)
{
    return ranges::any_of(ranges::views::iota(size_t{}, splitter->get_panel_count()), [&](auto&& index) {
        uie::splitter_item_ptr item;
        splitter->get_panel(index, item);
        return item->get_panel_guid() == id;
    });
}

auto get_supported_panel_info(const uie::splitter_window::ptr& splitter)
{
    pfc::list_t<uie::window::ptr> windows;

    for (const auto& window : uie::window::enumerate())
        windows.add_item(window);

    uie::splitter_window_v2::ptr splitter_v2;

    if (splitter->service_query_t(splitter_v2)) {
        bit_array_bittable mask_remove(windows.size());
        splitter_v2->get_supported_panels(windows, mask_remove);
        windows.remove_mask(mask_remove);
    }

    return cui::panel_utils::get_panel_info(windows) | ranges::actions::remove_if([&splitter](const auto& item) {
        return item.is_single_instance && does_splitter_have_panel(splitter, item.id);
    });
}

void toggle_splitter_item_boolean(
    const uie::splitter_window::ptr& splitter, const uie::window::ptr& panel, GUID property_id)
{
    size_t index{};

    if (!splitter->find_by_ptr(panel, index))
        return;

    if (const auto old_value = cui::splitter_utils::get_config_item<bool>(splitter, index, property_id)) {
        splitter->set_config_item_t(index, property_id, !*old_value, fb2k::noAbort);
    }
}

std::optional<pfc::array_t<uint8_t>> convert_splitter_and_get_config(
    HWND wnd, const uie::splitter_window::ptr& old_splitter, GUID new_panel_id)
{
    uie::window_ptr window;
    service_ptr_t<uie::splitter_window> new_splitter;
    if (!uie::window::create_by_guid(new_panel_id, window) || !window->service_query_t(new_splitter))
        return {};

    const auto panel_count = old_splitter->get_panel_count();
    if (panel_count > new_splitter->get_maximum_panel_count()
        && !cui::dark::modal_info_box(wnd, "Change container type",
            "The number of child items will not fit in the selected container type. Do you want to "
            "continue?",
            uih::InfoBoxType::Warning, uih::InfoBoxModalType::YesNo))
        return {};

    for (auto index : std::views::iota(size_t{}, panel_count)) {
        uie::splitter_item_ptr ptr;
        old_splitter->get_panel(index, ptr);
        new_splitter->add_panel(ptr.get_ptr());
    }

    pfc::array_t<uint8_t> data;
    stream_writer_memblock_ref conf(data);

    try {
        new_splitter->get_config(&conf, fb2k::noAbort);
    } catch (const std::exception& ex) {
        console::print("Columns UI – error changing container type: ", ex.what());
        return {};
    }

    return std::make_optional(std::move(data));
}

uie::splitter_item_ptr create_splitter_item(GUID panel_id)
{
    uie::splitter_item_ptr item = new uie::splitter_item_simple_t;
    item->set_panel_guid(panel_id);
    return item;
}

uih::Menu create_panels_menu(const std::vector<cui::panel_utils::PanelInfo>& panels,
    uih::MenuCommandCollector& command_collector, std::function<void(GUID)> handle_command)
{
    uih::Menu menu;

    auto grouped_panels = cui::panel_utils::get_grouped_panel_info(panels);

    for (auto&& group : grouped_panels) {
        uih::Menu category_menu;

        for (auto&& [index, panel] : group) {
            category_menu.append_command(
                command_collector.add([panel_id{panel.id}, handle_command] { handle_command(panel_id); }), panel.name);
        }

        const auto& category = group.front().second.category;
        menu.append_submenu(std::move(category_menu), category);
    }

    return menu;
}

uih::Menu create_splitters_menu(const std::vector<cui::panel_utils::PanelInfo>& panels, GUID current_id,
    uih::MenuCommandCollector& command_collector, std::function<void(GUID)> handle_command)
{
    uih::Menu menu;

    for (auto&& [index, panel] : ranges::views::enumerate(panels)) {
        if (panel.type & uie::type_splitter)
            menu.append_command(
                command_collector.add([panel_id{panel.id}, handle_command] { handle_command(panel_id); }), panel.name,
                {.is_radio_checked = panel.id == current_id});
    }

    return menu;
}

void move_panel_up(const uie::splitter_window::ptr& splitter, const uie::window::ptr& panel)
{
    size_t index{};

    if (!splitter->find_by_ptr(panel, index) || index == 0)
        return;

    const auto panel_count = splitter->get_panel_count();

    uie::splitter_window_v3::ptr splitter_v3;

    if (splitter->service_query_t(splitter_v3)) {
        auto permutation = mmh::Permutation(panel_count);
        std::swap(permutation[index - 1], permutation[index]);
        splitter_v3->reorder_panels(permutation.data(), panel_count);
    } else {
        splitter->move_up(index);
    }
}

void move_panel_down(const uie::splitter_window::ptr& splitter, const uie::window::ptr& panel)
{
    size_t index{};

    const auto panel_count = splitter->get_panel_count();

    if (!splitter->find_by_ptr(panel, index) || index + 1 >= panel_count)
        return;

    uie::splitter_window_v3::ptr splitter_v3;

    if (splitter->service_query_t(splitter_v3)) {
        auto permutation = mmh::Permutation(panel_count);
        std::swap(permutation[index], permutation[index + 1]);
        splitter_v3->reorder_panels(permutation.data(), panel_count);
    } else {
        splitter->move_down(index);
    }
}

} // namespace

// {755971A7-109B-41dc-BED9-5A05CC07C905}
static const GUID g_guid_layout = {0x755971a7, 0x109b, 0x41dc, {0xbe, 0xd9, 0x5a, 0x5, 0xcc, 0x7, 0xc9, 0x5}};

ConfigLayout cfg_layout(g_guid_layout);

class LayoutWindowHost : public ui_extension::window_host {
public:
    const GUID& get_host_guid() const override
    {
        // {DA9A1375-A411-48a9-AF74-4AC29FF9BE9C}
        static const GUID ret = {0xda9a1375, 0xa411, 0x48a9, {0xaf, 0x74, 0x4a, 0xc2, 0x9f, 0xf9, 0xbe, 0x9c}};
        return ret;
    }

    void on_size_limit_change(HWND wnd, unsigned flags) override {}

    bool override_status_text_create(service_ptr_t<ui_status_text_override>& p_out) override
    {
        const auto api = ui_control::get();
        return api->override_status_text_create(p_out);
    }

    unsigned is_resize_supported(HWND wnd) const override { return false; }

    bool request_resize(HWND wnd, unsigned flags, unsigned width, unsigned height) override
    {
        bool rv = false;
        return rv;
    }
    bool is_visible(HWND wnd) const override
    {
        bool rv = IsWindowVisible(g_layout_window.get_wnd()) != 0;
        return rv;
    }
    bool is_visibility_modifiable(HWND wnd, bool desired_visibility) const override
    {
        bool rv = false;
        return rv;
    }
    bool set_window_visibility(HWND wnd, bool visibility) override
    {
        bool rv = false;
        return rv;
    }

    void relinquish_ownership(HWND wnd) override { g_layout_window.relinquish_child(); }
};

ui_extension::window_host_factory_single<LayoutWindowHost> g_window_host_layout_factory;

bool LayoutWindow::set_focus()
{
    return __set_focus_recur(m_child);
}

void LayoutWindow::show_window()
{
    ShowWindow(m_child_wnd, SW_SHOWNORMAL);
    ShowWindow(get_wnd(), SW_SHOWNORMAL);
}

bool LayoutWindow::__set_focus_recur(const uie::window_ptr& p_wnd)
{
    service_ptr_t<uie::playlist_window> p_playlist_wnd;
    service_ptr_t<uie::splitter_window> p_splitter_wnd;
    if (p_wnd.is_valid()) {
        if (p_wnd->service_query_t(
                p_playlist_wnd)) // && (GetWindowLongPtr(p_playlist_wnd->get_wnd(), GWL_STYLE) & WS_VISIBLE)) //we
                                 // cheat: IsWindowVisible checks parent/main win visibility as well
        {
            p_playlist_wnd->set_focus();
            return true;
        }
        if (p_wnd->service_query_t(p_splitter_wnd)) {
            const auto count = p_splitter_wnd->get_panel_count();
            for (size_t n = 0; n < count; n++) {
                uie::splitter_item_ptr temp;
                p_splitter_wnd->get_panel(n, temp);
                if (temp.is_valid() && __set_focus_recur(temp->get_window_ptr()))
                    return true;
            }
        }
    }
    return false;
}

void LayoutWindow::show_menu_access_keys()
{
    return __show_menu_access_keys_recur(m_child);
}

void LayoutWindow::__show_menu_access_keys_recur(const uie::window_ptr& p_wnd)
{
    service_ptr_t<uie::menu_window> p_menu_wnd;
    service_ptr_t<uie::splitter_window> p_splitter_wnd;
    if (p_wnd.is_valid()) {
        if (p_wnd->service_query_t(p_menu_wnd)) {
            p_menu_wnd->show_accelerators();
        } else if (p_wnd->service_query_t(p_splitter_wnd)) {
            const auto count = p_splitter_wnd->get_panel_count();
            for (size_t n = 0; n < count; n++) {
                uie::splitter_item_ptr temp;
                p_splitter_wnd->get_panel(n, temp);
                if (temp.is_valid())
                    __show_menu_access_keys_recur(temp->get_window_ptr());
            }
        }
    }
}

void LayoutWindow::hide_menu_access_keys()
{
    __hide_menu_access_keys_recur(m_child);
}

bool LayoutWindow::on_menu_char(unsigned short c)
{
    return __on_menu_char_recur(m_child, c);
}

bool LayoutWindow::__on_menu_char_recur(const uie::window_ptr& p_wnd, unsigned short c)
{
    service_ptr_t<uie::menu_window> p_menu_wnd;
    service_ptr_t<uie::splitter_window> p_splitter_wnd;
    if (p_wnd.is_valid()) {
        if (p_wnd->service_query_t(p_menu_wnd)) {
            if (p_menu_wnd->on_menuchar(c))
                return true;
        } else if (p_wnd->service_query_t(p_splitter_wnd)) {
            const auto count = p_splitter_wnd->get_panel_count();
            for (size_t n = 0; n < count; n++) {
                uie::splitter_item_ptr temp;
                p_splitter_wnd->get_panel(n, temp);
                if (temp.is_valid() && __on_menu_char_recur(temp->get_window_ptr(), c))
                    return true;
            }
        }
    }
    return false;
}

bool LayoutWindow::set_menu_focus()
{
    return __set_menu_focus_recur(m_child);
}

bool LayoutWindow::__set_menu_focus_recur(const uie::window_ptr& p_wnd)
{
    bool ret = false;
    service_ptr_t<uie::menu_window> p_menu_wnd;
    service_ptr_t<uie::splitter_window> p_splitter_wnd;
    if (p_wnd.is_valid()) {
        if (p_wnd->service_query_t(p_menu_wnd)) {
            if (!ret) {
                p_menu_wnd->set_focus();
                ret = true;
            } else {
                p_menu_wnd->hide_accelerators();
            }
        } else if (p_wnd->service_query_t(p_splitter_wnd)) {
            const auto count = p_splitter_wnd->get_panel_count();
            for (size_t n = 0; n < count; n++) {
                uie::splitter_item_ptr temp;
                p_splitter_wnd->get_panel(n, temp);
                if (temp.is_valid()) {
                    if (!ret)
                        ret = __set_menu_focus_recur(temp->get_window_ptr());
                    else
                        __hide_menu_access_keys_recur(temp->get_window_ptr());
                }
            }
        }
    }
    return ret;
}

void LayoutWindow::set_layout_editing_active(bool b_val)
{
    if (b_val) {
        if (!m_layout_editing_active)
            enter_layout_editing_mode();
        m_layout_editing_active = true;
    } else {
        if (m_layout_editing_active)
            exit_layout_editing_mode();
        m_layout_editing_active = false;
    }
}
bool LayoutWindow::get_layout_editing_active()
{
    return m_layout_editing_active;
}

void LayoutWindow::enter_layout_editing_mode()
{
    if (get_wnd()) {
        register_message_hook(uih::MessageHookType::type_get_message, this);
        register_message_hook(uih::MessageHookType::type_mouse, this);
    }
    //__enter_layout_editing_mode_recur(m_child);
}

void LayoutWindow::exit_layout_editing_mode()
{
    deregister_message_hook(uih::MessageHookType::type_get_message, this);
    deregister_message_hook(uih::MessageHookType::type_mouse, this);
    //__exit_layout_editing_mode_recur(m_child);
}

bool LayoutWindow::is_menu_focused()
{
    return __is_menu_focused_recur(m_child);
}

bool LayoutWindow::__is_menu_focused_recur(const uie::window_ptr& p_wnd)
{
    service_ptr_t<uie::menu_window> p_menu_wnd;
    service_ptr_t<uie::splitter_window> p_splitter_wnd;
    if (p_wnd.is_valid()) {
        if (p_wnd->service_query_t(p_menu_wnd)) {
            if (p_menu_wnd->is_menu_focused())
                return true;
        } else if (p_wnd->service_query_t(p_splitter_wnd)) {
            const auto count = p_splitter_wnd->get_panel_count();
            for (size_t n = 0; n < count; n++) {
                uie::splitter_item_ptr temp;
                p_splitter_wnd->get_panel(n, temp);
                if (temp.is_valid()) {
                    if (__is_menu_focused_recur(temp->get_window_ptr()))
                        return true;
                }
            }
        }
    }
    return false;
}

HWND LayoutWindow::get_previous_menu_focus_window() const
{
    HWND ret = nullptr;
    __get_previous_menu_focus_window_recur(m_child, ret);
    return ret;
}

bool LayoutWindow::__get_previous_menu_focus_window_recur(const uie::window_ptr& p_wnd, HWND& wnd_previous) const
{
    service_ptr_t<uie::menu_window_v2> p_menu_wnd;
    service_ptr_t<uie::splitter_window> p_splitter_wnd;
    if (p_wnd.is_valid()) {
        if (p_wnd->service_query_t(p_menu_wnd)) {
            if (p_menu_wnd->is_menu_focused()) {
                wnd_previous = p_menu_wnd->get_previous_focus_window();
                return true;
            }
        } else if (p_wnd->service_query_t(p_splitter_wnd)) {
            const auto count = p_splitter_wnd->get_panel_count();
            for (size_t n = 0; n < count; n++) {
                uie::splitter_item_ptr temp;
                p_splitter_wnd->get_panel(n, temp);
                if (temp.is_valid()) {
                    if (__get_previous_menu_focus_window_recur(temp->get_window_ptr(), wnd_previous))
                        return true;
                }
            }
        }
    }
    return false;
}

void LayoutWindow::__hide_menu_access_keys_recur(const uie::window_ptr& p_wnd)
{
    service_ptr_t<uie::menu_window> p_menu_wnd;
    service_ptr_t<uie::splitter_window> p_splitter_wnd;
    if (p_wnd.is_valid()) {
        if (p_wnd->service_query_t(p_menu_wnd)) {
            p_menu_wnd->hide_accelerators();
        } else if (p_wnd->service_query_t(p_splitter_wnd)) {
            const auto count = p_splitter_wnd->get_panel_count();
            for (size_t n = 0; n < count; n++) {
                uie::splitter_item_ptr temp;
                p_splitter_wnd->get_panel(n, temp);
                if (temp.is_valid())
                    __hide_menu_access_keys_recur(temp->get_window_ptr());
            }
        }
    }
}

uie::splitter_item_ptr LayoutWindow::get_child() const
{
    auto item = create_splitter_item(m_child_guid);

    if (m_child.is_valid()) {
        stream_writer_memblock conf;
        abort_callback_dummy p_abort;
        m_child->get_config(&conf, p_abort);
        item->set_panel_config_from_ptr(conf.m_data.get_ptr(), conf.m_data.get_size());
    } else {
        item->set_panel_config_from_ptr(m_child_data.get_ptr(), m_child_data.get_size());
    }

    return item;
}
void LayoutWindow::set_child(const uie::splitter_item_t* item)
{
    if (get_wnd()) {
        SendMessage(get_wnd(), WM_SETREDRAW, FALSE, 0);
        destroy_child();
    }
    m_child_guid = item->get_panel_guid();
    item->get_panel_config_to_array(m_child_data, true);
    if (get_wnd()) {
        create_child();
        show_window();
        SendMessage(get_wnd(), WM_SETREDRAW, TRUE, 0);
        RedrawWindow(
            get_wnd(), nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_FRAME | RDW_ERASE);
    }
}

void __get_panel_list_recur(const uie::window_ptr& p_wnd, pfc::list_base_t<GUID>& p_out)
{
    service_ptr_t<uie::menu_window> p_menu_wnd;
    service_ptr_t<uie::splitter_window> p_splitter_wnd;
    if (p_wnd.is_valid()) {
        p_out.add_item(p_wnd->get_extension_guid());
        if (p_wnd->service_query_t(p_splitter_wnd)) {
            const auto count = p_splitter_wnd->get_panel_count();
            for (size_t n = 0; n < count; n++) {
                uie::splitter_item_ptr temp;
                p_splitter_wnd->get_panel(n, temp);
                if (temp.is_valid()) {
                    p_out.add_item(temp->get_panel_guid());
                    uie::window_ptr ptr = temp->get_window_ptr();
                    if (!ptr.is_valid()) {
                        if (uie::window::create_by_guid(temp->get_panel_guid(), ptr)) {
                            stream_writer_memblock w;
                            temp->get_panel_config(&w);
                            try {
                                abort_callback_dummy p_abort;
                                ptr->set_config_from_ptr(w.m_data.get_ptr(), w.m_data.get_size(), p_abort);
                            } catch (const exception_io&) {
                            }
                        } else {
                            throw cui::fcl::exception_missing_panel();
                        }
                    }
                    __get_panel_list_recur(ptr, p_out);
                }
            }
        }
    }
}

bool LayoutWindow::import_config_to_object(stream_reader* p_reader, size_t psize, uint32_t mode,
    ConfigLayout::Preset& p_out, pfc::list_base_t<GUID>& panels, abort_callback& p_abort)
{
    // uie::splitter_item_ptr item = new uie::splitter_item_simple_t;
    GUID guid;
    pfc::string8 name;
    p_reader->read_lendian_t(guid, p_abort);
    p_reader->read_string(name, p_abort);

    pfc::array_t<uint8_t> data, conf;
    uint32_t size;
    p_reader->read_lendian_t(size, p_abort);
    data.set_size(size);
    p_reader->read(data.get_ptr(), size, p_abort);

    p_out.m_guid = guid;
    p_out.m_name = name;

    if (guid == GUID{})
        return true;

    panels.add_item(guid);

    if (mode == cui::fcl::type_public) {
        uie::window_ptr wnd;
        if (uie::window::create_by_guid(guid, wnd)) {
            try {
                wnd->import_config_from_ptr(data.get_ptr(), data.get_size(), p_abort);
            } catch (const exception_io&) {
            }
            wnd->get_config_to_array(conf, p_abort);
            __get_panel_list_recur(wnd, panels);
        } else
            return false;

        p_out.m_val = conf;
    } else {
        p_out.m_val = data;
        uie::window_ptr wnd;
        if (uie::window::create_by_guid(guid, wnd)) {
            try {
                wnd->set_config_from_ptr(data.get_ptr(), data.get_size(), p_abort);
            } catch (const exception_io&) {
            }
            __get_panel_list_recur(wnd, panels);
        } else
            return false;
    }
    return true;

    // item->set_panel_guid(guid);
    // item->set_panel_config(&stream_reader_memblock_ref(conf.get_ptr(), conf.get_size()), conf.get_size());
    // p_out.set(item);
}

void LayoutWindow::export_config(
    stream_writer* p_out, uint32_t mode, pfc::list_base_t<GUID>& panels, abort_callback& p_abort)
{
    enum {
        stream_version = 0
    };
    p_out->write_lendian_t((uint32_t)stream_version, p_abort);
    size_t count = cfg_layout.get_presets().get_count();
    p_out->write_lendian_t(gsl::narrow<uint32_t>(cfg_layout.get_active()), p_abort);
    p_out->write_lendian_t(gsl::narrow<uint32_t>(count), p_abort);
    for (size_t i = 0; i < count; i++) {
        uie::splitter_item_ptr item;
        cfg_layout.get_preset(i, item);
        pfc::string8 name;
        cfg_layout.get_preset_name(i, name);
        try {
            if (mode == cui::fcl::type_public) {
                p_out->write_lendian_t(item->get_panel_guid(), p_abort);
                p_out->write_string(name, p_abort);

                const auto panel_id = item->get_panel_guid();

                if (panel_id == GUID{}) {
                    p_out->write_lendian_t(0u, p_abort);
                } else {
                    uie::window_ptr ptr;
                    if (!uie::window::create_by_guid(item->get_panel_guid(), ptr))
                        throw cui::fcl::exception_missing_panel();

                    panels.add_item(panel_id);

                    stream_writer_memblock writer;
                    stream_writer_memblock data;
                    item->get_panel_config(&data);
                    try {
                        ptr->set_config_from_ptr(data.m_data.get_ptr(), data.m_data.get_size(), p_abort);
                    } catch (const exception_io&) {
                    }
                    __get_panel_list_recur(ptr, panels);
                    ptr->export_config(&writer, p_abort);
                    p_out->write_lendian_t((uint32_t)writer.m_data.get_size(), p_abort);
                    p_out->write(writer.m_data.get_ptr(), (uint32_t)writer.m_data.get_size(), p_abort);
                }
            } else {
                p_out->write_lendian_t(item->get_panel_guid(), p_abort);
                p_out->write_string(name, p_abort);

                const auto panel_id = item->get_panel_guid();

                if (panel_id == GUID{}) {
                    p_out->write_lendian_t(0u, p_abort);
                } else {
                    panels.add_item(panel_id);
                    stream_writer_memblock writer;
                    item->get_panel_config(&writer);

                    uie::window_ptr ptr;
                    if (!uie::window::create_by_guid(panel_id, ptr))
                        throw cui::fcl::exception_missing_panel();
                    {
                        try {
                            ptr->set_config_from_ptr(writer.m_data.get_ptr(), writer.m_data.get_size(), p_abort);
                        } catch (const exception_io&) {
                        }
                        __get_panel_list_recur(ptr, panels);
                    }

                    p_out->write_lendian_t((uint32_t)writer.m_data.get_size(), p_abort);
                    p_out->write(writer.m_data.get_ptr(), (uint32_t)writer.m_data.get_size(), p_abort);
                }
            }
        } catch (const pfc::exception& ex) {
            pfc::string_formatter formatter;
            throw pfc::exception(formatter << "Error exporting layout preset \"" << name << "\" - " << ex.what());
        }
    }
}

void LayoutWindow::create_child()
{
    if (m_child_guid == GUID{} || !uie::window::create_by_guid(m_child_guid, m_child))
        return;

    RECT rc;
    GetClientRect(get_wnd(), &rc);

    try {
        abort_callback_dummy p_abort;
        m_child->set_config_from_ptr(m_child_data.get_ptr(), m_child_data.get_size(), p_abort);
    } catch (const exception_io& ex) {
        console::formatter formatter;
        formatter << "Error setting panel config: " << ex.what();
    }

    m_child_wnd = cui::panel_utils::create_or_transfer_window_safe("Columns UI"sv, m_child, get_wnd(),
        uie::window_host_ptr(&g_window_host_layout_factory.get_static_instance()), ui_helpers::window_position_t(rc),
        "root panel"sv);

    if (m_child_wnd) {
        SetWindowLongPtr(m_child_wnd, GWL_STYLE, GetWindowLongPtr(m_child_wnd, GWL_STYLE) | WS_CLIPSIBLINGS);
    }
}
void LayoutWindow::destroy_child()
{
    if (m_child_wnd && m_child.is_valid()) {
        abort_callback_dummy p_abort;
        m_child->get_config_to_array(m_child_data, p_abort, true);
        m_child->destroy_window();
        m_child.release();
    }
}

void LayoutWindow::relinquish_child()
{
    m_child_wnd = nullptr;
    m_child.release();
    m_child_data.set_size(0);
}

bool LayoutWindow::has_child() const
{
    return m_child_guid != GUID{};
}

void LayoutWindow::refresh_child()
{
    uie::splitter_item_ptr item;
    cfg_layout.get_active_preset_for_use(item);
    if (item.is_valid()) {
        m_child_guid = item->get_panel_guid();
        item->get_panel_config_to_array(m_child_data, true);
    } else {
        m_child_guid = columns_ui::panels::guid_playlist_view_v2;
        m_child_data.set_size(0);
    }
}

void LayoutWindow::run_live_edit_base_delayed(HWND wnd, POINT pt, pfc::list_t<uie::window::ptr>& p_hierarchy)
{
    m_live_edit_data.m_hierarchy = p_hierarchy;
    m_live_edit_data.m_wnd = wnd;
    m_live_edit_data.m_point = pt;
    PostMessage(get_wnd(), MSG_EDIT_PANEL, NULL, NULL);
}

void LayoutWindow::run_live_edit_base(LiveEditData p_data)
{
    if (m_trans_fill.get_wnd())
        return;

    size_t hierarchy_count = p_data.m_hierarchy.get_count();

    uie::window::ptr leaf = hierarchy_count > 0 ? p_data.m_hierarchy[hierarchy_count - 1] : uie::window::ptr{};

    uie::splitter_window_ptr leaf_splitter;

    if (leaf.is_valid())
        leaf->service_query_t(leaf_splitter);

    uie::splitter_window_ptr parent_splitter;

    if (hierarchy_count >= 2)
        p_data.m_hierarchy[hierarchy_count - 2]->service_query_t(parent_splitter);

    std::optional<RECT> overlay_rect;

    if (leaf.is_valid() || hierarchy_count == 0) {
        RECT rect{};
        GetRelativeRect(leaf.is_valid() ? leaf->get_wnd() : get_wnd(), HWND_DESKTOP, &rect);
        overlay_rect = rect;
    }

    if (overlay_rect) {
        const auto overlay_wnd = m_trans_fill.create(get_wnd(), uih::WindowPosition(*overlay_rect));

        cui::helpers::WindowEnum_t WindowEnum(GetAncestor(get_wnd(), GA_ROOT));
        WindowEnum.run();

        if (const auto count_owned = WindowEnum.m_wnd_list.get_count(); count_owned > 0)
            SetWindowPos(overlay_wnd, WindowEnum.m_wnd_list[count_owned - 1], 0, 0, 0, 0,
                SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

        ShowWindow(overlay_wnd, SW_SHOWNOACTIVATE);
    }

    const auto leaf_supported_panels = leaf_splitter.is_valid() ? get_supported_panel_info(leaf_splitter)
                                                                : std::vector<cui::panel_utils::PanelInfo>();

    const auto parent_supported_panels
        = parent_splitter.is_valid() ? get_supported_panel_info(parent_splitter) : cui::panel_utils::get_panel_info();

    uih::Menu menu;
    uih::MenuCommandCollector commands;

    if (hierarchy_count > 0) {
        pfc::string8 leaf_name;
        if (leaf.is_valid())
            leaf->get_name(leaf_name);
        else
            leaf_name = "Unknown panel";
        menu.append_command(0, mmh::to_utf16(leaf_name.c_str()), {.is_disabled = true});
        menu.append_separator();
    }

    const auto get_index_in_parent = [&]() -> std::optional<size_t> {
        size_t index{};
        if (parent_splitter.is_valid() && parent_splitter->find_by_ptr(p_data.m_hierarchy[hierarchy_count - 1], index))
            return index;

        return {};
    };

    const auto index_in_parent = get_index_in_parent();
    const auto splitter_item_in_clipboard = cui::splitter_utils::is_splitter_item_in_clipboard();
    const auto can_add_panel_to_leaf
        = leaf_splitter.is_valid() && leaf_splitter->get_panel_count() < leaf_splitter->get_maximum_panel_count();
    const auto leaf_id = leaf.is_valid() ? leaf->get_extension_guid() : GUID{};

    uie::splitter_item_ptr splitter_item;

    if (index_in_parent) {
        parent_splitter->get_panel(*index_in_parent, splitter_item);

        if (!leaf_splitter.is_valid()) {
            const auto show_caption = cui::splitter_utils::get_config_item<bool>(
                parent_splitter, *index_in_parent, uie::splitter_window::bool_show_caption);

            if (show_caption) {
                const auto show_caption_id = commands.add([&] {
                    toggle_splitter_item_boolean(parent_splitter, leaf, uie::splitter_window::bool_show_caption);
                });
                menu.append_command(show_caption_id, L"Show caption", {.is_checked = *show_caption});
            }
        }

        const auto is_locked = cui::splitter_utils::get_config_item<bool>(
            parent_splitter, *index_in_parent, uie::splitter_window::bool_locked);

        if (is_locked) {
            const auto locked_id = commands.add(
                [&] { toggle_splitter_item_boolean(parent_splitter, leaf, uie::splitter_window::bool_locked); });

            menu.append_command(locked_id, L"Locked", {.is_checked = *is_locked});
        }
    }

    if (leaf_splitter.is_valid()) {
        if (can_add_panel_to_leaf) {
            const auto handle_add_leaf_child
                = [&](GUID panel_id) { leaf_splitter->add_panel(create_splitter_item(panel_id).get_ptr()); };

            menu.append_submenu(
                create_panels_menu(leaf_supported_panels, commands, handle_add_leaf_child), L"Add child");
        }

        if (parent_splitter.is_valid()) {
            const auto handle_command = [&](GUID panel_id) {
                const auto config = convert_splitter_and_get_config(get_wnd(), leaf_splitter, panel_id);

                const auto index = get_index_in_parent();

                if (!(config && index))
                    return;

                uie::splitter_item_ptr new_splitter_item;
                parent_splitter->get_panel(*index, new_splitter_item);

                new_splitter_item->set_panel_guid(panel_id);
                new_splitter_item->set_panel_config_from_ptr(config->get_ptr(), config->get_size());
                parent_splitter->replace_panel(*index, new_splitter_item.get_ptr());
            };

            menu.append_submenu(
                create_splitters_menu(parent_supported_panels, leaf_id, commands, handle_command), L"Container type");
        }
    }

    if (hierarchy_count <= 1) {
        if (leaf_splitter.is_valid()) {
            const auto handle_change_root_splitter = [&](GUID panel_id) {
                const auto config = convert_splitter_and_get_config(get_wnd(), leaf_splitter, panel_id);

                if (!config)
                    return;

                const auto new_splitter_item = get_child();
                new_splitter_item->set_panel_guid(panel_id);
                new_splitter_item->set_panel_config_from_ptr(config->get_ptr(), config->get_size());
                set_child(new_splitter_item.get_ptr());
            };

            menu.append_submenu(
                create_splitters_menu(parent_supported_panels, leaf_id, commands, handle_change_root_splitter),
                L"Container type");

            menu.append_separator();
        }

        if (has_child()) {
            const auto cut_id = commands.add([&] {
                const auto root_splitter_item = get_child();

                if (cui::splitter_utils::copy_splitter_item_to_clipboard_safe(
                        cui::main_window.get_wnd(), root_splitter_item.get_ptr())) {
                    set_child(create_splitter_item(GUID{}).get_ptr());
                }
            });
            menu.append_command(cut_id, L"Cut");

            const auto copy_id = commands.add([&] {
                const auto root_splitter_item = get_child();

                cui::splitter_utils::copy_splitter_item_to_clipboard_safe(
                    cui::main_window.get_wnd(), root_splitter_item.get_ptr());
            });
            menu.append_command(copy_id, L"Copy");
        } else {
            const auto handle_set_root = [&](GUID panel_id) {
                if (!has_child())
                    set_child(create_splitter_item(panel_id).get_ptr());
            };

            menu.append_submenu(create_panels_menu(parent_supported_panels, commands, handle_set_root), L"Add panel");
        }

        if (splitter_item_in_clipboard && !has_child()) {
            const auto paste_id = commands.add([&] {
                if (const auto clipboard_splitter_item
                    = cui::splitter_utils::get_splitter_item_from_clipboard_safe(cui::main_window.get_wnd())) {
                    set_child(clipboard_splitter_item.get());
                }
            });

            menu.append_command(paste_id, L"Paste");
        }

        if (has_child()) {
            menu.append_separator();

            menu.append_command(commands.add([&] {
                if (!leaf_splitter.is_valid()
                    || cui::dark::modal_info_box(get_wnd(), "Remove root panel",
                        "This will remove the root panel, including all children. Do you want to continue?",
                        uih::InfoBoxType::Neutral, uih::InfoBoxModalType::YesNo))
                    set_child(create_splitter_item(GUID{}).get_ptr());
            }),
                L"Remove");
        }
    }

    if (index_in_parent) {
        if (menu.size() > 2)
            menu.append_separator();

        uih::Menu move_submenu;

        if (*index_in_parent > 0)
            move_submenu.append_command(
                commands.add([&] { move_panel_up(parent_splitter, leaf); }), L"Previous position");

        if (*index_in_parent + 1 < parent_splitter->get_panel_count())
            move_submenu.append_command(
                commands.add([&] { move_panel_down(parent_splitter, leaf); }), L"Next position");

        if (move_submenu.size() > 0)
            menu.append_submenu(std::move(move_submenu), L"Move to");

        menu.append_command(commands.add([&] { parent_splitter->remove_panel(leaf); }), L"Remove");

        menu.append_separator();

        const auto cut_id = commands.add([&] {
            if (cui::splitter_utils::copy_splitter_item_to_clipboard_safe(
                    cui::main_window.get_wnd(), splitter_item.get_ptr())) {
                parent_splitter->remove_panel(leaf);
            }
        });
        menu.append_command(cut_id, L"Cut");

        const auto copy_id = commands.add([&] {
            cui::splitter_utils::copy_splitter_item_to_clipboard_safe(
                cui::main_window.get_wnd(), splitter_item.get_ptr());
        });
        menu.append_command(copy_id, L"Copy");

        uih::Menu paste_submenu;

        if (splitter_item_in_clipboard && can_add_panel_to_leaf) {
            const auto paste_add_id = commands.add([&] {
                if (const auto clipboard_splitter_item
                    = cui::splitter_utils::get_splitter_item_from_clipboard_safe(cui::main_window.get_wnd())) {
                    leaf_splitter->add_panel(clipboard_splitter_item.get());
                }
            });

            paste_submenu.append_command(paste_add_id, L"As child");
        }

        if (parent_splitter.is_valid()
            && parent_splitter->get_panel_count() < parent_splitter->get_maximum_panel_count()) {
            if (splitter_item_in_clipboard) {
                const auto paste_item = [&](size_t offset) {
                    const auto clipboard_splitter_item
                        = cui::splitter_utils::get_splitter_item_from_clipboard_safe(cui::main_window.get_wnd());

                    if (const auto index = get_index_in_parent(); index && clipboard_splitter_item)
                        parent_splitter->insert_panel(*index + offset, clipboard_splitter_item.get());
                };

                paste_submenu.append_command(commands.add([&] { paste_item(0); }), L"Before");
                paste_submenu.append_command(commands.add([&] { paste_item(1); }), L"After");
            }
        }

        if (paste_submenu.size() > 0)
            menu.append_submenu(std::move(paste_submenu), L"Paste");
    }

    const auto has_add_sibling_submenu
        = parent_splitter.is_valid() && parent_splitter->get_panel_count() < parent_splitter->get_maximum_panel_count();

    if (index_in_parent && (has_add_sibling_submenu || hierarchy_count > 1))
        menu.append_separator();

    if (has_add_sibling_submenu) {
        const auto handle_add_sibling = [&](GUID panel_id) {
            const auto new_splitter_item = create_splitter_item(panel_id);

            if (const auto index = get_index_in_parent())
                parent_splitter->insert_panel(*index + 1, new_splitter_item.get_ptr());
            else
                parent_splitter->add_panel(new_splitter_item.get_ptr());
        };

        menu.append_submenu(create_panels_menu(parent_supported_panels, commands, handle_add_sibling), L"Add sibling");
    }

    if (hierarchy_count > 1) {
        const auto edit_parent_id = commands.add([&] {
            m_live_edit_data = p_data;
            m_live_edit_data.m_hierarchy.remove_by_idx(hierarchy_count - 1);
            PostMessage(get_wnd(), MSG_EDIT_PANEL, NULL, NULL);
        });

        menu.append_command(edit_parent_id, L"Edit parent");
    }

    menu_helpers::win32_auto_mnemonics(menu.get());

    const auto cmd = menu.run(get_wnd(), p_data.m_point);
    if (overlay_rect)
        m_trans_fill.destroy();

    commands.execute(cmd);
}

bool LayoutWindow::on_hooked_message(uih::MessageHookType p_type, int code, WPARAM wp, LPARAM lp)
{
    if (p_type == uih::MessageHookType::type_get_message) {
        auto* lpmsg = (LPMSG)lp;
        if (lpmsg->message == WM_CONTEXTMENU) {
            if (lpmsg->hwnd == get_wnd() || IsChild(get_wnd(), lpmsg->hwnd)) {
                uie::splitter_window_v2_ptr sw2;
                if (m_child.is_valid()) {
                    m_child->service_query_t(sw2);

                    RECT rc;
                    GetRelativeRect(lpmsg->hwnd, HWND_DESKTOP, &rc);

                    POINT pt = {rc.left + wil::rect_width(rc) / 2, rc.top + wil::rect_height(rc) / 2};

                    pfc::list_t<uie::window::ptr> hierarchy;
                    if (!sw2.is_valid() || sw2->is_point_ours(lpmsg->hwnd, pt, hierarchy)) {
                        if (!sw2.is_valid())
                            hierarchy.add_item(m_child);
                        HWND wnd_panel = nullptr;
                        if (hierarchy.get_count()) {
                            wnd_panel = hierarchy[hierarchy.get_count() - 1]->get_wnd();

                            GetRelativeRect(wnd_panel, HWND_DESKTOP, &rc);
                            POINT pt = {rc.left + wil::rect_width(rc) / 2, rc.top + wil::rect_height(rc) / 2};

                            run_live_edit_base_delayed(wnd_panel, pt, hierarchy);
                        }

                        lpmsg->message = WM_NULL;
                        lpmsg->lParam = NULL;
                        lpmsg->wParam = NULL;
                        lpmsg->hwnd = nullptr;
                    }
                }
            }
        }
        return false;
    }
    if (p_type == uih::MessageHookType::type_mouse) {
        const auto* lpmhs = reinterpret_cast<LPMOUSEHOOKSTRUCT>(lp);
        if (lpmhs->hwnd != get_wnd() && !IsChild(get_wnd(), lpmhs->hwnd))
            return false;

        uie::splitter_window_v2_ptr splitter_v2;

        if (m_child.is_valid())
            m_child->service_query_t(splitter_v2);

        const auto is_rbutton_down = wp == WM_RBUTTONDOWN || wp == WM_NCRBUTTONDOWN;
        const auto is_rbutton_up = wp == WM_RBUTTONUP || wp == WM_NCRBUTTONUP;

        if (!is_rbutton_down && !is_rbutton_up)
            return false;

        pfc::list_t<uie::window::ptr> hierarchy;
        if (splitter_v2.is_valid() && !splitter_v2->is_point_ours(lpmhs->hwnd, lpmhs->pt, hierarchy))
            return false;

        if (is_rbutton_down) {
            SendMessage(lpmhs->hwnd, WM_CANCELMODE, NULL, NULL);
        } else {
            if (!splitter_v2.is_valid() && m_child.is_valid())
                hierarchy.add_item(m_child);

            if (!m_trans_fill.get_wnd())
                run_live_edit_base_delayed(lpmhs->hwnd, lpmhs->pt, hierarchy);
        }
        return true;
    }
    return false;
}

LRESULT LayoutWindow::on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE:
        refresh_child();
        create_child();
        break;
    case WM_WINDOWPOSCHANGED: {
        auto lpwp = (LPWINDOWPOS)lp;
        if (!(lpwp->flags & SWP_NOSIZE)) {
            if (m_child_wnd)
                SetWindowPos(m_child_wnd, nullptr, 0, 0, lpwp->cx, lpwp->cy, SWP_NOZORDER);
        }
    } break;
    case WM_SIZE:
        break;
    case WM_SETFOCUS:
        PostMessage(wnd, MSG_LAYOUT_SET_FOCUS, 0, 0);
        break;
    case MSG_EDIT_PANEL:
        run_live_edit_base(std::move(m_live_edit_data));
        break;
    case MSG_LAYOUT_SET_FOCUS:
        set_focus();
        break;
#if 0
    case WM_CONTEXTMENU:
        {
            if (m_layout_editing_active)
            {
                POINT pt = {GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
                if (pt.x == -1 && pt.y == -1)
                {
                    RECT rc;
                    GetRelativeRect(m_child_wnd, HWND_DESKTOP, &rc);

                    pt.x = rc.left + wil::rect_width(rc)/2;
                    pt.y = rc.top + wil::rect_height(rc)/2;
                }
                run_live_edit_base(pt);
            }
        }
        return 0;
#endif
    case WM_DESTROY:
        destroy_child();
        deregister_message_hook(uih::MessageHookType::type_get_message, this);
        deregister_message_hook(uih::MessageHookType::type_mouse, this);
        break;
    }
    return DefWindowProc(wnd, msg, wp, lp);
}
