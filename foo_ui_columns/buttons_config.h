#pragma once

namespace cui::toolbars::buttons {

enum class ConfigVersion {
    VERSION_1,
    VERSION_2,
    VERSION_CURRENT = VERSION_2
};

enum class FCBVersion {
    VERSION_1,
    VERSION_2,
    VERSION_3,
    VERSION_CURRENT = VERSION_3
};

/** For config dialog */
enum {
    MSG_BUTTON_CHANGE = WM_USER + 3,
    MSG_COMMAND_CHANGE = WM_USER + 4
};

} // namespace cui::toolbars::buttons

namespace pfc {

template <>
class traits_t<cui::toolbars::buttons::ConfigVersion> : public traits_rawobject {};

} // namespace pfc
