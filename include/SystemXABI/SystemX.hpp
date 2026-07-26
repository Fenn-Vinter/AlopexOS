#include "AlopexOS/AlopexOS_ErrorCodes.hpp"
#include "AlopexOS/gaossd/gaossd.hpp"
#include "arr.hpp"
#include "primitives.hpp"
#include <AlopexOS/AVFS/avfs.hpp>

#if !defined (SYSTEM_X_HPP)
#define SYSTEM_X_HPP

class SystemX {
    AlopexOS::AVFS* avfs{nullptr};

public:
    enum class permission : u64 {
        NONE               = 0,
            
        // Core Access Control
        READ               = 1ULL << 0,
        WRITE              = 1ULL << 1,
        EXECUTE            = 1ULL << 2,
        ADMIN              = 1ULL << 3,
            
        // System Resource Controls
        NETWORK            = 1ULL << 4,
        STORAGE            = 1ULL << 5,
        DIAGNOSTIC         = 1ULL << 6,
        MEMORY_MAP         = 1ULL << 7,
            
        // Advanced Hardening & Isolation Flags
        RAW_IO             = 1ULL << 8,  
        INTERRUPT_CONTROL  = 1ULL << 9,  
        PROCESS_SPAWN      = 1ULL << 10, 
        DEBUGGER           = 1ULL << 11, 
        SYS_CONFIG         = 1ULL << 12, 

        // Combined security profiles and permission masks
        READ_WRITE         = READ | WRITE,
        DEFAULT            = READ | WRITE,
        EXECUTABLE         = READ | WRITE | EXECUTE,
        SANDBOXED          = READ | EXECUTE,                            
        SERVICE_DAEMON     = READ | WRITE | EXECUTE | NETWORK | STORAGE, 
        SUPERUSER          = 0xFFFFFFFFFFFFFFFFULL
    };

    static constexpr permission kernelSpace = permission::SUPERUSER;

    class programs {
    public:
        u64 pid{};
        AlopexOS::Path path{};
        permission permissions{};
    };

    dynarr<programs> _active_programs;

    fn executeProgram(const AlopexOS::Path& path) -> isize;
    fn attachAVFS(AlopexOS::AVFS* AVFS) -> AlopexOS::errorCode;
    fn getAVFSInstance() -> AlopexOS::AVFS*;
};

constexpr SystemX::permission operator|(SystemX::permission lhs, SystemX::permission rhs) {
    return static_cast<SystemX::permission>(static_cast<u64>(lhs) | static_cast<u64>(rhs));
}

constexpr SystemX::permission operator&(SystemX::permission lhs, SystemX::permission rhs) {
    return static_cast<SystemX::permission>(static_cast<u64>(lhs) & static_cast<u64>(rhs));
}

constexpr SystemX::permission operator~(SystemX::permission val) {
    return static_cast<SystemX::permission>(~static_cast<u64>(val));
}

constexpr bool has_permission(SystemX::permission target, SystemX::permission flag) {
    return (static_cast<u64>(target) & static_cast<u64>(flag)) == static_cast<u64>(flag);
}

#endif