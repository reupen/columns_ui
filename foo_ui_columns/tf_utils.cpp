#include "pch.h"

#include "tf_utils.h"

namespace cui::tf {

bool is_field_used(const char* pattern, wil::zstring_view field)
{
    const auto tf_object = titleformat_compiler::get()->compile(pattern);

    if (!tf_object.is_valid())
        return false;

    return is_field_used(tf_object, field);
}

bool is_field_used(const titleformat_object::ptr& tf_object, wil::zstring_view field)
{
    titleformat_object_v2::ptr tf_object_v2;
    tf_object_v2 &= tf_object;

    if (!tf_object_v2.is_valid())
        return true;

    for (size_t index{}; true; ++index) {
        const char* used_field = tf_object_v2->enum_used_fields(index);

        if (!used_field)
            return false;

        if (!stricmp_utf8(field.c_str(), used_field))
            return true;
    }

    return false;
}

std::string_view get_param(titleformat_hook_function_params& params, size_t index)
{
    const char* param{};
    size_t param_length{};
    params.get_param(index, param, param_length);

    return {param, param_length};
}

} // namespace cui::tf
