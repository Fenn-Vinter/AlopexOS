#pragma once
#include <fennlib/types>

namespace test_programs {
    inline constexpr unsigned char sample_program[] = {
        0x48, 0xC7, 0xC0, 0x2A, 0x00, 0x00, 0x00, // mov rax, 42
        0xC3 // ret
    };

    template <fennlib::types::usize N>
    constexpr auto program_start(const unsigned char (&arr)[N]) -> fennlib::types::uintptr {
        return reinterpret_cast<fennlib::types::uintptr>(&arr[0]);
    }

    template <fennlib::types::usize N>
    constexpr auto program_end(const unsigned char (&arr)[N]) -> fennlib::types::uintptr {
        return reinterpret_cast<fennlib::types::uintptr>(&arr[N]);
    }
}