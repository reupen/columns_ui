#pragma once

namespace cui::toolbars::spectrum_analyser {

constexpr GUID colour_client_id = {0xe91a4bd1, 0x9372, 0x4252, {0xab, 0xce, 0x4, 0x39, 0x4b, 0xdf, 0xc4, 0x24}};

enum class Mode {
    Standard,
    Bars,
};

enum class Scale {
    Linear,
    Logarithmic,
};

} // namespace cui::toolbars::spectrum_analyser
