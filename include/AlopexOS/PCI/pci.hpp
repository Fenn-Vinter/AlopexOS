#if !defined(ALOPEXOS_PCI_HPP)
#define ALOPEXOS_PCI_HPP

#include <primitives.hpp>

namespace AlopexOS::PCI {

struct HeaderType0 {
    u32 bar[6];
    u32 cardbus_cis_pointer;
    u16 subsystem_vendor_id;
    u16 subsystem_id;
    u32 expansion_rom_base;
    u8 capabilities_pointer;
    u8 reserved[7];
    u8 interrupt_line;
    u8 interrupt_pin;
    u8 min_gnt;
    u8 max_lat;
} __attribute__((packed));

struct HeaderType1 {
    u32 bar[2];
    u8 primary_bus_number;
    u8 secondary_bus_number;
    u8 subordinate_bus_number;
    u8 secondary_latency_timer;
    u8 io_base;
    u8 io_limit;
    u16 secondary_status;
    u16 memory_base;
    u16 memory_limit;
    u16 prefetchable_memory_base;
    u16 prefetchable_memory_limit;
    u32 prefetchable_base_upper32;
    u32 prefetchable_limit_upper32;
    u16 io_base_upper16;
    u16 io_limit_upper16;
    u8 capabilities_pointer;
    u8 reserved[3];
    u32 expansion_rom_base;
    u8 interrupt_line;
    u8 interrupt_pin;
    u16 bridge_control;
} __attribute__((packed));

struct CommonHeader {
    u16 vendor_id;
    u16 device_id;
    u16 command;
    u16 status;
    u8 revision_id;
    u8 program_interface;
    u8 subclass;
    u8 class_code;
    u8 cache_line_size;
    u8 latency_timer;
    u8 header_type;
    u8 bist;
} __attribute__((packed));

struct DeviceConfig {
    CommonHeader common;
    union {
        HeaderType0 type0;
        HeaderType1 type1;
    };
} __attribute__((packed));

struct DeviceAddress {
    u16 segment;
    u8 bus;
    u8 slot;
    u8 func;
};

class Device {
public:
    DeviceAddress address;
    uptr mmio_base;

    auto read16(u16 offset) const -> u16;
    auto read32(u16 offset) const -> u32;
    auto write16(u16 offset, u16 value) -> void;
    auto write32(u16 offset, u32 value) -> void;
    
    auto is_valid() const -> bool;
};

}

#endif