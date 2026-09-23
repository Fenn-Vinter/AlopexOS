#pragma once
#include <fennlib/types>

namespace bootLogo {
    using namespace fennlib::types;

    inline constexpr u32 icon_width = 4;
    inline constexpr u32 icon_height = 4;
    
    inline constexpr u32 icon_data[16] = {
        0xFF000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFF000000,
        0xFFFFFFFF, 0xFF000000, 0xFF000000, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFF000000, 0xFF000000, 0xFFFFFFFF,
        0xFF000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFF000000
    };
}