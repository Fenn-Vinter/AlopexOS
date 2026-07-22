#include <AlopexOS/AlopexIBus/AlopexIBus.hpp>
#include <AlopexOS/PCI/pcie.hpp>
#include <AlopexOS/ACPI/mcfg.hpp>
#include <AlopexOS/limine_requests.hpp>

inline void serial_out(u16 port, u8 val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void serial_print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        serial_out(0x3F8, str[i]);
    }
}

const AlopexOS::ACPI::MCFGHeader* locate_acpi_table_mcfg(const AlopexOS::ACPI::SDTHeader* rsdt) {
    if (!rsdt) {
        return nullptr;
    }

    u32 entries = (rsdt->length - sizeof(AlopexOS::ACPI::SDTHeader)) / sizeof(u32);
    const u32* table_pointers = reinterpret_cast<const u32*>(
        reinterpret_cast<uptr>(rsdt) + sizeof(AlopexOS::ACPI::SDTHeader)
    );

    for (u32 i = 0; i < entries; ++i) {
        auto* header = reinterpret_cast<const AlopexOS::ACPI::SDTHeader*>(
            static_cast<uptr>(table_pointers[i])
        );

        if (header && AlopexOS::MCFG::is_mcfg_signature(header->signature)) {
            return reinterpret_cast<const AlopexOS::ACPI::MCFGHeader*>(header);
        }
    }

    return nullptr;
}


inline void outl(u16 port, u32 val) {
    __asm__ volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

inline auto inl(u16 port) -> u32 {
    u32 ret;
    __asm__ volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

auto AlopexOS::AlopexIBus::getPCIe() -> AlopexOS::PCIe* { return this->_pcie; }

auto AlopexOS::AlopexIBus::get_hhdm_offset() const -> u64 {
    return this->hhdm_offset;
}

auto AlopexOS::AlopexIBus::get_storage_devices() const -> const dynarr<BusDeviceInfo>& {
    const_cast<AlopexOS::AlopexIBus*>(this)->_storage_devices.clear();
    for (size_t i = 0; i < _discovered_devices.size(); ++i) {
        auto type = _discovered_devices[i].deduced_type;
        if (type == PhysicalDeducedType::NVME || 
            type == PhysicalDeducedType::AHCI_SATA || 
            type == PhysicalDeducedType::RAW_BLOCK_STORAGE) {
            const_cast<AlopexOS::AlopexIBus*>(this)->_storage_devices.push_back(_discovered_devices[i]);
        }
    }
    return _storage_devices;
}

AlopexOS::AlopexIBus::AlopexIBus() {
    if (hhdm_request.response != nullptr) {
        hhdm_offset = hhdm_request.response->offset;
        serial_print("[IBus] Limine HHDM offset retrieved successfully.\n");
    } else {
        serial_print("[IBus] WARNING: Limine HHDM response is null!\n");
    }

    if (rsdp_request.response != nullptr) {
        serial_print("[IBus] Limine RSDP response found.\n");
        auto* rsdp = reinterpret_cast<const AlopexOS::ACPI::RSDPDescriptor*>(rsdp_request.response->address);
        if (rsdp) {
            serial_print("[IBus] Extracting RSDT base address...\n");
            auto* rsdt = reinterpret_cast<const AlopexOS::ACPI::SDTHeader*>(static_cast<uptr>(rsdp->rsdt_address));
            _mcfg = locate_acpi_table_mcfg(rsdt);
            if (_mcfg) {
                serial_print("[IBus] MCFG table successfully located in RSDT!\n");
            } else {
                serial_print("[IBus] MCFG table not found in RSDT. Defaulting to Port I/O fallback.\n");
            }
        }
    } else {
        serial_print("[IBus] Limine RSDP response is null! Defaulting to Port I/O fallback.\n");
    }

    _pcie = &AlopexOS::PCIe::get_instance();
    _pcie->initialize(_mcfg);
}

auto AlopexOS::AlopexIBus::get_instance() -> AlopexIBus& {
    static AlopexIBus instance{};
    return instance;
}

auto AlopexOS::AlopexIBus::deduce_pci_type(u8 class_code, u8 subclass_code, u8 prog_if) -> PhysicalDeducedType {
    if (class_code == 0x01) {
        if (subclass_code == 0x08 && prog_if == 0x02) {
            return PhysicalDeducedType::NVME;
        }
        if (subclass_code == 0x06 && prog_if == 0x01) {
            return PhysicalDeducedType::AHCI_SATA;
        }
        return PhysicalDeducedType::RAW_BLOCK_STORAGE;
    }
    
    if (class_code == 0x0C && subclass_code == 0x03) {
        return PhysicalDeducedType::USB_HOST;
    }

    return PhysicalDeducedType::Unknown;
}

auto AlopexOS::AlopexIBus::scan_pci_bus() -> errorCode {
    for (u16 bus = 0; bus < 256; ++bus) {
        for (u8 dev = 0; dev < 32; ++dev) {
            for (u8 func = 0; func < 8; ++func) {
                u32 pci_addr = (1 << 31) | (bus << 16) | (dev << 11) | (func << 8);
                
                outl(0xCF8, pci_addr);
                u32 vendor_device = inl(0xCFC);
                u16 vendor_id = static_cast<u16>(vendor_device & 0xFFFF);
                
                if (vendor_id == 0xFFFF) {
                    continue;
                }

                u16 device_id = static_cast<u16>((vendor_device >> 16) & 0xFFFF);

                outl(0xCF8, pci_addr | 0x08);
                u32 class_rev = inl(0xCFC);
                u8 prog_if = static_cast<u8>((class_rev >> 8) & 0xFF);
                u8 subclass_code = static_cast<u8>((class_rev >> 16) & 0xFF);
                u8 class_code = static_cast<u8>((class_rev >> 24) & 0xFF);

                outl(0xCF8, pci_addr | 0x10);
                u32 bar0 = inl(0xCFC);

                BusDeviceInfo info{};
                info.bus_type = BusType::PCIe;
                info.vendor_id = vendor_id;
                info.device_id = device_id;
                info.base_address = static_cast<PhysicalAddress>(bar0 & ~0x0F);
                info.location.pci.bus = static_cast<u8>(bus);
                info.location.pci.device = dev;
                info.location.pci.function = func;
                info.deduced_type = deduce_pci_type(class_code, subclass_code, prog_if);

                _discovered_devices.push_back(info);
            }
        }
    }

    return errorCode::Success;
}

auto AlopexOS::AlopexIBus::scan_gpio_bus() -> errorCode {
    return errorCode::Success;
}

auto AlopexOS::AlopexIBus::scan_all_buses() -> errorCode {
    _discovered_devices.clear();
    
    scan_pci_bus();
    scan_gpio_bus();

    return errorCode::Success;
}

auto AlopexOS::AlopexIBus::get_devices_by_bus(BusType bus) const -> const dynarr<BusDeviceInfo>& {
    const_cast<AlopexOS::AlopexIBus*>(this)->_filtered_results.clear();

    for (size_t i = 0; i < _discovered_devices.size(); ++i) {
        if (_discovered_devices[i].bus_type == bus) {
            const_cast<AlopexOS::AlopexIBus*>(this)->_filtered_results.push_back(_discovered_devices[i]);
        }
    }

    return _filtered_results;
}

auto AlopexOS::AlopexIBus::get_devices_by_deduced_type(PhysicalDeducedType type) const -> const dynarr<BusDeviceInfo>& {
    const_cast<AlopexOS::AlopexIBus*>(this)->_filtered_results.clear();

    for (size_t i = 0; i < _discovered_devices.size(); ++i) {
        if (_discovered_devices[i].deduced_type == type) {
            const_cast<AlopexOS::AlopexIBus*>(this)->_filtered_results.push_back(_discovered_devices[i]);
        }
    }

    return _filtered_results;
}