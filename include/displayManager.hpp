#pragma once

#include <fennlib/sequence>
#include <skeleton.hpp>
#include <limine/limine.h>
#include "screenBuffer.hpp"

namespace graphics {
    class DisplayManager {
        fennlib::sequence<ScreenBuffer*> active_displays;
        usize primary_index{0};

    public:
        auto update(const io::Skeleton& skel) -> void {
            auto* fb = skel.get_framebuffer();
            if (!fb) return;
            
            
            if (!active_displays.empty()) {
                
                return;
            }

            registerDisplay(
                static_cast<volatile u32*>(fb->address),
                static_cast<u32>(fb->width),
                static_cast<u32>(fb->height),
                static_cast<u64>(fb->pitch)
            );
        }

        auto registerDisplay(volatile u32* fb_addr, u32 width, u32 height, u64 pitch) -> void {
            
            for (usize i = 0; i < active_displays.size(); ++i) {
                
            }

            ScreenBuffer* new_screen = new ScreenBuffer(fb_addr, width, height, pitch);
            active_displays.push_back(new_screen);
        }

        auto removeDisplay(usize index) -> void {
            if (index >= active_displays.size()) return;

            delete active_displays[index];

            for (usize i = index; i < active_displays.size() - 1; ++i)
                active_displays[i] = active_displays[i + 1];

            active_displays.pop_back();

            if (primary_index >= active_displays.size() && primary_index > 0)
                primary_index = active_displays.size() - 1;
        }

        auto flushAll() -> void {
            for (usize i = 0; i < active_displays.size(); ++i)
                active_displays[i]->flush();
        }

        auto getPrimaryDisplay() -> ScreenBuffer* {
            if (active_displays.empty()) return nullptr;
            return active_displays[primary_index];
        }
    };
}