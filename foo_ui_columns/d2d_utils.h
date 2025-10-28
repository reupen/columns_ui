#pragma once
#include "wic.h"

namespace cui::d2d {

constexpr bool is_device_reset_error(const HRESULT hr)
{
    return hr == D2DERR_RECREATE_TARGET || hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET;
}

wil::com_ptr<ID2D1Factory1> create_factory(D2D1_FACTORY_TYPE factory_type);

using MainThreadD2D1Factory = std::shared_ptr<wil::com_ptr<ID2D1Factory1>>;
MainThreadD2D1Factory create_main_thread_factory();

wil::com_ptr<ID2D1Effect> create_colour_management_effect(const wil::com_ptr<ID2D1DeviceContext>& device_context,
    const wil::com_ptr<ID2D1ColorContext>& source_color_context,
    const wil::com_ptr<ID2D1ColorContext>& dest_color_context,
    std::optional<D2D1_COLORMANAGEMENT_RENDERING_INTENT> source_rendering_intent = {},
    std::optional<D2D1_COLORMANAGEMENT_RENDERING_INTENT> dest_rendering_intent = {});

wil::com_ptr<ID2D1Effect> create_scale_effect(
    const wil::com_ptr<ID2D1DeviceContext>& device_context, D2D1_VECTOR_2F scale);

wil::com_ptr<ID2D1Effect> create_2d_affine_transform_effect(
    const wil::com_ptr<ID2D1DeviceContext>& device_context, D2D1_MATRIX_3X2_F matrix);

D2D1_MATRIX_3X2_F create_orientation_transform_matrix(
    wic::PhotoOrientation orientation, D2D1_SIZE_F input_size, D2D1_VECTOR_2F scaling_factor);

wil::com_ptr<ID2D1Effect> create_white_level_adjustment_effect(const wil::com_ptr<ID2D1DeviceContext>& device_context,
    std::optional<float> input_white_level, std::optional<float> output_white_level);

} // namespace cui::d2d
