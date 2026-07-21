#ifndef DISK_DEVICE_HPP
#define DISK_DEVICE_HPP

#include <primetives.h>

namespace AlopexOS {
    class DiskDevice {
    public:
        // Pure virtual interface for block operations (assuming 512-byte sectors or 4KB blocks)
        virtual auto read_blocks(u64 start_sector, u64 sector_count, void* buffer) -> bool = 0;
        virtual auto write_blocks(u64 start_sector, u64 sector_count, const void* buffer) -> bool = 0;
        
        virtual ~DiskDevice() = default;
    };
}

#endif