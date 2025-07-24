#pragma once

namespace cui::d3d {

HRESULT create_d3d_device(D3D_DRIVER_TYPE driver_type, std::span<const D3D_FEATURE_LEVEL> feature_levels,
    ID3D11Device** device, ID3D11DeviceContext** device_context = nullptr);

}
