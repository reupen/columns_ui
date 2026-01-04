#include "pch.h"

namespace cui::tf {

std::string_view get_param(titleformat_hook_function_params& params, size_t index)
{
    const char* param{};
    size_t param_length{};
    params.get_param(index, param, param_length);

    return {param, param_length};
}

} // namespace cui::tf
