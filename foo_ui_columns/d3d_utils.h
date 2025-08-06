#pragma once

namespace cui::d3d {

wil::com_ptr<ID3D11Device> create_d3d_device(
    std::span<const D3D_FEATURE_LEVEL> feature_levels, ID3D11DeviceContext** device_context = nullptr);

}
