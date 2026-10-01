#pragma once
#include <fennlib/types>

namespace io::pci {
    using namespace fennlib::types;

    constexpr u16 PCI_CONFIG_ADDRESS = 0xCF8;
    constexpr u16 PCI_CONFIG_DATA    = 0xCFC;

     inline auto outl(u16 port, u32 value) -> void {
        asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
    }

    [[nodiscard]] inline auto inl(u16 port) -> u32 {
        u32 value;
        asm volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
        return value;
    }

    [[nodiscard]] inline auto inw(u16 port) -> u16 {
        u16 value;
        asm volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
        return value;
    }

    [[nodiscard]] inline auto inb(u16 port) -> u8 {
        u8 value;
        asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
        return value;
    }

    [[nodiscard]] inline auto read32(u8 bus, u8 device, u8 function, u8 offset) -> u32 {
        u32 address = static_cast<u32>(
            (1U << 31) | 
            (static_cast<u32>(bus) << 16) | 
            (static_cast<u32>(device << 11)) | 
            (static_cast<u32>(function << 8)) | 
            (offset & 0xFC)
        );
        outl(PCI_CONFIG_ADDRESS, address);
        return inl(PCI_CONFIG_DATA);
    }

    [[nodiscard]] inline auto read16(u8 bus, u8 device, u8 function, u8 offset) -> u16 {
        u32 value = read32(bus, device, function, offset);
        return static_cast<u16>((value >> ((offset & 2) * 8)) & 0xFFFF);
    }

    [[nodiscard]] inline auto read8(u8 bus, u8 device, u8 function, u8 offset) -> u8 {
        u32 value = read32(bus, device, function, offset);
        return static_cast<u8>((value >> ((offset & 3) * 8)) & 0xFF);
    }

    struct PciDeviceInfo {
        u8 bus{};
        u8 device{};
        u8 function{};
        u16 vendor_id{};
        u16 device_id{};
        u8 class_code{};
        u8 subclass{};
        u64 bar0{};
    };

    [[nodiscard]] inline auto get_bar0(u8 bus, u8 device, u8 function) -> u64 {
        u32 bar_low = read32(bus, device, function, 0x10);
        if ((bar_low & 0x01) == 0) {
            u8 type = (bar_low >> 1) & 0x03;
            if (type == 0x02) {
                u32 bar_high = read32(bus, device, function, 0x14);
                return static_cast<u64>(bar_low & 0xFFFFFFF0) | (static_cast<u64>(bar_high) << 32);
            }
            return static_cast<u64>(bar_low & 0xFFFFFFF0);
        }
        return 0;
    }
}