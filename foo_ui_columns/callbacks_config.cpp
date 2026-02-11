#include "pch.h"
#include "main_window.h"

class AlwaysOnTopNotifyReceiver : public config_object_notify {
    size_t get_watched_object_count() override { return 1; }
    GUID get_watched_object(size_t p_index) override { return standard_config_objects::bool_ui_always_on_top; }
    void on_watched_object_changed(const service_ptr_t<config_object>& p_object) noexcept override
    {
        if (cui::main_window.get_wnd()) {
            bool aot = false;
            p_object->get_data_bool(aot);
            uPostMessage(cui::main_window.get_wnd(), MSG_SET_AOT, aot, 0);
        }
    }
};

service_factory_single_t<AlwaysOnTopNotifyReceiver> hj;
