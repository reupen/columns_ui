#include "pch.h"

#include "resource_utils.h"

namespace cui::resource_utils {

std::tuple<void*, size_t> get_resource_data(WORD id, LPCWSTR type)
{
    // No need to free any of these
    const HRSRC resource = FindResource(wil::GetModuleInstanceHandle(), MAKEINTRESOURCE(id), type);
    auto size = SizeofResource(wil::GetModuleInstanceHandle(), resource);
    const HGLOBAL hglobal = LoadResource(wil::GetModuleInstanceHandle(), resource);
    void* data = LockResource(hglobal);

    return std::make_tuple(data, static_cast<size_t>(size));
}

} // namespace cui::resource_utils
