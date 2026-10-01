#pragma once

#include <fennlib/types>

namespace AlopexOS {
    enum class error_code : fennlib::types::u32 {
        Success = 0,
        Error = static_cast<fennlib::u32>(-1),

        synapse_generic = 0x1000,
        synapse_not_initialized,
        device_not_found,
        device_not_ready,
        device_unresponsive,
        invalid_mmio,
        smbios_not_found,

        drama_generic = 0x2000,
        out_of_memory,
        invalid_alignment,
        double_free,
        corruption_detected,
        out_of_bounds,

        systemx_generic = 0x3000,
        invalid_program_range,

        avfs_generic = 0x4000,
        avfs_read_failed,
        avfs_write_failed,
        avfs_invalid_format,
        invalid_filesystem,

        dycora_generic = 0x5000
    };
}