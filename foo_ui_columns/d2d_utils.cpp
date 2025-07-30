#include "pch.h"

namespace cui::d2d {

wil::com_ptr<ID2D1Effect> create_colour_management_effect(const wil::com_ptr<ID2D1DeviceContext>& device_context,
    const wil::com_ptr<ID2D1ColorContext>& source_color_context,
    const wil::com_ptr<ID2D1ColorContext>& dest_color_context)
{
    wil::com_ptr<ID2D1Effect> colour_management_effect;
    THROW_IF_FAILED(device_context->CreateEffect(CLSID_D2D1ColorManagement, &colour_management_effect));

    if (device_context->IsBufferPrecisionSupported(D2D1_BUFFER_PRECISION_32BPC_FLOAT)) {
        THROW_IF_FAILED(
            colour_management_effect->SetValue(D2D1_COLORMANAGEMENT_PROP_QUALITY, D2D1_COLORMANAGEMENT_QUALITY_BEST));
    }

    if (source_color_context) {
        THROW_IF_FAILED(colour_management_effect->SetValue(
            D2D1_COLORMANAGEMENT_PROP_SOURCE_COLOR_CONTEXT, source_color_context.get()));
    }

    if (dest_color_context) {
        THROW_IF_FAILED(colour_management_effect->SetValue(
            D2D1_COLORMANAGEMENT_PROP_DESTINATION_COLOR_CONTEXT, dest_color_context.get()));
    }

    return colour_management_effect;
}

} // namespace cui::d2d
