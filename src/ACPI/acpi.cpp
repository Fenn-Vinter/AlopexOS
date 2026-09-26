#include <AlopexOS/ACPI/acpi.hpp>

namespace AlopexOS::ACPI {

bool parse_mcfg(const MCFGHeader* mcfg, ECAMMap& out_map) {
    if (!mcfg) {
        return false;
    }

    out_map.count = 0;

    const u8* ptr = reinterpret_cast<const u8*>(mcfg) + sizeof(MCFGHeader);
    const u8* end = reinterpret_cast<const u8*>(mcfg) + mcfg->length;

    while (ptr + sizeof(MCFGAllocationEntry) <= end && out_map.count < MaxECAMRegions) {
        const auto* entry = reinterpret_cast<const MCFGAllocationEntry*>(ptr);

        out_map.regions[out_map.count] = ECAMRegion{
            .base_address = entry->base_address,
            .segment_group = entry->segment_group,
            .start_bus = entry->start_bus,
            .end_bus = entry->end_bus
        };

        out_map.count++;
        ptr += sizeof(MCFGAllocationEntry);
    }

    return out_map.count > 0;
}

}