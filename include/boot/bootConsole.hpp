#pragma once

#include <fennlib/types>
#include <fennlib/string>
#include <fennlib/sequence>
#include "bootPixel.hpp"

namespace bootConsole {
    using namespace fennlib::types;

    constexpr unsigned int BUFFER_HEIGHT = 60;
    inline fennlib::sequence<fennlib::string> text_buffer;

    inline auto write_boot_log(const char* text) -> void {
        
        if (text_buffer.size() >= BUFFER_HEIGHT) {
            text_buffer.pop_back();
        }
        
        text_buffer.push_back(text);
    }

    inline auto flush(volatile u32* fb_address, u64 pitch, u16 start_x, u16 start_y, u32 color) -> void {
        for (usize i = 0; i < text_buffer.size(); ++i) {
            const auto& line = text_buffer[i];
            if (!line.empty()) {
                bootPixel::printSingleLine(fb_address, pitch, start_x, start_y + (i * 10), line.c_str(), color);
            }
        }
    }
}