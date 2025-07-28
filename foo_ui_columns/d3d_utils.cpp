#include "pch.h"

namespace cui::d3d {

HRESULT create_d3d_device(D3D_DRIVER_TYPE driver_type, std::span<const D3D_FEATURE_LEVEL> feature_levels,
    ID3D11Device** device, ID3D11DeviceContext** device_context)
{
#ifdef _DEBUG
    const auto hr = D3D11CreateDevice(nullptr, driver_type, nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG, feature_levels.data(),
        gsl::narrow<uint32_t>(std::size(feature_levels)), D3D11_SDK_VERSION, device, nullptr, device_context);

    if (SUCCEEDED(hr) || (hr != DXGI_ERROR_SDK_COMPONENT_MISSING && hr != E_FAIL))
        return hr;

    console::print("Columns UI – Direct3D debug layer not installed");
#endif

    return D3D11CreateDevice(nullptr, driver_type, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, feature_levels.data(),
        gsl::narrow<uint32_t>(std::size(feature_levels)), D3D11_SDK_VERSION, device, nullptr, device_context);
}

} // namespace cui::d3d
