#pragma once
#include <logger.hpp>
#include <fennlib/types>
#include <fennlib/sequence>
#include <fennlib/string>
#include <synapse.hpp>
#include <AlopexOS/errorCodes.hpp>
#include <AlopexOS/types.hpp>

extern fennlib::u8 heap_pool[16 * 1024 * 1024];

namespace AlopexOS::Kernel {
    class DRAMA {
        struct PhysicalRAM {
            DeviceID device_id;
            fennlib::types::uintptr base_address;
            fennlib::types::usize size_bytes;
        };
        struct VirtualRAM {
            DeviceID device_id;
            PID pid;
            fennlib::types::uintptr base_address;
            fennlib::types::usize size_bytes;
        };

        io::Synapse*  p_synapse{nullptr};

        fennlib::sequence<PhysicalRAM> p_mountedRAM;

        fennlib::sequence<VirtualRAM> p_mountedVRAM;
    public:
        DRAMA() = default;
        ~DRAMA() = default;
        DRAMA(io::Synapse* synapse) : p_synapse(synapse) {};

        template<bool Debug = false>
        inline auto init(io::Logger* logger) -> AlopexOS::error_code;

        inline auto init() -> AlopexOS::error_code { return init<false>(nullptr); };

        /**
        * @brief Mounts a physical RAM device from the Synapse.
        * 
        * @tparam Debug Enables debug logging if true. Logger must != nullptr
        * @param ram_device_id The device ID of the RAM module to mount.
        * @param logger Optional logger instance for debug output.
        * @return AlopexOS::error_code Success if discovered, or device_not_found if it wasn't found via synapse.
        */
        template<bool Debug = false>
        inline auto mount(DeviceID ram_device_id, io::Logger* logger = nullptr) -> AlopexOS::error_code;

        /**
        * @brief Mounts a physical RAM device from the Synapse.
        * 
        * @param ram_device_id The device ID of the RAM module to mount.
        * @return AlopexOS::error_code Success if discovered, or device_not_found if it wasn't found via synapse.
        */
        inline auto mount(DeviceID ram_device_id) -> AlopexOS::error_code { return mount<false>(ram_device_id); }

        /**
        * @brief Unmounts a physical RAM device from the Synapse.
        * 
        * @tparam Debug Enables debug logging if true.
        * @param ram_device_id The device ID of the RAM module to remove.
        * @param logger Optional logger instance for debug output.
        * @return AlopexOS::error_code Success if removed, or device_not_found if it wasn't mounted.
        */
        template<bool Debug = false>
        inline auto unmount(DeviceID ram_device_id, io::Logger* logger = nullptr) -> AlopexOS::error_code;

        /**
        * @brief Unmounts a physical RAM device from the Synapse.
        * 
        * @param ram_device_id The device ID of the RAM module to remove.
        * @return AlopexOS::error_code Success if removed, or device_not_found if it wasn't mounted.
        */
        inline auto unmount(DeviceID ram_device_id) -> AlopexOS::error_code { return unmount<false>(ram_device_id); }

        template<bool Debug = false>
        inline auto mount_all(io::Logger* logger) -> AlopexOS::error_code;

        inline auto mount_all() -> AlopexOS::error_code { return mount_all<false>(nullptr); };

        template<bool Debug = false>
        inline auto malloc(fennlib::types::usize size_bytes, PID pid, DeviceID ram_device_id = "", io::Logger* logger = nullptr) -> fennlib::types::uintptr;

        inline auto malloc(fennlib::types::usize size_bytes, PID pid, DeviceID ram_device_id = "") -> fennlib::types::uintptr { return malloc<false>(size_bytes, pid, ram_device_id); }

        inline auto lastIndex() -> const VirtualRAM& { return *p_mountedVRAM.end(); }
    };
}

template<bool Debug>
inline auto AlopexOS::Kernel::DRAMA::init(io::Logger* logger) -> AlopexOS::error_code {
    error_code err = mount_all<Debug>(logger);
    if (err != error_code::Success) {
        return err;
    }
    malloc(sizeof(heap_pool), 0);
    return error_code::Success;
}

template<bool Debug>
inline auto AlopexOS::Kernel::DRAMA::mount_all(io::Logger* logger) -> AlopexOS::error_code {
    p_synapse->scan_hardware<Debug>(logger);

    const auto& modules = p_synapse->get_ram_modules();
    if (modules.size() == 0) {
        if constexpr (Debug) {
            if (logger) {
                logger->log(io::LogLevel::Info, "[DRAMA::mount_all]: No RAM modules discovered.");
            }
        }
        return AlopexOS::error_code::device_not_found;
    }

    for (const auto& module : modules) {
        DeviceID dev_id = module.get_device_locator();
        
        bool already_mounted = false;
        for (const auto& mounted : p_mountedRAM) {
            if (mounted.device_id == dev_id) {
                already_mounted = true;
                break;
            }
        }

        if (already_mounted) continue;

        fennlib::types::usize size_in_bytes = static_cast<fennlib::types::usize>(module.get_size_mb()) * 1024 * 1024;

        p_mountedRAM.push_back(PhysicalRAM{
            .device_id = dev_id,
            .base_address = module.get_base_address(),
            .size_bytes = size_in_bytes
        });

        if constexpr (Debug) {
            if (logger) {
                logger->log(io::LogLevel::Info, "[DRAMA::mount_all]: Automatically mounted RAM module.");
            }
        }
    }

    return AlopexOS::error_code::Success;
}

template<bool Debug>
inline auto AlopexOS::Kernel::DRAMA::mount(DeviceID ram_device_id, io::Logger* logger) -> AlopexOS::error_code {
    auto find_module = [this, &ram_device_id]() -> const auto* {
        for (const auto& module : p_synapse->get_ram_modules()) {
            if (module.get_device_locator() == ram_device_id) return &module;
        }
        return static_cast<const decltype(&p_synapse->get_ram_modules()[0])>(nullptr);
    };

    const auto* module_ptr = find_module();

    if (!module_ptr) {
        p_synapse->scan_hardware<Debug>(logger);
        module_ptr = find_module();
    }

    if (!module_ptr) return AlopexOS::error_code::device_not_found;

    fennlib::types::usize size_in_bytes = static_cast<fennlib::types::usize>(module_ptr->get_size_mb()) * 1024 * 1024;

    p_mountedRAM.push_back(PhysicalRAM{
        .device_id = ram_device_id,
        .base_address = module_ptr->get_base_address(),
        .size_bytes = size_in_bytes
    });

    return AlopexOS::error_code::Success;
}

template<bool Debug>
inline auto AlopexOS::Kernel::DRAMA::unmount(DeviceID ram_device_id, io::Logger* logger) -> AlopexOS::error_code {
    for (fennlib::types::usize i = 0; i < p_mountedRAM.size(); ++i) {
        if (p_mountedRAM[i].device_id == ram_device_id) {
            p_mountedRAM.erase_ordered(i);
            if constexpr (Debug) logger->log(io::LogLevel::Info, (fennlib::string("[DRAMA::unmount]: Device successfully unmounted: ") + ram_device_id).c_str());
            return AlopexOS::error_code::Success;
        }
    }
    if constexpr (Debug) logger->log(io::LogLevel::Info, (fennlib::string("[DRAMA::unmount]: Device not successfully unmounted: ") + ram_device_id).c_str());
    return AlopexOS::error_code::device_not_found;
}

template<bool Debug>
inline auto AlopexOS::Kernel::DRAMA::malloc(fennlib::types::usize size_bytes, PID pid, AlopexOS::types::DeviceID ram_device_id, io::Logger* logger) -> fennlib::types::uintptr {
    PhysicalRAM* target_phys = nullptr;
    fennlib::types::uintptr allocation_address = 0;

    if (ram_device_id == "") {
        for (auto& phys_ram : p_mountedRAM) {
            fennlib::types::uintptr highest_end = phys_ram.base_address;
            
            for (const auto& vram : p_mountedVRAM) {
                if (vram.device_id == phys_ram.device_id) {
                    fennlib::types::uintptr end_addr = vram.base_address + vram.size_bytes;
                    if (end_addr > highest_end) highest_end = end_addr;
                }
            }

            if (highest_end + size_bytes <= phys_ram.base_address + phys_ram.size_bytes) {
                target_phys = &phys_ram;
                allocation_address = highest_end;
                break;
            }
        }
    } else {
        for (auto& phys_ram : p_mountedRAM) {
            if (phys_ram.device_id == ram_device_id) {
                target_phys = &phys_ram;
                break;
            }
        }

        if (target_phys) {
            fennlib::types::uintptr highest_end = target_phys->base_address;
            
            for (const auto& vram : p_mountedVRAM) {
                if (vram.device_id == target_phys->device_id) {
                    fennlib::types::uintptr end_addr = vram.base_address + vram.size_bytes;
                    if (end_addr > highest_end) highest_end = end_addr;
                }
            }

            if (highest_end + size_bytes <= target_phys->base_address + target_phys->size_bytes) allocation_address = highest_end;
            else target_phys = nullptr;
        }
    }

    if (!target_phys) {
        if constexpr (Debug) {
            if (logger) {
                logger->log(io::LogLevel::Info, "[DRAMA::malloc]: Allocation failed - insufficient physical RAM space.");
            }
        }
        return static_cast<fennlib::types::uintptr>(-1) - static_cast<fennlib::types::uintptr>(AlopexOS::error_code::out_of_memory);
    }

    p_mountedVRAM.push_back(VirtualRAM{
        .device_id = target_phys->device_id,
        .pid = pid,
        .base_address = allocation_address,
        .size_bytes = size_bytes
    });

    if constexpr (Debug) if (logger) logger->log(io::LogLevel::Info, "[DRAMA::malloc]: Virtual RAM successfully allocated to PID.");

    return allocation_address;
}