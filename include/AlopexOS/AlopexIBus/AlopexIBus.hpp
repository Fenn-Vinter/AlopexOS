#if !defined(ALOPEX_IBUS_HPP)
#define ALOPEX_IBUS_HPP

#include <primitives.h>
#include <arr.hpp>
#include <AlopexOS/AlopexOS.hpp>
#include <AlopexOS/ACPI/acpi.hpp>
#include <AlopexOS/PCI/pcie.hpp>

namespace AlopexOS {

    enum class BusType : u8 {
        PCI,
        PCIe,
        NRZI,
        SERIAL,
        GPIO
    };

    enum class PhysicalDeducedType : u8 {
        Unknown = 0,
        NVME,
        AHCI_SATA,
        USB_HOST,
        GENERIC_SERIAL_PORT,
        RAW_BLOCK_STORAGE,
        GPIO_CONTROLLER,
        GPIO_BITBANG_DEVICE
    };

    struct BusDeviceInfo {
        BusType bus_type{BusType::PCIe};
        PhysicalDeducedType deduced_type{PhysicalDeducedType::Unknown};
        
        u16 vendor_id{0};
        u16 device_id{0};
        
        PhysicalAddress base_address{0};
        u32 io_port_base{0};
        u8 interrupt_line{0};
        char name[64]{0};

        union {
            struct {
                u8 bus;
                u8 device;
                u8 function;
            } pci;
            struct {
                u8 port_index;
                u8 endpoint;
            } nrzi;
            struct {
                u8 channel_id;
            } serial;
            struct {
                u8 bank;
                u8 pin_mask;
                u8 mode;
            } gpio;
        } location{};
    };

    class AlopexIBus {
    public:
        static auto get_instance() -> AlopexIBus&;

        AlopexIBus(const AlopexIBus&) = delete;
        auto operator=(const AlopexIBus&) -> AlopexOS::AlopexIBus& = delete;
        AlopexIBus(AlopexIBus&&) = delete;
        auto operator=(AlopexIBus&&) = delete;

        auto scan_all_buses() -> errorCode;
        auto get_devices_by_bus(BusType bus) const -> const dynarr<BusDeviceInfo>&;
        auto get_devices_by_deduced_type(PhysicalDeducedType type) const -> const dynarr<BusDeviceInfo>&;
        
        auto get_storage_devices() const -> const dynarr<BusDeviceInfo>&;

        auto getPCIe() -> AlopexOS::PCIe*;
        auto get_hhdm_offset() const -> u64;

    private:
        AlopexIBus();

        auto deduce_pci_type(u8 class_code, u8 subclass_code, u8 prog_if) -> PhysicalDeducedType;
        auto scan_pci_bus() -> errorCode;
        auto scan_gpio_bus() -> errorCode;

        dynarr<BusDeviceInfo> _discovered_devices{};
        dynarr<BusDeviceInfo> _filtered_results{};
        dynarr<BusDeviceInfo> _storage_devices{};

        u64 hhdm_offset = 0;
        const AlopexOS::ACPI::MCFGHeader* _mcfg = nullptr;
        AlopexOS::PCIe* _pcie = nullptr;
    };

}

#endif