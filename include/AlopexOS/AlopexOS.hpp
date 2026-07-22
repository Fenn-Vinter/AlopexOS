#if !defined(ALOPEXOS_HPP)
#define ALOPEXOS_HPP

#if !defined(__AlopexOS__)
#define __AlopexOS__
#endif

#include "AlopexOS_ErrorCodes.hpp"

namespace AlopexOS {
    enum class errorCode : u64;
    
    // Address Abstractions
    using PhysicalAddress = uptr;
    using VirtualAddress  = uptr;

    // Storage & Block I/O Types
    using LBA = u64;
    using BlockCount = u32;

    // Hardware Identification
    using DeviceID = u32;
    using VendorID = u16;

    // Common Handle / Identifier Type
    using Handle = u64;
    constexpr Handle InvalidHandle = static_cast<Handle>(-1);
}

#endif