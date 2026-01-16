#include "pch.h"

#include "tf_utils.h"

namespace cui::tf {

namespace {

bool is_name_used(const titleformat_object::ptr& tf_object, wil::zstring_view name, bool is_function)
{
    titleformat_object_v2::ptr tf_object_v2;
    tf_object_v2 &= tf_object;

    if (!tf_object_v2.is_valid())
        return true;

    for (size_t index{}; true; ++index) {
        const char* enumerated_name
            = is_function ? tf_object_v2->enum_used_functions(index) : tf_object_v2->enum_used_fields(index);

        if (!enumerated_name)
            return false;

        if (!stricmp_utf8(name.c_str(), enumerated_name))
            return true;
    }
}

} // namespace

bool is_field_used(const char* pattern, wil::zstring_view field)
{
    const auto tf_object = titleformat_compiler::get()->compile(pattern);

    if (!tf_object.is_valid())
        return false;

    return is_field_used(tf_object, field);
}

bool is_field_used(const titleformat_object::ptr& tf_object, wil::zstring_view field)
{
    return is_name_used(tf_object, field, false);
}

bool is_function_used(const titleformat_object::ptr& tf_object, wil::zstring_view field)
{
    return is_name_used(tf_object, field, true);
}

std::string_view get_param(titleformat_hook_function_params& params, size_t index)
{
    const char* param{};
    size_t param_length{};
    params.get_param(index, param, param_length);

    return {param, param_length};
}

} // namespace cui::tf
