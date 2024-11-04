#include "pch.h"

#include "string.h"

namespace cui::string {

std::optional<float> safe_stof(const std::wstring& value)
{
    try {
        return std::stof(value);
    } catch (const std::exception&) {
        return {};
    }
}

std::optional<int> safe_stoi(const std::wstring& value)
{
    try {
        return std::stoi(value);
    } catch (const std::exception&) {
        return {};
    }
}

} // namespace cui::string
