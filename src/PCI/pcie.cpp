#include <AlopexOS/PCI/pcie.hpp>
#include <AlopexOS/ACPI/ecam.hpp>
#include <AlopexOS/ACPI/acpi.hpp>

namespace AlopexOS {

PCIe& PCIe::get_instance() {
    static PCIe instance;
    return instance;
}

PCIe::PCIe() : active(false) {}

PCIe::~PCIe() {
    if (active) {
        destroy();
    }
}

void PCIe::outl(u16 port, u32 val) {
    asm volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

u32 PCIe::inl(u16 port) {
    u32 ret;
    asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

u32 PCIe::read32(u8 bus, u8 slot, u8 func, u16 offset) {
    u32 val = PCI::ECAM::read32(0, bus, slot, func, offset);
    if (val != 0xFFFFFFFF) {
        return val;
    }

    return config_read32(bus, slot, func, static_cast<u8>(offset));
}

void PCIe::write32(u8 bus, u8 slot, u8 func, u16 offset, u32 val) {
    uptr ecam_addr = PCI::ECAM::read32(0, bus, slot, func, 0x00);
    if (ecam_addr != 0xFFFFFFFF) {
        PCI::ECAM::write32(0, bus, slot, func, offset, val);
        return;
    }

    config_write32(bus, slot, func, static_cast<u8>(offset), val);
}

u32 PCIe::config_read32(u8 bus, u8 slot, u8 func, u8 offset) {
    u32 address = static_cast<u32>((static_cast<u32>(bus) << 16) |
                                  (static_cast<u32>(slot) << 11) |
                                  (static_cast<u32>(func) << 8) |
                                  (offset & 0xFC) |
                                  0x80000000);
    outl(0xCF8, address);
    return inl(0xCFC);
}

void PCIe::config_write32(u8 bus, u8 slot, u8 func, u8 offset, u32 val) {
    u32 address = static_cast<u32>((static_cast<u32>(bus) << 16) |
                                  (static_cast<u32>(slot) << 11) |
                                  (static_cast<u32>(func) << 8) |
                                  (offset & 0xFC) |
                                  0x80000000);
    outl(0xCF8, address);
    outl(0xCFC, val);
}

void PCIe::enable_bus_master(u8 bus, u8 slot, u8 func) {
    u32 cmd_status = read32(bus, slot, func, 0x04);
    cmd_status |= (1 << 2);
    write32(bus, slot, func, 0x04, cmd_status);
}

void PCIe::initialize(const ACPI::MCFGHeader* mcfg) {
    if (active) {
        return;
    }

    if (mcfg) {
        ACPI::ECAMMap map{};
        if (ACPI::parse_mcfg(mcfg, map)) {
            PCI::ECAM::init(map);
        }
    }

    scan_and_leach_resources();

    if (!active) {
        destroy();
    }
}

void PCIe::destroy() {
    release_resources();
    active = false;
}

bool PCIe::is_active() const {
    return active;
}

void PCIe::scan_and_leach_resources() {
    for (u16 bus = 0; bus < 256; ++bus) {
        for (u8 slot = 0; slot < 32; ++slot) {
            for (u8 func = 0; func < 8; ++func) {
                u32 vendor_device = read32(static_cast<u8>(bus), slot, func, 0x00);
                u16 vendor_id = static_cast<u16>(vendor_device & 0xFFFF);

                if (vendor_id == 0xFFFF) {
                    if (func == 0) {
                        break;
                    }
                    continue;
                }

                u32 class_rev = read32(static_cast<u8>(bus), slot, func, 0x08);
                u8 class_code = static_cast<u8>(class_rev >> 24);
                u8 subclass = static_cast<u8>(class_rev >> 16);

                if (class_code == 0x01 && subclass == 0x08) {
                    active = true;
                    enable_bus_master(static_cast<u8>(bus), slot, func);
                }

                u32 header_type_reg = read32(static_cast<u8>(bus), slot, func, 0x0C);
                u8 header_type = static_cast<u8>(header_type_reg >> 16);
                if (func == 0 && (header_type & 0x80) == 0) {
                    break;
                }
            }
        }
    }
}

void PCIe::release_resources() {
}

uptr PCIe::find_nvme_bar0() {
    if (!active) {
        return 0;
    }

    for (u16 bus = 0; bus < 256; ++bus) {
        for (u8 slot = 0; slot < 32; ++slot) {
            for (u8 func = 0; func < 8; ++func) {
                u32 vendor_device = read32(static_cast<u8>(bus), slot, func, 0x00);
                u16 vendor_id = static_cast<u16>(vendor_device & 0xFFFF);

                if (vendor_id == 0xFFFF) {
                    if (func == 0) {
                        break;
                    }
                    continue;
                }

                u32 class_rev = read32(static_cast<u8>(bus), slot, func, 0x08);
                u8 class_code = static_cast<u8>(class_rev >> 24);
                u8 subclass = static_cast<u8>(class_rev >> 16);

                if (class_code == 0x01 && subclass == 0x08) {
                    u32 bar0_lo = read32(static_cast<u8>(bus), slot, func, 0x10);
                    u32 bar0_hi = 0;

                    if ((bar0_lo & 0x06) == 0x04) {
                        bar0_hi = read32(static_cast<u8>(bus), slot, func, 0x14);
                    }

                    uptr bar0 = (static_cast<uptr>(bar0_hi) << 32) | (bar0_lo & ~0x0FU);
                    return bar0;
                }

                u32 header_type_reg = read32(static_cast<u8>(bus), slot, func, 0x0C);
                u8 header_type = static_cast<u8>(header_type_reg >> 16);
                if (func == 0 && (header_type & 0x80) == 0) {
                    break;
                }
            }
        }
    }

    return 0;
}

}