#include "pch.h"

namespace cui::wcs {

namespace {

namespace state {

std::mutex profile_names_mutex;
std::unordered_map<std::wstring, std::wstring> profile_names;

std::mutex profiles_mutex;
std::unordered_map<std::wstring, std::vector<uint8_t>> profiles;

} // namespace state

std::wstring read_display_colour_profile_name(const wchar_t* device_key)
{
    BOOL per_user{};

    if (!LOG_IF_WIN32_BOOL_FALSE(WcsGetUsePerUserProfiles(device_key, CLASS_MONITOR, &per_user)))
        return {};

    const auto scope = per_user ? WCS_PROFILE_MANAGEMENT_SCOPE_CURRENT_USER : WCS_PROFILE_MANAGEMENT_SCOPE_SYSTEM_WIDE;

    constexpr auto filename_buffer_size = MAX_PATH + 1u;
    constexpr auto filename_buffer_size_bytes = gsl::narrow<uint32_t>(filename_buffer_size * sizeof(wchar_t));
    std::wstring filename;
    filename.resize(filename_buffer_size);

    if (!WcsGetDefaultColorProfile(
            scope, device_key, CPT_ICC, CPST_NONE, 0, filename_buffer_size_bytes, filename.data())) {
        if (const auto last_error = GetLastError(); last_error != ERROR_FILE_NOT_FOUND)
            LOG_WIN32(last_error);

        return {};
    }

    filename.resize(wcslen(filename.c_str()));

    return filename;
}

std::vector<uint8_t> read_display_colour_profile(const std::wstring& filename)
{
    PROFILE profile_desc{};
    profile_desc.cbDataSize = gsl::narrow<uint32_t>((filename.size() + 1) * sizeof(wchar_t));
    profile_desc.dwType = PROFILE_FILENAME;
    profile_desc.pProfileData = const_cast<wchar_t*>(filename.data());

    const auto profile = WcsOpenColorProfile(
        &profile_desc, nullptr, nullptr, PROFILE_READ, FILE_SHARE_READ, OPEN_EXISTING, DONT_USE_EMBEDDED_WCS_PROFILES);

    if (!profile) {
#if _DEBUG
        console::print(
            fmt::format("Columns UI – WcsOpenColorProfile() for profile \"{}\" failed", mmh::to_utf8(filename))
                .c_str());
#endif
        return {};
    }

    auto _ = gsl::finally([profile] { CloseColorProfile(profile); });

    DWORD data_size{};
    GetColorProfileFromHandle(profile, nullptr, &data_size);

    std::vector<uint8_t> profile_data(data_size);
    if (!LOG_IF_WIN32_BOOL_FALSE(GetColorProfileFromHandle(profile, profile_data.data(), &data_size)))
        return {};

    return profile_data;
}

} // namespace

std::wstring get_display_colour_profile_name(const wchar_t* device_key)
{
    std::scoped_lock _(state::profile_names_mutex);

    if (const auto iter = state::profile_names.find(device_key); iter != state::profile_names.end())
        return iter->second;

    auto profile_data = read_display_colour_profile_name(device_key);
    state::profile_names.insert_or_assign(device_key, std::move(profile_data));

    return state::profile_names.at(device_key);
}

std::vector<uint8_t> get_display_colour_profile(const std::wstring& filename)
{
    if (filename.empty())
        return {};

    std::scoped_lock _(state::profiles_mutex);

    if (const auto iter = state::profiles.find(filename); iter != state::profiles.end())
        return iter->second;

    auto profile_data = read_display_colour_profile(filename);
    state::profiles.insert_or_assign(filename, std::move(profile_data));

    return state::profiles.at(filename);
}

void reset_colour_profiles()
{
    std::scoped_lock _(state::profile_names_mutex);
    state::profile_names.clear();
}

} // namespace cui::wcs
