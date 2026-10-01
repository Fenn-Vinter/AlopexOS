#pragma once
#include <fennlib/types>

namespace io::pcie {
    using namespace fennlib::types;

    constexpr u64 PCIE_CONFIG_SPACE_SIZE = 4096; // 4KB

    [[nodiscard]] inline auto get_ecam_address(u64 ecam_base, u8 bus, u8 device, u8 function, u16 offset) -> volatile u8* {
        u64 bus_offset = static_cast<u64>(bus) << 20;   // 1MB per bus
        u64 dev_offset = static_cast<u64>(device) << 15; // 32KB per device
        u64 func_offset = static_cast<u64>(function) << 12; // 4KB per function
        return reinterpret_cast<volatile u8*>(ecam_base + bus_offset + dev_offset + func_offset + (offset & 0xFFF));
    }

    [[nodiscard]] inline auto get_bar0_ecam(u64 ecam_base, u8 bus, u8 device, u8 function) -> u64 {
        volatile u8* bar0_addr = get_ecam_address(ecam_base, bus, device, function, 0x10);
        return *reinterpret_cast<volatile u32*>(bar0_addr);
    }

    [[nodiscard]] inline auto read32(u64 ecam_base, u8 bus, u8 device, u8 function, u16 offset) -> u32 {
        volatile u8* addr = get_ecam_address(ecam_base, bus, device, function, offset);
        return *reinterpret_cast<volatile u32*>(addr);
    }

    [[nodiscard]] inline auto read16(u64 ecam_base, u8 bus, u8 device, u8 function, u16 offset) -> u16 {
        volatile u8* addr = get_ecam_address(ecam_base, bus, device, function, offset);
        return *reinterpret_cast<volatile u16*>(addr);
    }

    [[nodiscard]] inline auto read8(u64 ecam_base, u8 bus, u8 device, u8 function, u16 offset) -> u8 {
        volatile u8* addr = get_ecam_address(ecam_base, bus, device, function, offset);
        return *addr;
    }

    constexpr u8 PCI_CAP_ID_PM = 0x01; // Power Management
    constexpr u8 PCI_CAP_ID_MSI = 0x05; // Message Signaled Interrupts
    constexpr u8 PCI_CAP_ID_PCIE = 0x10; // PCI Express Capability
    constexpr u8 PCI_CAP_ID_MSIX = 0x11; // MSI-X

    [[nodiscard]] inline auto find_capability(u64 ecam_base, u8 bus, u8 device, u8 function, u8 cap_id) -> u8 {
        u8 status = read8(ecam_base, bus, device, function, 0x06);
        if ((status & 0x10) == 0) {
            return 0;
        }

        u8 cap_ptr = read8(ecam_base, bus, device, function, 0x34) & ~3;
        int safety = 48;

        while (cap_ptr != 0 && safety > 0) {
            u8 current_id = read8(ecam_base, bus, device, function, cap_ptr);
            if (current_id == cap_id) {
                return cap_ptr;
            }
            cap_ptr = read8(ecam_base, bus, device, function, cap_ptr + 1) & ~3;
            safety--;
        }

        return 0;
    }
}