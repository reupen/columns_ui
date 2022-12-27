#pragma once

namespace cui::resource_utils {

std::tuple<void*, size_t> get_resource_data(WORD id, LPCWSTR type);

}
