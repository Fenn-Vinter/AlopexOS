#include <AlopexOS/ACPI/ecam.hpp>

namespace AlopexOS::PCI {

static ACPI::ECAMMap g_ecam_map{};

auto ECAM::init(const ACPI::ECAMMap& map) -> bool {
    if (map.count == 0) {
        return false;
    }

    g_ecam_map = map;
    return true;
}

auto ECAM::get_device_address(u16 segment, u8 bus, u8 slot, u8 func, u16 offset) -> uptr {
    if (offset >= 4096) {
        return 0;
    }

    for (usize i = 0; i < g_ecam_map.count; ++i) {
        const auto& region = g_ecam_map.regions[i];

        if (region.segment_group == segment && bus >= region.start_bus && bus <= region.end_bus) {
            u64 bus_offset = static_cast<u64>(bus - region.start_bus);
            u64 dev_offset = static_cast<u64>(slot);
            u64 func_offset = static_cast<u64>(func);

            u64 relative_address = ((bus_offset << 20) | (dev_offset << 15) | (func_offset << 12)) + offset;
            return static_cast<uptr>(region.base_address + relative_address);
        }
    }

    return 0;
}

auto ECAM::read32(u16 segment, u8 bus, u8 slot, u8 func, u16 offset) -> u32 {
    uptr address = get_device_address(segment, bus, slot, func, offset);
    if (!address) {
        return 0xFFFFFFFF;
    }

    return *reinterpret_cast<volatile u32*>(address);
}

auto ECAM::write32(u16 segment, u8 bus, u8 slot, u8 func, u16 offset, u32 value) -> void {
    uptr address = get_device_address(segment, bus, slot, func, offset);
    if (!address) {
        return;
    }

    *reinterpret_cast<volatile u32*>(address) = value;
}

auto ECAM::read16(u16 segment, u8 bus, u8 slot, u8 func, u16 offset) -> u16 {
    uptr address = get_device_address(segment, bus, slot, func, offset);
    if (!address) {
        return 0xFFFF;
    }

    return *reinterpret_cast<volatile u16*>(address);
}

auto ECAM::write16(u16 segment, u8 bus, u8 slot, u8 func, u16 offset, u16 value) -> void {
    uptr address = get_device_address(segment, bus, slot, func, offset);
    if (!address) {
        return;
    }

    *reinterpret_cast<volatile u16*>(address) = value;
}

auto ECAM::read8(u16 segment, u8 bus, u8 slot, u8 func, u16 offset) -> u8 {
    uptr address = get_device_address(segment, bus, slot, func, offset);
    if (!address) {
        return 0xFF;
    }

    return *reinterpret_cast<volatile u8*>(address);
}

auto ECAM::write8(u16 segment, u8 bus, u8 slot, u8 func, u16 offset, u8 value) -> void {
    uptr address = get_device_address(segment, bus, slot, func, offset);
    if (!address) {
        return;
    }

    *reinterpret_cast<volatile u8*>(address) = value;
}

}