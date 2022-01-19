#pragma once

#include "stdafx.h"
#include "file_info_reader.h"

namespace cui::panels::item_details {

extern const GUID g_guid_item_details;
extern const GUID g_guid_item_details_tracking_mode;
extern const GUID g_guid_item_details_script;
extern const GUID g_guid_item_details_font_client;
extern const GUID g_guid_item_details_colour_client;
extern const GUID g_guid_item_details_hscroll;
extern const GUID g_guid_item_details_horizontal_alignment;
extern const GUID g_guid_item_details_vertical_alignment;
extern const GUID g_guid_item_details_word_wrapping;
extern const GUID g_guid_item_details_edge_style;

extern cfg_uint cfg_item_details_tracking_mode;
extern cfg_uint cfg_item_details_edge_style;
extern cfg_bool cfg_item_details_hscroll;
extern cfg_uint cfg_item_details_horizontal_alignment;
extern cfg_uint cfg_item_details_vertical_alignment;
extern cfg_bool cfg_item_details_word_wrapping;

struct LineSize {
    t_size m_length{};
    int m_width{0};
    int m_height{0};
};

struct RawFont {
    std::wstring m_face;
    t_size m_point{10};
    bool m_bold{false};
    bool m_underline{false};
    bool m_italic{false};

    static bool s_are_equal(const RawFont& item1, const RawFont& item2);
};

bool operator==(const RawFont& item1, const RawFont& item2);

class RawFontChange {
public:
    RawFont m_font_data;
    bool m_reset{false};
    t_size m_character_index{NULL};

    RawFontChange() = default;
};

using RawFontChanges = pfc::list_t<RawFontChange, pfc::alloc_fast_aggressive>;

using LineLengths = std::vector<size_t>;
using LineSizes = std::vector<LineSize>;

class Font {
public:
    using Ptr = std::shared_ptr<Font>;

    RawFont m_raw_font;

    wil::unique_hfont m_font;
    int m_height{};
};

using Fonts = pfc::list_t<Font::Ptr>;

class FontChanges {
public:
    class FontChange {
    public:
        t_size m_text_index{};
        Font::Ptr m_font;
    };

    Fonts m_fonts;
    std::vector<FontChange> m_font_changes;
    Font::Ptr m_default_font;

    bool find_font(const RawFont& raw_font, t_size& index);

    void reset(bool keep_handles = false);
};

struct DisplayInfo {
    SIZE sz{};
    std::vector<LineSize> line_sizes;
};

DisplayInfo g_get_multiline_text_dimensions(HDC dc, std::wstring_view text, const LineLengths& line_lengths,
    const FontChanges& font_data, bool word_wrapping, int max_width);

std::wstring g_get_text_line_lengths(const wchar_t* text, LineLengths& line_lengths);

void g_parse_font_format_string(const wchar_t* str, t_size len, RawFont& p_out);
std::wstring g_get_raw_font_changes(const wchar_t* formatted_text, RawFontChanges& p_out);
void g_get_font_changes(const RawFontChanges& raw_font_changes, FontChanges& font_changes);

void g_text_out_multiline_font(HDC dc, RECT rc_placement, const wchar_t* text, const FontChanges& font_changes,
    const LineSizes& wrapped_line_sizes, COLORREF cr_text, uih::alignment align);

class TitleformatHookChangeFont : public titleformat_hook {
public:
    bool process_field(
        titleformat_text_out* p_out, const char* p_name, unsigned p_name_length, bool& p_found_flag) override;

    bool process_function(titleformat_text_out* p_out, const char* p_name, unsigned p_name_length,
        titleformat_hook_function_params* p_params, bool& p_found_flag) override;

    TitleformatHookChangeFont(const LOGFONT& lf);

private:
    pfc::string8 m_default_font_face;
    t_size m_default_font_size;
};

class FontCodeGenerator {
    class StringFontCode : private pfc::string8_fast_aggressive {
    public:
        operator const char*() const;
        StringFontCode(const LOGFONT& lf);
    };

public:
    void run(HWND parent, UINT edit);
    void initialise(const LOGFONT& p_lf_default, HWND parent, UINT edit);

private:
    LOGFONT m_lf{};
};

class ItemDetails
    : public uie::container_ui_extension
    , public ui_selection_callback
    , public play_callback
    , public playlist_callback_single
    , public metadb_io_callback_dynamic {
    class MessageWindow : public container_window {
        class_data& get_class_data() const override;
        LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;
    };

    static MessageWindow g_message_window;

    enum {
        MSG_REFRESH = WM_USER + 2,
    };

    class_data& get_class_data() const override;

public:
    enum TrackingMode {
        track_auto_playlist_playing,
        track_playlist,
        track_playing,
        track_auto_selection_playing,
        track_selection,
    };

    bool g_track_mode_includes_now_playing(t_size mode);

    bool g_track_mode_includes_plalist(t_size mode);

    bool g_track_mode_includes_auto(t_size mode);

    bool g_track_mode_includes_selection(t_size mode);

    class MenuNodeTrackMode : public ui_extension::menu_node_command_t {
        service_ptr_t<ItemDetails> p_this;
        t_size m_source;

    public:
        static const char* get_name(t_size source);
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        MenuNodeTrackMode(ItemDetails* p_wnd, t_size p_value);
        ;
    };

    class MenuNodeSourcePopup : public ui_extension::menu_node_popup_t {
        pfc::list_t<ui_extension::menu_node_ptr> m_items;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        unsigned get_children_count() const override;
        void get_child(unsigned p_index, uie::menu_node_ptr& p_out) const override;
        MenuNodeSourcePopup(ItemDetails* p_wnd);
        ;
    };

    class MenuNodeAlignment : public ui_extension::menu_node_command_t {
        service_ptr_t<ItemDetails> p_this;
        t_size m_type;

    public:
        static const char* get_name(t_size source);
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        MenuNodeAlignment(ItemDetails* p_wnd, t_size p_value);
        ;
    };

    class MenuNodeAlignmentPopup : public ui_extension::menu_node_popup_t {
        pfc::list_t<ui_extension::menu_node_ptr> m_items;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        unsigned get_children_count() const override;
        void get_child(unsigned p_index, uie::menu_node_ptr& p_out) const override;
        MenuNodeAlignmentPopup(ItemDetails* p_wnd);
        ;
    };

    class MenuNodeOptions : public ui_extension::menu_node_command_t {
        service_ptr_t<ItemDetails> p_this;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        MenuNodeOptions(ItemDetails* p_wnd);
        ;
    };
    class MenuNodeHorizontalScrolling : public ui_extension::menu_node_command_t {
        service_ptr_t<ItemDetails> p_this;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        MenuNodeHorizontalScrolling(ItemDetails* p_wnd);
        ;
    };

    class MenuNodeWordWrap : public ui_extension::menu_node_command_t {
        service_ptr_t<ItemDetails> p_this;

    public:
        bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override;
        bool get_description(pfc::string_base& p_out) const override;
        void execute() override;
        MenuNodeWordWrap(ItemDetails* p_wnd);
        ;
    };

    // UIE funcs
    const GUID& get_extension_guid() const override;
    void get_name(pfc::string_base& out) const override;
    void get_category(pfc::string_base& out) const override;
    unsigned get_type() const override;

    enum { stream_version_current = 2 };
    void set_config(stream_reader* p_reader, t_size p_size, abort_callback& p_abort) override;
    void get_config(stream_writer* p_writer, abort_callback& p_abort) const override;
    bool have_config_popup() const override;
    bool show_config_popup(HWND wnd_parent) override;

    void get_menu_items(ui_extension::menu_hook_t& p_hook) override;

    // UI SEL API
    void on_selection_changed(const pfc::list_base_const_t<metadb_handle_ptr>& p_selection) override;

    // PC
    void on_playback_starting(play_control::t_track_command p_command, bool p_paused) override;
    void on_playback_new_track(metadb_handle_ptr p_track) override;
    void on_playback_stop(play_control::t_stop_reason p_reason) override;
    void on_playback_seek(double p_time) override;
    void on_playback_pause(bool p_state) override;
    void on_playback_edited(metadb_handle_ptr p_track) override;
    void on_playback_dynamic_info(const file_info& p_info) override;
    void on_playback_dynamic_info_track(const file_info& p_info) override;
    void on_playback_time(double p_time) override;
    void on_volume_change(float p_new_val) override;

    // PL
    enum { playlist_callback_flags = flag_on_items_selection_change | flag_on_playlist_switch };
    void on_playlist_switch() override;
    void on_item_focus_change(t_size p_from, t_size p_to) override;

    void on_items_added(
        t_size p_base, const pfc::list_base_const_t<metadb_handle_ptr>& p_data, const bit_array& p_selection) override;
    void on_items_reordered(const t_size* p_order, t_size p_count) override;
    void on_items_removing(const bit_array& p_mask, t_size p_old_count, t_size p_new_count) override;
    void on_items_removed(const bit_array& p_mask, t_size p_old_count, t_size p_new_count) override;
    void on_items_selection_change(const bit_array& p_affected, const bit_array& p_state) override;
    void on_items_modified(const bit_array& p_mask) override;
    void on_items_modified_fromplayback(const bit_array& p_mask, play_control::t_display_level p_level) override;
    void on_items_replaced(const bit_array& p_mask,
        const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry>& p_data) override;
    void on_item_ensure_visible(t_size p_idx) override;

    void on_playlist_renamed(const char* p_new_name, t_size p_new_name_len) override;
    void on_playlist_locked(bool p_locked) override;

    void on_default_format_changed() override;
    void on_playback_order_changed(t_size p_new_index) override;

    // ML
    void on_changed_sorted(metadb_handle_list_cref p_items_sorted, bool p_fromhook) override;

    static void g_on_app_activate(bool b_activated);
    static void g_on_font_change();
    static void g_on_colours_change();

    void set_horizontal_alignment(t_size horizontal_alignment);
    void set_vertical_alignment(t_size vertical_alignment);

    void set_edge_style(t_size edge_style);

    void on_edge_style_change();

    void set_script(const char* p_script);

    void set_config_wnd(HWND wnd);

    ItemDetails();

private:
    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override;

    void register_callback();
    void deregister_callback();
    void on_app_activate(bool b_activated);

    void set_handles(const metadb_handle_list& handles);
    void refresh_contents(bool reset_vertical_scroll_position = false, bool reset_horizontal_scroll_position = false);
    void request_full_file_info();
    void on_full_file_info_request_completion(std::shared_ptr<helpers::FullFileInfoRequest> request);
    void release_aborted_full_file_info_requests();
    void release_all_full_file_info_requests();

    void update_font_change_info();
    void reset_font_change_info();
    void update_display_info(HDC dc);
    void update_display_info();
    void reset_display_info();
    void on_tracking_mode_change();
    bool check_process_on_selection_changed();

    void scroll(INT sb, int position, bool b_absolute);

    void invalidate_all(bool b_update = true);
    void update_now();

    enum class ScrollbarType {
        vertical = SB_VERT,
        horizontal = SB_HORZ,
    };

    void update_scrollbar(ScrollbarType scrollbar_type, bool reset_position);
    void update_scrollbars(bool reset_vertical_position, bool reset_horizontal_position);

    void on_size();
    void on_size(t_size cx, t_size cy);

    void on_font_change();
    void on_colours_change();

    static std::vector<ItemDetails*> g_windows;

    size_t m_last_cx{};
    size_t m_last_cy{};
    ui_selection_holder::ptr m_selection_holder;
    metadb_handle_list m_handles;
    metadb_handle_list m_selection_handles;
    std::shared_ptr<file_info_impl> m_full_file_info;
    std::shared_ptr<helpers::FullFileInfoRequest> m_full_file_info_request;
    std::vector<std::shared_ptr<helpers::FullFileInfoRequest>> m_aborting_full_file_info_requests;
    bool m_full_file_info_requested{};
    bool m_callback_registered{false};
    bool m_nowplaying_active{false};
    t_size m_tracking_mode;

    bool m_font_change_info_valid{false};
    FontChanges m_font_changes;
    RawFontChanges m_raw_font_changes;

    LineLengths m_line_lengths;
    LineSizes m_line_sizes;
    bool m_display_info_valid{false};

    SIZE m_display_sz{};

    pfc::string8 m_script;
    std::wstring m_current_text_raw;
    std::wstring m_current_text;
    std::wstring m_current_display_text;
    titleformat_object::ptr m_to;

    t_size m_horizontal_alignment;
    t_size m_vertical_alignment;
    t_size m_edge_style;
    bool m_hscroll;
    bool m_word_wrapping;

    HWND m_wnd_config{nullptr};
};

class ItemDetailsConfig {
public:
    pfc::string8 m_script;
    uint32_t m_edge_style{};
    uint32_t m_horizontal_alignment{};
    uint32_t m_vertical_alignment{};
    FontCodeGenerator m_font_code_generator;
    bool m_modal{};
    bool m_timer_active{};
    service_ptr_t<ItemDetails> m_this;
    HWND m_wnd{};

    enum { timer_id = 100 };

    ItemDetailsConfig(const char* p_text, uint32_t edge_style, uint32_t halign, uint32_t valign);

    ItemDetailsConfig(const ItemDetailsConfig&) = default;
    ItemDetailsConfig(ItemDetailsConfig&&) = default;

    bool run_modal(HWND wnd);
    void run_modeless(HWND wnd, ItemDetails* p_this) &&;

private:
    void kill_timer();
    void start_timer();
    void on_timer();
    BOOL CALLBACK on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
};

} // namespace cui::panels::item_details
