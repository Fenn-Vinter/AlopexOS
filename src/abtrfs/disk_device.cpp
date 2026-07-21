#include <AlopexOS/abtrfs/disk_device.hpp>

namespace AlopexOS {

    class ConcreteDiskDevice : public DiskDevice {
    public:
        ConcreteDiskDevice(u32 drive_port) : m_port(drive_port) {}

        auto read_blocks(u64 start_sector, u64 sector_count, void* buffer) -> bool override {
            // Route to your low-level storage controller driver (e.g., AHCI/NVMe read)
            // Example: return ahci_port_read(m_port, start_sector, sector_count, buffer);
            return false;
        }

        auto write_blocks(u64 start_sector, u64 sector_count, const void* buffer) -> bool override {
            // Route to your low-level storage controller driver write routine
            // Example: return ahci_port_write(m_port, start_sector, sector_count, buffer);
            return false;
        }

    private:
        u32 m_port;
    };

}