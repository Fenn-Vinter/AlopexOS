#if !defined(ERROR_CODE_HPP)
#define ERROR_CODE_HPP

#include <primitives.h>

namespace AlopexOS {

enum class errorCode : u64 {
    Success = 0,

    // Core / Generic Errors (1 - 99)
    Generic_Error       = static_cast<u64>(-1),
    InvalidParameters   = 1,
    NotImplemented      = 2,
    PermissionDenied    = 3,
    Timeout             = 4,
    Busy                = 5,
    AlreadyExists       = 6,
    NotFound            = 7,

    // Memory / System (100 - 199)
    OutOfMemory         = 100,
    InvalidAddress      = 101,
    AlignmentFault      = 102,
    PageFault           = 103,

    // Hardware / Device Driver (200 - 299)
    DeviceNotReady      = 200,
    DeviceTimeout       = 201,
    DeviceInitializationFailed = 202,
    IOError             = 203,
    HardwareFault       = 204,

    // Storage / Block I/O (300 - 399)
    InvalidLBA          = 300,
    OutOfBounds         = 301,
    BufferUnaligned     = 302,
    WriteProtected      = 303,

    // Filesystem / VFS (400 - 499)
    InvalidSuperblock   = 400,
    CorruptedHeader     = 401,
    NodeNotFound        = 402,
    NoSpaceLeft         = 403,
    ReadOnlyFilesystem  = 404,
    IsDirectory         = 405,
    NotDirectory        = 406,
    FileNotFound        = 407,
    FileAlreadyExists   = 408,
    BufferTooSmall      = 409,
    DeviceNotMounted    = 410,
    InvalidPath         = 411,
    AllocationFailed    = 412,
    WriteFailed         = 413,
};

}

#endif