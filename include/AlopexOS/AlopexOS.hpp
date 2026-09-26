#include "arr.hpp"
#include "expected.hpp"
#include "primitives.hpp"
#if !defined(ALOPEXOS_HPP)
#define ALOPEXOS_HPP

#if !defined(__AlopexOS__)
#define __AlopexOS__
#endif

#include "AlopexOS_ErrorCodes.hpp" // IWYU pragma: keep

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

    template<typename Type>
    using Expected = AurenFox::core::Expected<Type, AlopexOS::errorCode>;

    // Common Handle / Identifier Type
    using Handle = u64;
    constexpr Handle InvalidHandle = static_cast<Handle>(-1);

    using Path = arr<byte, 2048>;

    extern "C" [[nodiscard]] fn AlopexOS_SysCall(
        usize syscall_number, 
        usize arg1, 
        usize arg2, 
        usize arg3
    ) noexcept -> usize;
}

#endif