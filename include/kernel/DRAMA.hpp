#include "skeleton.hpp"
#include <logger.hpp>
#include <fennlib/types>
#include <fennlib/sequence>
#include <synapse.hpp>
#include <AlopexOS/errorCodes.hpp> 

extern fennlib::u8 heap_pool[16 * 1024 * 1024];

namespace AlopexOS::Kernel {
    using RAMCellID = fennlib::types::uintptr;
    using DeviceID = fennlib::types::usize;

    class DRAMA {
        io::Synapse* p_synapse{nullptr};
        RAMCellID m_next_cell_id{1};
        
        struct ramAllocation {
            fennlib::types::uintptr base_address{0};
            fennlib::types::usize size_bytes{0};
            fennlib::string serial{""};
            RAMCellID ram_cell_id{static_cast<RAMCellID>(-1)};
            DeviceID ram_device_id{static_cast<DeviceID>(-1)};
            bool locked{false};
        };
        
        fennlib::sequence<ramAllocation> p_ram_allocations{};

    public:
        DRAMA() = default;
        ~DRAMA() = default;
        DRAMA(io::Synapse* synapse) : p_synapse(synapse) {};

        template<bool debug = false>
        inline auto init(io::Skeleton skeleton, io::Logger* logger = nullptr) -> AlopexOS::error_code;

        template<bool debug = false>
        inline auto ram_setup(io::Logger* logger = nullptr) -> AlopexOS::error_code;

        inline auto malloc(fennlib::types::usize size_bytes, DeviceID ram_device_id = static_cast<DeviceID>(-1)) -> RAMCellID;
        inline auto migrate(RAMCellID ram_cell_id, DeviceID ram_device_id = static_cast<DeviceID>(-1)) -> RAMCellID;
    };

}

template<bool debug>
inline auto AlopexOS::Kernel::DRAMA::init(io::Skeleton skeleton, io::Logger* logger) -> AlopexOS::error_code {
    [[unlikely]] if (!p_synapse) return error_code::synapse_not_initialized;
    
    if constexpr (debug) {
        if (logger) {
            logger->log(io::LogLevel::Info, "[DRAMA::init()] Scanning for devices via Synapse");
        }
    }

    p_synapse->scan_hardware(skeleton, logger);

    ram_setup<debug>(logger);

    return error_code::Success;
}

template<bool debug>
inline auto AlopexOS::Kernel::DRAMA::ram_setup(io::Logger* logger) -> AlopexOS::error_code {
    for (const auto& ram_module : p_synapse->get_ram_modules()) {
        bool found = false;
        for (const auto& allocation : p_ram_allocations) {
            if (allocation.serial == ram_module.get_serial()) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            fennlib::types::uintptr computed_base = reinterpret_cast<fennlib::types::uintptr>(heap_pool) + sizeof(heap_pool);
            
            if (!p_ram_allocations.empty()) {
                const auto& last_alloc = p_ram_allocations[p_ram_allocations.size() - 1];
                computed_base = last_alloc.base_address + last_alloc.size_bytes;
            }

            p_ram_allocations.push_back(ramAllocation{
                .base_address = computed_base,
                .size_bytes = static_cast<fennlib::types::usize>(ram_module.get_size_mb() * 1000000),
                .serial = ram_module.get_serial(),
                .ram_cell_id = m_next_cell_id++,
                .locked = false
            });

            if constexpr (debug) {
                if (logger) {
                    logger->log(io::LogLevel::Info, "[DRAMA::ram_setup()] Registered RAM module cleanly past heap pool.");
                }
            }
        }
    }

    return error_code::Success;
}