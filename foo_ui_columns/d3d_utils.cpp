#include "pch.h"

namespace cui::d3d {

wil::com_ptr<ID3D11Device> create_d3d_device(
    const std::span<const D3D_FEATURE_LEVEL> feature_levels, ID3D11DeviceContext** device_context)
{
    if (!config::use_hardware_acceleration)
        return uih::d3d::create_d3d_device(D3D_DRIVER_TYPE_WARP, feature_levels);

    try {
        return uih::d3d::create_d3d_device(D3D_DRIVER_TYPE_HARDWARE, feature_levels);
    } catch (const wil::ResultException&) {
        const auto device = uih::d3d::create_d3d_device(D3D_DRIVER_TYPE_WARP, feature_levels);
        console::print("Columns UI – failed to create a hardware Direct3D renderer, using a software renderer instead");
        return device;
    }
}

} // namespace cui::d3d
