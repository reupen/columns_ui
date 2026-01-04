#pragma once

namespace cui::tf {

bool is_field_used(const titleformat_object::ptr& tf_object, wil::zstring_view field);
std::string_view get_param(titleformat_hook_function_params& params, size_t index);

} // namespace cui::tf
