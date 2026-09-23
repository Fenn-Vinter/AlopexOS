#pragma once
#include <fennlib/types>

struct limine_framebuffer;

namespace io {
    using namespace fennlib::types;

    class Skeleton {
        const struct limine_framebuffer* m_framebuffer{nullptr};
        const void* m_memmap{nullptr};
        const void* m_rsdp{nullptr};
        u64 m_hhdm_offset{0};
        bool m_initialized{false};

    public:
        constexpr Skeleton() = default;

        void init();

        [[noreturn]] static inline void halt() noexcept {
            for (;;) {
                asm volatile ("hlt");
            }
        }

        [[nodiscard]] inline auto is_initialized() const noexcept -> bool {
            return m_initialized;
        }

        [[nodiscard]] inline auto get_framebuffer() const noexcept -> const struct limine_framebuffer* {
            return m_framebuffer;
        }

        [[nodiscard]] inline auto get_rsdp() const noexcept -> const void* {
            return m_rsdp;
        }

        [[nodiscard]] inline auto get_hhdm_offset() const noexcept -> u64 {
            return m_hhdm_offset;
        }
    };
}