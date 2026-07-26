#include <AlopexOS/AlopexOS_ErrorCodes.hpp>

fn AlopexOS::returnErrorAsCstring(errorCode err) -> const char* {
    if (err == errorCode::Success) return "Err::Success";
    if (err == errorCode::Generic_Error) return "Err::Generic_Error";
    if (err == errorCode::InvalidParameters) return "Err::InvalidParameters";
    if (err == errorCode::NotImplemented) return "Err::NotImplemented";
    if (err == errorCode::PermissionDenied) return "Err::PermissionDenied";
    if (err == errorCode::Timeout) return "Err::Timeout";
    if (err == errorCode::Busy) return "Err::Busy";
    if (err == errorCode::AlreadyExists) return "Err::AlreadyExists";
    if (err == errorCode::NotFound) return "Err::NotFound";
    if (err == errorCode::OutOfMemory) return "Err::OutOfMemory";
    if (err == errorCode::InvalidAddress) return "Err::InvalidAddress";
    if (err == errorCode::AlignmentFault) return "Err::AlignmentFault";
    if (err == errorCode::PageFault) return "Err::PageFault";
    if (err == errorCode::DeviceNotReady) return "Err::DeviceNotReady";
    if (err == errorCode::DeviceTimeout) return "Err::DeviceTimeout";
    if (err == errorCode::DeviceInitializationFailed) return "Err::DeviceInitializationFailed";
    if (err == errorCode::IOError) return "Err::IOError";
    if (err == errorCode::HardwareFault) return "Err::HardwareFault";
    if (err == errorCode::InvalidLBA) return "Err::InvalidLBA";
    if (err == errorCode::OutOfBounds) return "Err::OutOfBounds";
    if (err == errorCode::BufferUnaligned) return "Err::BufferUnaligned";
    if (err == errorCode::WriteProtected) return "Err::WriteProtected";
    if (err == errorCode::InvalidSuperblock) return "Err::InvalidSuperblock";
    if (err == errorCode::CorruptedHeader) return "Err::CorruptedHeader";
    if (err == errorCode::NodeNotFound) return "Err::NodeNotFound";
    if (err == errorCode::NoSpaceLeft) return "Err::NoSpaceLeft";
    if (err == errorCode::ReadOnlyFilesystem) return "Err::ReadOnlyFilesystem";
    if (err == errorCode::IsDirectory) return "Err::IsDirectory";
    if (err == errorCode::NotDirectory) return "Err::NotDirectory";
    if (err == errorCode::FileNotFound) return "Err::FileNotFound";
    if (err == errorCode::FileAlreadyExists) return "Err::FileAlreadyExists";
    if (err == errorCode::BufferTooSmall) return "Err::BufferTooSmall";
    if (err == errorCode::DeviceNotMounted) return "Err::DeviceNotMounted";
    if (err == errorCode::InvalidPath) return "Err::InvalidPath";
    if (err == errorCode::AllocationFailed) return "Err::AllocationFailed";
    if (err == errorCode::WriteFailed) return "Err::WriteFailed";
    return "Err::Error_code_not_implemented_yet";
}