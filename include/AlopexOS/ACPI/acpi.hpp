#if !defined(ALOPEXOS_ACPI_HPP)
#define ALOPEXOS_ACPI_HPP

#include <primitives.h>

namespace AlopexOS::ACPI {

struct SDTHeader {
        char signature[4];
        u32 length;
        u8 revision;
        u8 checksum;
        char oem_id[6];
        char oem_table_id[8];
        u32 oem_revision;
        u32 creator_id;
        u32 creator_revision;
    } __attribute__((packed));

    struct RSDPDescriptor {
        char signature[8];
        u8 checksum;
        char oem_id[6];
        u8 revision;
        u32 rsdt_address;
    } __attribute__((packed));
    
struct MCFGAllocationEntry {
    u64 base_address;
    u16 segment_group;
    u8 start_bus;
    u8 end_bus;
    u32 reserved;
} __attribute__((packed));

struct MCFGHeader {
    char signature[4];
    u32 length;
    u8 revision;
    u8 checksum;
    char oem_id[6];
    char oem_table_id[8];
    u32 oem_revision;
    u32 creator_id;
    u32 creator_revision;
    u64 reserved;
} __attribute__((packed));

struct ECAMRegion {
    u64 base_address;
    u16 segment_group;
    u8 start_bus;
    u8 end_bus;
};

constexpr usize MaxECAMRegions = 16;

struct ECAMMap {
    ECAMRegion regions[MaxECAMRegions];
    usize count;
};

bool parse_mcfg(const MCFGHeader* mcfg, ECAMMap& out_map);

}

#endif