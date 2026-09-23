#include "skeleton.hpp"
#include <logger.hpp>
#include <fennlib/types>
#include <fennlib/sequence>
#include <synapse.hpp>
#include <AlopexOS/errorCodes.hpp> 

extern fennlib::u8 heap_pool[16 * 1024 * 1024];

namespace AlopexOS::Kernel {
    using namespace fennlib::types;
    using fennlib::sequence;

    class DRAMA {
        io::Synapse* p_synapse{nullptr};
        
        struct ramBlock {
            uintptr base_address{0};
            usize size_bytes{0};
            fennlib::string serial{""};
        };
        sequence<ramBlock> p_ram_blocks{};
    public:
        DRAMA() = default;
        ~DRAMA() = default;
        DRAMA(io::Synapse* synapse) : p_synapse(synapse) {};

        template<bool debug = false>
        inline auto init(io::Skeleton skeleton, io::Logger* logger = nullptr) -> AlopexOS::error_code;

        template<bool debug = false>
        inline auto ram_setup(io::Logger* logger = nullptr) -> AlopexOS::error_code;
    };
}

template<bool debug>
inline auto AlopexOS::Kernel::DRAMA::init(io::Skeleton skeleton,  io::Logger* logger) -> AlopexOS::error_code {
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
        for (const auto& block : p_ram_blocks) {
            if (block.serial == ram_module.get_serial()) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            usize computed_base = 0;
            if (p_ram_blocks.empty()) {
                computed_base = sizeof(heap_pool);
            }

            p_ram_blocks.push_back(ramBlock{
                .base_address = computed_base,
                .size_bytes = static_cast<usize>(ram_module.get_size_mb() * 1000000),
                .serial = ram_module.get_serial()
            });
        }
    }

    return error_code::Success;
}