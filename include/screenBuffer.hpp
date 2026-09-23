#pragma once
#include <fennlib/types>

namespace graphics {
    using namespace fennlib::types;

    class BufferLatch {
        u32 *buffer_a{nullptr};
        u32 *buffer_b{nullptr};
        u32 *buffer_ptr{nullptr};
        usize pixel_count{0};

    public:
        BufferLatch(usize total_pixels) : pixel_count(total_pixels) {
            buffer_a = new u32[pixel_count];
            buffer_b = new u32[pixel_count];
            buffer_ptr = buffer_a;
        }

        ~BufferLatch() {
            delete[] buffer_a;
            delete[] buffer_b;
        }

        auto getBuffer() -> u32* {
            return buffer_ptr;
        }

        auto swap() -> void {
            buffer_ptr = (buffer_ptr == buffer_a) ? buffer_b : buffer_a;
        }

        auto getReadBuffer() const -> u32* {
            return (buffer_ptr == buffer_a) ? buffer_b : buffer_a;
        }
    };

    class ScreenBuffer {
        volatile u32* front_buffer{nullptr};
        BufferLatch latch;
        u32 width{0}, height{0};
        u64 pitch{0};

    public:
        ScreenBuffer(volatile u32* fb_addr, u32 w, u32 h, u64 p)
            : front_buffer(fb_addr), latch((p / 4) * h), width(w), height(h), pitch(p) {}

        ~ScreenBuffer() = default;
        
        auto getWidth()  const -> u32 { return this->width; }
        auto getHeight() const -> u32 { return this->height; }
        auto getPitch()  const -> u64 { return this->pitch; }
        auto getBackBuffer() -> u32*  { return latch.getBuffer(); }

        auto clear(u32 color) -> void {
            usize total_pixels = (pitch / 4) * height;
            u32* buf = latch.getBuffer();
            for (usize i = 0; i < total_pixels; i++)
                buf[i] = color;
        }

        auto flush() -> void {
            usize total_pixels = (pitch / 4) * height;
            const u32* read_buf = latch.getReadBuffer();
            
            for (usize i = 0; i < total_pixels; i++)
                front_buffer[i] = read_buf[i];

            latch.swap();
        }
    };
}