#include "pch.h"

namespace cui::d3d {

namespace {

wil::com_ptr<ID3D11Device> create_d3d_device(D3D_DRIVER_TYPE driver_type,
    std::span<const D3D_FEATURE_LEVEL> feature_levels, ID3D11DeviceContext** device_context = nullptr)
{
    wil::com_ptr<ID3D11Device> device;

    const auto base_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT
        | (driver_type == D3D_DRIVER_TYPE_HARDWARE ? D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS : 0);
#if CUI_ENABLE_D3D_D2D_DEBUG_LAYER == 1
    const auto hr = D3D11CreateDevice(nullptr, driver_type, nullptr, base_flags | D3D11_CREATE_DEVICE_DEBUG,
        feature_levels.data(), gsl::narrow<uint32_t>(std::size(feature_levels)), D3D11_SDK_VERSION, &device, nullptr,
        device_context);

    if (SUCCEEDED(hr))
        return device;

    if (hr != DXGI_ERROR_SDK_COMPONENT_MISSING && hr != E_FAIL)
        THROW_HR(hr);

    console::print("Columns UI – Direct3D debug layer not installed");
#endif

    THROW_IF_FAILED(D3D11CreateDevice(nullptr, driver_type, nullptr, base_flags, feature_levels.data(),
        gsl::narrow<uint32_t>(std::size(feature_levels)), D3D11_SDK_VERSION, &device, nullptr, device_context));

    return device;
}

} // namespace

wil::com_ptr<ID3D11Device> create_d3d_device(
    const std::span<const D3D_FEATURE_LEVEL> feature_levels, ID3D11DeviceContext** device_context)
{
    if (!config::use_hardware_acceleration)
        return create_d3d_device(D3D_DRIVER_TYPE_WARP, feature_levels);

    try {
        return create_d3d_device(D3D_DRIVER_TYPE_HARDWARE, feature_levels);
    } catch (const wil::ResultException&) {
        const auto device = create_d3d_device(D3D_DRIVER_TYPE_WARP, feature_levels);
        console::print("Columns UI – failed to create a hardware Direct3D renderer, using a software renderer instead");
        return device;
    }
}

} // namespace cui::d3d
