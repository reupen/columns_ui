#pragma once

class StringPlaylistFormatName : private pfc::string8 {
    class PlaylistSwitcherTitleformatHook : public titleformat_hook {
        const char* m_name;
        bool m_locked;
        bool m_playing;
        bool m_active;
        unsigned m_size;
        service_ptr_t<playlist_manager_v3> m_api;
        service_ptr_t<metadb> m_metadb_api;
        unsigned m_index;
        bool m_lock_name_initialised;
        bool m_length_initialised;
        bool m_filesize_initialised;
        t_filesize m_filesize;
        pfc::string8 m_lock_name;
        pfc::string8 m_length;

    public:
        bool process_field(
            titleformat_text_out* p_out, const char* p_name, unsigned p_name_length, bool& p_found_flag) override;
        static int g_compare_metadb_handle_path(const metadb_handle_ptr& ptr1, const metadb_handle_ptr& ptr2)
        {
            return strcmp(ptr1->get_path(), ptr2->get_path());
        }
        void initialise_filesize()
        {
            if (!m_filesize_initialised) {
                metadb_handle_list_t<pfc::alloc_fast_aggressive> list;
                list.set_count(m_size);
                m_api->playlist_get_all_items(m_index, list);
                m_filesize = metadb_handle_list_helper::calc_total_size(list, true);
                m_filesize_initialised = true;
            }
        }

        bool process_function(titleformat_text_out* p_out, const char* p_name, unsigned p_name_length,
            titleformat_hook_function_params* p_params, bool& p_found_flag) override
        {
            p_found_flag = false;
            return false;
        }
        PlaylistSwitcherTitleformatHook(unsigned p_index, const char* p_name, t_size p_playing);
    };

public:
    StringPlaylistFormatName(unsigned p_index, const char* src, t_size p_playing);
    operator const char*() { return get_ptr(); }
};

namespace playlist_manager_utils {
void rename_playlist(size_t index, HWND wnd_parent);

bool check_clipboard();
bool cut(const pfc::list_base_const_t<t_size>& indices);
bool copy(const pfc::list_base_const_t<t_size>& indices);
bool cut(const pfc::bit_array& mask);
bool copy(const pfc::bit_array& mask);
bool paste(HWND wnd, t_size index_insert);
} // namespace playlist_manager_utils
