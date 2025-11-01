#include "pch.h"

#include "d2d_utils.h"

namespace cui::d2d {

namespace {

std::weak_ptr<wil::com_ptr<ID2D1Factory1>> weak_main_thread_factory;

}

wil::com_ptr<ID2D1Factory1> create_factory(D2D1_FACTORY_TYPE factory_type)
{
    wil::com_ptr<ID2D1Factory1> factory;
    D2D1_FACTORY_OPTIONS options{};

#if CUI_ENABLE_D3D_D2D_DEBUG_LAYER == 1
    options.debugLevel = IsDebuggerPresent() ? D2D1_DEBUG_LEVEL_INFORMATION : D2D1_DEBUG_LEVEL_NONE;
#endif

    THROW_IF_FAILED(D2D1CreateFactory(factory_type, options, &factory));

    return factory;
}

MainThreadD2D1Factory create_main_thread_factory()
{
    if (const auto factory = weak_main_thread_factory.lock())
        return factory;

    const auto factory
        = std::make_shared<wil::com_ptr<ID2D1Factory1>>(create_factory(D2D1_FACTORY_TYPE_SINGLE_THREADED));
    weak_main_thread_factory = factory;
    return factory;
}

wil::com_ptr<ID2D1Effect> create_colour_management_effect(const wil::com_ptr<ID2D1DeviceContext>& device_context,
    const wil::com_ptr<ID2D1ColorContext>& source_color_context,
    const wil::com_ptr<ID2D1ColorContext>& dest_color_context,
    std::optional<D2D1_COLORMANAGEMENT_RENDERING_INTENT> source_rendering_intent,
    std::optional<D2D1_COLORMANAGEMENT_RENDERING_INTENT> dest_rendering_intent)
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

    if (source_rendering_intent) {
        THROW_IF_FAILED(colour_management_effect->SetValue(
            D2D1_COLORMANAGEMENT_PROP_SOURCE_RENDERING_INTENT, *source_rendering_intent));
    }

    if (dest_rendering_intent) {
        THROW_IF_FAILED(colour_management_effect->SetValue(
            D2D1_COLORMANAGEMENT_PROP_DESTINATION_RENDERING_INTENT, *dest_rendering_intent));
    }

    return colour_management_effect;
}

wil::com_ptr<ID2D1Effect> create_scale_effect(
    const wil::com_ptr<ID2D1DeviceContext>& device_context, D2D1_VECTOR_2F scale)
{
    wil::com_ptr<ID2D1Effect> scale_effect;
    THROW_IF_FAILED(device_context->CreateEffect(CLSID_D2D1Scale, &scale_effect));
    THROW_IF_FAILED(
        scale_effect->SetValue(D2D1_SCALE_PROP_INTERPOLATION_MODE, D2D1_SCALE_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC));
    THROW_IF_FAILED(scale_effect->SetValue(D2D1_SCALE_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD));
    THROW_IF_FAILED(scale_effect->SetValue(D2D1_SCALE_PROP_SCALE, scale));

    return scale_effect;
}

wil::com_ptr<ID2D1Effect> create_2d_affine_transform_effect(
    const wil::com_ptr<ID2D1DeviceContext>& device_context, D2D1_MATRIX_3X2_F matrix)
{
    wil::com_ptr<ID2D1Effect> transform_effect;
    THROW_IF_FAILED(device_context->CreateEffect(CLSID_D2D12DAffineTransform, &transform_effect));
    THROW_IF_FAILED(transform_effect->SetValue(
        D2D1_2DAFFINETRANSFORM_PROP_INTERPOLATION_MODE, D2D1_2DAFFINETRANSFORM_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC));
    THROW_IF_FAILED(transform_effect->SetValue(D2D1_2DAFFINETRANSFORM_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD));
    THROW_IF_FAILED(transform_effect->SetValue(D2D1_2DAFFINETRANSFORM_PROP_TRANSFORM_MATRIX, matrix));

    return transform_effect;
}

D2D1_MATRIX_3X2_F create_orientation_transform_matrix(
    wic::PhotoOrientation orientation, D2D1_SIZE_F input_size, D2D1_VECTOR_2F scaling_factor)
{
    const auto [width, height] = input_size;

    return [&] {
        switch (orientation) {
        case wic::PhotoOrientation::FlipX:
            return D2D1::Matrix3x2F::Scale(-1.f, 1.f) * D2D1::Matrix3x2F::Translation(width, 0.f);
        case wic::PhotoOrientation::FlipY:
            return D2D1::Matrix3x2F::Scale(1.f, -1.f) * D2D1::Matrix3x2F::Translation(0.f, height);
        case wic::PhotoOrientation::Rotate90:
            return D2D1::Matrix3x2F::Rotation(90.f) * D2D1::Matrix3x2F::Translation(height, 0.f);
        case wic::PhotoOrientation::Rotate180:
            return D2D1::Matrix3x2F::Rotation(180.f) * D2D1::Matrix3x2F::Translation(width, height);
        case wic::PhotoOrientation::Rotate270:
            return D2D1::Matrix3x2F::Rotation(270.f) * D2D1::Matrix3x2F::Translation(0.f, width);
        case wic::PhotoOrientation::FlipXAndRotate270:
            return D2D1::Matrix3x2F::Scale(-1.f, 1.f) * D2D1::Matrix3x2F::Rotation(270.f);
        case wic::PhotoOrientation::FlipXAndRotate90:
            return D2D1::Matrix3x2F::Scale(-1.f, 1.f) * D2D1::Matrix3x2F::Rotation(90.f)
                * D2D1::Matrix3x2F::Translation(height, width);
        default:
            return D2D1::Matrix3x2F::Identity();
        }
    }() * D2D1::Matrix3x2F::Scale(scaling_factor.x, scaling_factor.y);
}

wil::com_ptr<ID2D1Effect> create_white_level_adjustment_effect(const wil::com_ptr<ID2D1DeviceContext>& device_context,
    std::optional<float> input_white_level, std::optional<float> output_white_level)
{
    wil::com_ptr<ID2D1Effect> white_level_adjustment_effect;
    THROW_IF_FAILED(device_context->CreateEffect(CLSID_D2D1WhiteLevelAdjustment, &white_level_adjustment_effect));

    if (input_white_level)
        THROW_IF_FAILED(white_level_adjustment_effect->SetValue(
            D2D1_WHITELEVELADJUSTMENT_PROP_INPUT_WHITE_LEVEL, *input_white_level));

    if (output_white_level)
        THROW_IF_FAILED(white_level_adjustment_effect->SetValue(
            D2D1_WHITELEVELADJUSTMENT_PROP_OUTPUT_WHITE_LEVEL, *output_white_level));

    return white_level_adjustment_effect;
}

} // namespace cui::d2d
