#pragma once
#include <logger.hpp>
#include <fennlib/types>
#include <fennlib/sequence>
#include <fennlib/sys>
#include <fennlib/string>
#include <skeleton.hpp>
#include <AlopexOS/chrono.hpp>

namespace io {
    using namespace fennlib::types;

    namespace DeviceTypes {
        class GEN;
        class NVM;
        class RAM;
        class CPU;
        class GPU;
        class NPU;
        class MCU;
        class ARM;
        class PCI;
        class PCIE;
        class GPIO;
        class SERIAL;
        class MOTHERBOARD;

        class MOTHERBOARD {
            fennlib::string m_manufacturer{""};
            fennlib::string m_name{""};
            fennlib::string m_version{""};
            fennlib::string m_serial{""};
            fennlib::string m_asset_tag{""};
            fennlib::u64 m_rsdp_address{0};

        public:
            constexpr MOTHERBOARD() = default;

            auto configure(const char* mfr, const char* board_name, const char* ver, const char* ser, const char* asset, fennlib::u64 rsdp) -> void {
                m_rsdp_address = rsdp;
                m_manufacturer = mfr;
                m_name = board_name;
                m_version = ver;
                m_serial = ser;
                m_asset_tag = asset;
            }

            [[nodiscard]] inline auto get_manufacturer() const noexcept -> const fennlib::string& {
                return m_manufacturer;
            }

            [[nodiscard]] inline auto get_name() const noexcept -> const fennlib::string& {
                return m_name;
            }

            [[nodiscard]] inline auto get_version() const noexcept -> const fennlib::string& {
                return m_version;
            }

            [[nodiscard]] inline auto get_serial() const noexcept -> const fennlib::string& {
                return m_serial;
            }

            [[nodiscard]] inline auto get_asset_tag() const noexcept -> const fennlib::string& {
                return m_asset_tag;
            }

            [[nodiscard]] inline auto get_rsdp_address() const noexcept -> fennlib::u64 {
                return m_rsdp_address;
            }
        };

        class CPU {
            fennlib::string m_manufacturer{""};
            fennlib::string m_version{""};
            fennlib::u32 m_core_count{0};
            fennlib::u32 m_thread_count{0};
            fennlib::u32 m_max_speed_mhz{0};
            bool m_populated{false};

        public:
            constexpr CPU() = default;

            auto configure(const char* mfr, const char* ver, fennlib::u32 cores, fennlib::u32 threads, fennlib::u32 speed, bool populated) -> void {
                m_manufacturer = mfr;
                m_version = ver;
                m_core_count = cores;
                m_thread_count = threads;
                m_max_speed_mhz = speed;
                m_populated = populated;
            }

            [[nodiscard]] inline auto get_manufacturer() const noexcept -> const fennlib::string& {
                return m_manufacturer;
            }

            [[nodiscard]] inline auto get_version() const noexcept -> const fennlib::string& {
                return m_version;
            }

            [[nodiscard]] inline auto get_core_count() const noexcept -> fennlib::u32 {
                return m_core_count;
            }

            [[nodiscard]] inline auto get_thread_count() const noexcept -> fennlib::u32 {
                return m_thread_count;
            }

            [[nodiscard]] inline auto get_max_speed_mhz() const noexcept -> fennlib::u32 {
                return m_max_speed_mhz;
            }

            [[nodiscard]] inline auto is_populated() const noexcept -> bool {
                return m_populated;
            }
        };

        class RAM {
            fennlib::string m_device_locator{""};
            fennlib::string m_bank_locator{""};
            fennlib::string m_manufacturer{""};
            fennlib::string m_serial{""};
            fennlib::string m_part_number{""};
            fennlib::u64 m_base_address{0};
            fennlib::u64 m_size_mb{0};
            fennlib::u32 m_speed_mhz{0};
            bool m_populated{false};

        public:
            constexpr RAM() = default;

            auto configure(const char* dev_loc, const char* bank_loc, const char* mfr, const char* ser, const char* part, fennlib::u64 size_mb, fennlib::u32 speed, bool populated, fennlib::u64 base_address) -> void {
                m_device_locator = dev_loc;
                m_bank_locator = bank_loc;
                m_manufacturer = mfr;
                m_serial = ser;
                m_part_number = part;
                m_size_mb = size_mb;
                m_speed_mhz = speed;
                m_populated = populated;
                m_base_address = base_address;
            }

            // The return value works interchangibly as deviceID
            [[nodiscard]] inline auto get_device_locator() const noexcept -> const fennlib::string& {
                return m_device_locator;
            }

            [[nodiscard]] inline auto get_bank_locator() const noexcept -> const fennlib::string& {
                return m_bank_locator;
            }

            [[nodiscard]] inline auto get_manufacturer() const noexcept -> const fennlib::string& {
                return m_manufacturer;
            }

            [[nodiscard]] inline auto get_serial() const noexcept -> const fennlib::string& {
                return m_serial;
            }

            [[nodiscard]] inline auto get_part_number() const noexcept -> const fennlib::string& {
                return m_part_number;
            }

            [[nodiscard]] inline auto get_base_address() const noexcept -> fennlib::u64 {
                return m_base_address;
            }

            [[nodiscard]] inline auto get_size_mb() const noexcept -> fennlib::u64 {
                return m_size_mb;
            }

            [[nodiscard]] inline auto get_speed_mhz() const noexcept -> fennlib::u32 {
                return m_speed_mhz;
            }

            [[nodiscard]] inline auto is_populated() const noexcept -> bool {
                return m_populated;
            }
        };

        class NVM {
            fennlib::string m_model{""};
            fennlib::string m_serial{""};
            fennlib::string m_firmware{""};
            fennlib::u64 m_capacity_mb{0};
            bool m_is_nvme{false};

        public:
            constexpr NVM() = default;

            auto configure(const char* model, const char* serial, const char* firmware, fennlib::u64 capacity_mb, bool is_nvme) -> void {
                m_model = model;
                m_serial = serial;
                m_firmware = firmware;
                m_capacity_mb = capacity_mb;
                m_is_nvme = is_nvme;
            }

            [[nodiscard]] inline auto get_model() const noexcept -> const fennlib::string& {
                return m_model;
            }

            [[nodiscard]] inline auto get_serial() const noexcept -> const fennlib::string& {
                return m_serial;
            }

            [[nodiscard]] inline auto get_firmware() const noexcept -> const fennlib::string& {
                return m_firmware;
            }

            [[nodiscard]] inline auto get_capacity_mb() const noexcept -> fennlib::u64 {
                return m_capacity_mb;
            }

            [[nodiscard]] inline auto is_nvme() const noexcept -> bool {
                return m_is_nvme;
            }
        };
    }

    enum class DeviceClass {
        GEN, NVM, RAM, CPU, GPU,
        NPU, MCU, ARM, PCI, PCIE,
        GPIO, SERIAL, MOTHERBOARD
    };

    struct DeviceDescriptor {
        DeviceClass type{DeviceClass::GEN};
        u64 uid{0};
        u64 mmio_base{0};
        u64 mmio_size{0};
        u32 parent_bus{0};
        bool active{false};
        AlopexOS::chrono::RTT rtt{0};
    };
    
    class Synapse {
        fennlib::sequence<DeviceDescriptor> registry{};
        DeviceTypes::MOTHERBOARD m_root_motherboard{};
        fennlib::sequence<DeviceTypes::CPU> m_cpus{};
        fennlib::sequence<DeviceTypes::RAM> m_ram_modules{};
        fennlib::sequence<DeviceTypes::NVM> m_nvm_devices{};
        bool m_initialized{false};
    public:
        auto registerDevice(const DeviceDescriptor& desc) -> void {
            registry.push_back(desc);
        }

        auto inline init(const Skeleton& skel, Logger& logger) -> void;

        template<bool debug = false>
        auto scan_hardware(const Skeleton& skel, Logger* logger) -> void;

        [[nodiscard]] auto get_root_motherboard() const noexcept -> const DeviceTypes::MOTHERBOARD& {
            return m_root_motherboard;
        }

        [[nodiscard]] auto get_cpus() const noexcept -> const fennlib::sequence<DeviceTypes::CPU>& {
            return m_cpus;
        }

        [[nodiscard]] auto get_ram_modules() const noexcept -> const fennlib::sequence<DeviceTypes::RAM>& {
            return m_ram_modules;
        }

        [[nodiscard]] auto get_nvm_devices() const noexcept -> const fennlib::sequence<DeviceTypes::NVM>& {
            return m_nvm_devices;
        }

        inline auto getDeviceCount() const -> uint {
            return registry.size();
        }

        [[nodiscard]] inline auto ping_device(u64 uid) const -> AlopexOS::chrono::CCT;
        inline auto audit_latencies(io::Logger& logger) -> void;

        [[nodiscard]] inline auto get_registry() const noexcept -> const fennlib::sequence<DeviceDescriptor>& {
            return registry;
        }
    };
}

template<bool debug>
auto inline io::Synapse::scan_hardware(const Skeleton& skel, Logger* logger) -> void {
    u64 hhdm = skel.get_hhdm_offset();
    
    const char* mfr = "";
    const char* prod = "";
    const char* ver = "";
    const char* ser = "";
    const char* asset = "";
    u64 table_phys = 0;

    u8* bios_scan_base = reinterpret_cast<u8*>(0x000F0000 + hhdm);
    u8* bios_scan_end = reinterpret_cast<u8*>(0x000FFFF0 + hhdm);

    for (u8* ptr = bios_scan_base; ptr < bios_scan_end; ptr += 16)
        if (ptr[0] == '_' && ptr[1] == 'S' && ptr[2] == 'M' && ptr[3] == '_') {
            u32 phys_table_addr = *reinterpret_cast<u32*>(ptr + 0x18);
            if (phys_table_addr != 0) {
                table_phys = static_cast<u64>(phys_table_addr);
                break;
            }
        }
    

    if (table_phys == 0) {
        if constexpr (debug)
            logger->log(LogLevel::Warning, "Synapse: SMBIOS entry point not found in legacy BIOS range.");
    } else {
        if constexpr (debug)
            logger->log(LogLevel::Info, "Synapse: SMBIOS entry point located.");
        u8* iter = reinterpret_cast<u8*>(table_phys + hhdm);
        for (int safety = 0; safety < 100; ++safety) {
            u8 type = iter[0];
            u8 length = iter[1];

            if (type == 127) break;

            auto get_string = [](u8* struct_ptr, u8 len, u8 str_num) -> const char* {
                if (str_num == 0) return "";
                const char* string_start = reinterpret_cast<const char*>(struct_ptr + len);
                for (int s = 1; s < str_num; ++s) {
                    while (*string_start != '\0') string_start++;
                    string_start++;
                    if (*string_start == '\0') return "";
                }
                if (*string_start == '\0') return "";
                return string_start;
            };

            if (type == 1) {
                if (mfr[0] == '\0') mfr = get_string(iter, length, iter[4]);
                if (prod[0] == '\0') prod = get_string(iter, length, iter[5]);
                if (ver[0] == '\0') ver = get_string(iter, length, iter[6]);
                if (ser[0] == '\0') ser = get_string(iter, length, iter[7]);
            } else if (type == 2) {
                mfr = get_string(iter, length, iter[4]);
                prod = get_string(iter, length, iter[5]);
                ver = get_string(iter, length, iter[6]);
                ser = get_string(iter, length, iter[7]);
                asset = get_string(iter, length, iter[8]);
            } else if (type == 4) {
                const char* cpu_mfr = get_string(iter, length, iter[7]);
                const char* cpu_ver = get_string(iter, length, iter[16]);
                u32 cpu_speed = *reinterpret_cast<u16*>(iter + 0x14);
                
                u8 status = iter[0x18];
                bool cpu_populated = (status & 0x40) != 0;

                u32 cpu_cores = 1;
                u32 cpu_threads = 1;
                if (length > 0x2A) {
                    cpu_cores = *reinterpret_cast<u16*>(iter + 0x2A);
                    cpu_threads = *reinterpret_cast<u16*>(iter + 0x2C);
                }

                DeviceTypes::CPU cpu_device;
                cpu_device.configure(cpu_mfr, cpu_ver, cpu_cores, cpu_threads, cpu_speed, cpu_populated);
                m_cpus.push_back(cpu_device);

                registerDevice(DeviceDescriptor{
                    .type = DeviceClass::CPU,
                    .uid = 0x2 + m_cpus.size(),
                    .active = cpu_populated
                });
            } else if (type == 17) {
                u16 size_raw = (length > 0x11) ? *reinterpret_cast<u16*>(iter + 0x0C) : 0;
                u64 size_mb = 0;
                if (size_raw != 0 && size_raw != 0xFFFF) {
                    if ((size_raw & 0x8000) != 0) size_mb = static_cast<u64>(size_raw & 0x7FFF) / 1024;
                    else size_mb = static_cast<u64>(size_raw);
                }

                const char* dev_loc = (length > 0x10) ? get_string(iter, length, iter[0x10]) : "";
                const char* bank_loc = (length > 0x11) ? get_string(iter, length, iter[0x11]) : "";
                const char* ram_mfr = (length > 0x17) ? get_string(iter, length, iter[0x17]) : "";
                const char* ram_ser = (length > 0x18) ? get_string(iter, length, iter[0x18]) : "";
                const char* ram_part = (length > 0x1A) ? get_string(iter, length, iter[0x1A]) : "";
                u32 ram_speed = (length > 0x15) ? *reinterpret_cast<u16*>(iter + 0x15) : 0;

                bool populated = (size_mb > 0);

                u64 base_address = 0;
                if (m_ram_modules.size() > 0) {
                    auto& prev = m_ram_modules[m_ram_modules.size() - 1];
                    base_address = prev.get_base_address() + (prev.get_size_mb() * 1024 * 1024);
                }

                DeviceTypes::RAM ram_device;
                ram_device.configure(dev_loc, bank_loc, ram_mfr, ram_ser, ram_part, size_mb, ram_speed, populated, base_address);
                m_ram_modules.push_back(ram_device);

                registerDevice(DeviceDescriptor{
                    .type = DeviceClass::RAM,
                    .uid = 0x100 + m_ram_modules.size(),
                    .mmio_base = base_address,
                    .mmio_size = size_mb * 1024 * 1024,
                    .active = populated
                });
            }
            
            u8* next_iter = iter + length;
            while (!(next_iter[0] == 0 && next_iter[1] == 0)) next_iter++;
            next_iter += 2;
            iter = next_iter;
        }
    }

    DeviceTypes::NVM sample_nvm;
    sample_nvm.configure("", "", "", 0, false);
    m_nvm_devices.push_back(sample_nvm);

    registerDevice(DeviceDescriptor{
        .type = DeviceClass::NVM,
        .uid = 0x200,
        .active = true
    });

    auto* rsdp = skel.get_rsdp();
    u64 rsdp_addr = (rsdp != nullptr) ? reinterpret_cast<fennlib::u64>(rsdp) : 0;

    m_root_motherboard.configure(mfr, prod, ver, ser, asset, rsdp_addr);

    registerDevice(DeviceDescriptor{
        .type = DeviceClass::MOTHERBOARD,
        .uid = 0x1,
        .mmio_base = table_phys,
        .active = true
    });
}

auto io::Synapse::init(const Skeleton& skel, Logger& logger) -> void {
    if (!skel.is_initialized()) {
        logger.log(LogLevel::Critical, "Synapse: Skeleton is not initialized, halting system.");
        skel.halt();
    }

    logger.log(LogLevel::Info, "Synapse: Beginning initialization sequence.");
    
    scan_hardware<true>(skel, &logger);
    audit_latencies(logger);

    m_initialized = true;
    logger.log(LogLevel::Info, "Synapse: Initialization complete.");
}

[[nodiscard]] auto io::Synapse::ping_device(fennlib::u64 uid) const -> AlopexOS::chrono::CCT {
    fennlib::u64 const start = AlopexOS::chrono::read_tsc();

    bool found = false;
    for (fennlib::u64 i = 0; i < registry.size(); ++i)
        if (registry[i].uid == uid) {
            found = true;
            if (registry[i].active && registry[i].mmio_base)
                [[maybe_unused]] volatile auto dummy = *reinterpret_cast<const volatile u64*>(registry[i].mmio_base);
            break;
        }

    fennlib::u64 const end = AlopexOS::chrono::read_tsc();
    return found ? (end - start) : 0;
}

auto io::Synapse::audit_latencies(io::Logger& logger) -> void {
    logger.log(LogLevel::Info, "Synapse: Beginning device pingback and latency audit.");

    for (fennlib::u64 i = 0; i < registry.size(); ++i) {
        auto& desc = registry[i];
        AlopexOS::chrono::CCT const latency = ping_device(desc.uid);

        desc.rtt = latency;

        if (desc.active)
            logger.log(LogLevel::Info, "Synapse: Device UID [active] responded in CCT cycles.");
    }

    logger.log(LogLevel::Info, "Synapse: Latency audit complete.");
}