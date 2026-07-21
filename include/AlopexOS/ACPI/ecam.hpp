#if !defined(ALOPEXOS_ECAM_HPP)
#define ALOPEXOS_ECAM_HPP

#include <primitives.h>
#include "acpi.hpp"

namespace AlopexOS::PCI {

class ECAM {
public:
    static auto init(const ACPI::ECAMMap& map) -> bool;

    static auto read8(u16 segment, u8 bus, u8 slot, u8 func, u16 offset) -> u8;
    static auto read16(u16 segment, u8 bus, u8 slot, u8 func, u16 offset) -> u16;
    static auto read32(u16 segment, u8 bus, u8 slot, u8 func, u16 offset) -> u32;

    static auto write8(u16 segment, u8 bus, u8 slot, u8 func, u16 offset, u8 value) -> void;
    static auto write16(u16 segment, u8 bus, u8 slot, u8 func, u16 offset, u16 value) -> void;
    static auto write32(u16 segment, u8 bus, u8 slot, u8 func, u16 offset, u32 value) -> void;

private:
    static auto get_device_address(u16 segment, u8 bus, u8 slot, u8 func, u16 offset) -> uptr;
};

}

#endif