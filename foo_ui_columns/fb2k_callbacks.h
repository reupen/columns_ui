#pragma once

#include "event_token.h"

namespace cui::fb2k_utils {

using MetadbIoCallbackFunc = std::function<void(metadb_handle_list_cref, bool)>;

EventToken::Ptr add_metadb_io_callback(MetadbIoCallbackFunc callback);

} // namespace cui::fb2k_utils
