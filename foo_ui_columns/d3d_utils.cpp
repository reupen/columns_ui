#include "pch.h"

namespace cui::d3d {

HRESULT create_d3d_device(D3D_DRIVER_TYPE driver_type, std::span<const D3D_FEATURE_LEVEL> feature_levels,
    ID3D11Device** device, ID3D11DeviceContext** device_context)
{
    const auto base_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT
        | (driver_type == D3D_DRIVER_TYPE_HARDWARE ? D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS : 0);
#if CUI_ENABLE_D3D_D2D_DEBUG_LAYER == 1
    const auto hr = D3D11CreateDevice(nullptr, driver_type, nullptr, base_flags | D3D11_CREATE_DEVICE_DEBUG,
        feature_levels.data(), gsl::narrow<uint32_t>(std::size(feature_levels)), D3D11_SDK_VERSION, device, nullptr,
        device_context);

    if (SUCCEEDED(hr) || (hr != DXGI_ERROR_SDK_COMPONENT_MISSING && hr != E_FAIL))
        return hr;

    console::print("Columns UI – Direct3D debug layer not installed");
#endif

    return D3D11CreateDevice(nullptr, driver_type, nullptr, base_flags, feature_levels.data(),
        gsl::narrow<uint32_t>(std::size(feature_levels)), D3D11_SDK_VERSION, device, nullptr, device_context);
}

} // namespace cui::d3d
