#pragma once

namespace cui::d2d {

wil::com_ptr<ID2D1Factory1> create_factory(D2D1_FACTORY_TYPE factory_type);

using MainThreadD2D1Factory = std::shared_ptr<wil::com_ptr<ID2D1Factory1>>;
MainThreadD2D1Factory create_main_thread_factory();

wil::com_ptr<ID2D1Effect> create_colour_management_effect(const wil::com_ptr<ID2D1DeviceContext>& device_context,
    const wil::com_ptr<ID2D1ColorContext>& source_color_context,
    const wil::com_ptr<ID2D1ColorContext>& dest_color_context);

wil::com_ptr<ID2D1Effect> create_scale_effect(
    const wil::com_ptr<ID2D1DeviceContext>& device_context, D2D1_VECTOR_2F scale);

wil::com_ptr<ID2D1Effect> create_white_level_adjustment_effect(const wil::com_ptr<ID2D1DeviceContext>& device_context,
    std::optional<float> input_white_level, std::optional<float> output_white_level);

} // namespace cui::d2d
