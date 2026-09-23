#pragma once

#include <fennlib/types>
#include "bootFont.hpp"

namespace bootPixel {
    using namespace fennlib::types;

    auto inline drawPixel(volatile u32* fb_address, u64 pitch, u16 x, u16 y, u32 color) -> void {
        volatile u8* row = reinterpret_cast<volatile u8*>(fb_address) + (y * pitch);
        volatile u32* pixel_ptr = reinterpret_cast<volatile u32*>(row) + x;
        *pixel_ptr = color;
    }

    auto inline drawImage(volatile u32* fb_address, u64 pitch, u16 start_x, u16 start_y, u16 width, u16 height, const u32* pixels) -> void {
        for (u16 y = 0; y < height; y++) {
            for (u16 x = 0; x < width; x++) {
                u32 color = pixels[y * width + x];
                
                if (color != 0xFFFF00FF) { 
                    drawPixel(fb_address, pitch, start_x + x, start_y + y, color);
                }
            }
        }
    }

    auto inline printText(volatile u32* fb_address, u64 pitch, u16 start_x, u16 start_y, const char* str, u32 color) -> void {
        u16 current_x = start_x;
        u16 current_y = start_y;

        while (*str != '\0') {
            char c = *str;

            if (c == '\n') {
                current_x = start_x;
                current_y += 10;
                str++;
                continue;
            }

            const fennlib::u8* glyph = bootFonts::char_to_font(c);

            for (u8 row = 0; row < 8; row++) {
                u8 row_byte = glyph[row];
                
                for (u8 bit = 0; bit < 8; bit++) {
                    if ((row_byte >> (7 - bit)) & 1) {
                        drawPixel(fb_address, pitch, current_x + bit, current_y + row, color);
                    }
                }
            }

            current_x += 9;
            str++;
        }
    }

    
    auto inline printSingleLine(volatile u32* fb_address, u64 pitch, u16 start_x, u16 start_y, const char* str, u32 color) -> void {
        u16 current_x = start_x;

        while (*str != '\0') {
            char c = *str;

            if (c == '\n') {
                str++;
                continue; 
            }

            const fennlib::u8* glyph = bootFonts::char_to_font(c);

            for (u8 row = 0; row < 8; row++) {
                u8 row_byte = glyph[row];
                
                for (u8 bit = 0; bit < 8; bit++) {
                    if ((row_byte >> (7 - bit)) & 1) {
                        drawPixel(fb_address, pitch, current_x + bit, start_y + row, color);
                    }
                }
            }

            current_x += 9;
            str++;
        }
    }

    auto inline drawLoadingIndicator(volatile u32* fb_address, u64 pitch, u16 start_x, u16 start_y, u32 frame_step) -> void {
        const u16 size = 256;
        const u16 center = size / 2;

        
        for (u16 y = 0; y < size; y++) {
            for (u16 x = 0; x < size; x++) {
                drawPixel(fb_address, pitch, start_x + x, start_y + y, 0xFF000000);
            }
        }

        
        const int dot_y = center;
        const int dot_spacing = 40;
        const int dot_x_coords[3] = {
            static_cast<int>(center) - dot_spacing,
            static_cast<int>(center),
            static_cast<int>(center) + dot_spacing
        };

        
        for (u16 y = 0; y < size; y++) {
            for (u16 x = 0; x < size; x++) {
                for (int d = 0; d < 3; d++) {
                    
                    u32 phase = (frame_step + (d * 21)) % 64;
                    u16 radius = 6 + (phase < 32 ? (phase * 12) / 32 : ((64 - phase) * 12) / 32);

                    int dx = static_cast<int>(x) - dot_x_coords[d];
                    int dy = static_cast<int>(y) - dot_y;

                    
                    if ((dx * dx + dy * dy) <= (radius * radius)) {
                        drawPixel(fb_address, pitch, start_x + x, start_y + y, 0xFFFF7F00);
                    }
                }
            }
        }
    }
}