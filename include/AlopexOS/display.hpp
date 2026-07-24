#if !defined(DISPLAY_HPP)
#define DISPLAY_HPP

#include <primitives.hpp>
#include <arr.hpp>
#include <limine.h>

namespace AlopexOS {
    class screen {
        public:
            screen();
            screen(limine_framebuffer* buffer);
            ~screen();

            auto getPixels() -> u32*;
            auto getPitch() -> u64;
            auto get_screen_size() -> uvector2D;
            
            auto set_pixel(vector2D<u32> coord, u32 color) -> void;
            auto draw_line(vector2D<u32> p0, vector2D<u32> p1, u32 color) -> void;
            auto draw_rect(vector2D<u32> pos, vector2D<u32> size, u32 color) -> void;
            auto fill_rect(vector2D<u32> pos, vector2D<u32> size, u32 color) -> void;
            auto clear(u32 color) -> void;
        private:
            limine_framebuffer* _frameBuffer = nullptr;
    };

    class displays {
        public:
            displays(const displays&) = delete;
            displays& operator=(const displays&) = delete;
            displays(displays&&) = delete;
            displays& operator=(displays&&) = delete;
            ~displays() = default;

            static auto initialize() -> displays&;
            static auto get_instance() -> displays&;

            auto get_screen_count() -> u8;
            auto get_screen(u8 id) -> screen*;
            auto get_screens() -> dynarr<screen>*;
        private:
            displays();

            inline static displays* instance = nullptr;
            limine_framebuffer_response* response = nullptr;
            dynarr<screen> _screen_buffer{};
    };
}

#endif