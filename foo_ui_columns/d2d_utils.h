#pragma once

namespace cui::d2d {

wil::com_ptr<ID2D1Effect> create_colour_management_effect(const wil::com_ptr<ID2D1DeviceContext>& device_context,
    const wil::com_ptr<ID2D1ColorContext>& source_color_context,
    const wil::com_ptr<ID2D1ColorContext>& dest_color_context);

} // namespace cui::d2d
