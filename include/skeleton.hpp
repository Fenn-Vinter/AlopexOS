#pragma once
#include <fennlib/types>
#include <limine/limine.h>

namespace io {
    using namespace fennlib::types;

    class Skeleton {
        const struct limine_framebuffer* m_framebuffer{nullptr};
        const struct limine_memmap_response* m_memmap{nullptr};
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

        [[nodiscard]] inline auto get_memmap() const noexcept -> const struct limine_memmap_response* {
            return m_memmap;
        }

        [[nodiscard]] inline auto get_memmap_entry_count() const noexcept -> u64 {
            if (!m_memmap) return 0;
            return m_memmap->entry_count;
        }

        [[nodiscard]] inline auto get_memmap_entry(u64 index) const noexcept -> const struct limine_memmap_entry* {
            if (!m_memmap || index >= m_memmap->entry_count) return nullptr;
            return m_memmap->entries[index];
        }

        [[nodiscard]] inline auto get_memmap_entry_base(u64 index) const noexcept -> u64 {
            const auto* entry = get_memmap_entry(index);
            return entry ? entry->base : 0;
        }

        [[nodiscard]] inline auto get_memmap_entry_length(u64 index) const noexcept -> u64 {
            const auto* entry = get_memmap_entry(index);
            return entry ? entry->length : 0;
        }

        [[nodiscard]] inline auto get_memmap_entry_type(u64 index) const noexcept -> u32 {
            const auto* entry = get_memmap_entry(index);
            return entry ? static_cast<u32>(entry->type) : 0;
        }

        [[nodiscard]] inline auto get_memmap_entry_type_string(u64 index) const noexcept -> const char* {
            u32 type = get_memmap_entry_type(index);
            switch (type) {
                case LIMINE_MEMMAP_USABLE: return "Usable";
                case LIMINE_MEMMAP_RESERVED: return "Reserved";
                case LIMINE_MEMMAP_ACPI_RECLAIMABLE: return "ACPI Reclaimable";
                case LIMINE_MEMMAP_ACPI_NVS: return "ACPI NVS";
                case LIMINE_MEMMAP_BAD_MEMORY: return "Bad Memory";
                default: return "Unknown";
            }
        }

        [[nodiscard]] inline auto get_ecam_base() const noexcept -> u64 {
            for (u64 i = 0; i < get_memmap_entry_count(); ++i) {
                u32 type = get_memmap_entry_type(i);
                if (type == LIMINE_MEMMAP_USABLE) {
                    u64 base = get_memmap_entry_base(i);
                    u64 length = get_memmap_entry_length(i);
                    if (length >= 0x100000) { 
                        return base;
                    }
                }
            }
            return 0;
        }
    };
}