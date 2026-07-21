#if !defined(PCI__PCIE_HPP)
#define PCI__PCIE_HPP

#include <primitives.h>
#include <AlopexOS/ACPI/acpi.hpp>

namespace AlopexOS {
    class PCIe;
}

class AlopexOS::PCIe {
public:
    static PCIe& get_instance();

    void initialize(const ACPI::MCFGHeader* mcfg = nullptr);
    void destroy();
    bool is_active() const;
    uptr find_nvme_bar0();

private:
    PCIe();
    ~PCIe();

    PCIe(const PCIe&) = delete;
    PCIe& operator=(const PCIe&) = delete;

    bool active;

    void scan_and_leach_resources();
    void release_resources();
    void outl(u16 port, u32 val);
    u32 inl(u16 port);
    u32 read32(u8 bus, u8 slot, u8 func, u16 offset);
    void write32(u8 bus, u8 slot, u8 func, u16 offset, u32 val);
    u32 config_read32(u8 bus, u8 slot, u8 func, u8 offset);
    void config_write32(u8 bus, u8 slot, u8 func, u8 offset, u32 val);
    void enable_bus_master(u8 bus, u8 slot, u8 func);
};

#endif // PCI/PCIE.HPP