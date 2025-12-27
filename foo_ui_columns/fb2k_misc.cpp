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

} // namespace cui::fb2k_utils
