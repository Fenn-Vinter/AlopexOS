#include "AlopexOS/AlopexOS_ErrorCodes.hpp"
#include "AlopexOS/gaossd/gaossd.hpp"
#if !defined (ALOPEXOS_VIRTUAL_FILE_SYSTEM)
#define ALOPEXOS_VIRTUAL_FILE_SYSTEM

#include <primitives.hpp>
#include <arr.hpp>
#include <AlopexOS/AlopexOS.hpp>
#include <string.hpp>

namespace AlopexOS {
    class AVFS;
}

class AlopexOS::AVFS {
    class drive;
    dynarr<drive> drives{};

    u64 hhdm{};
    
    fn findDrive(const Path& path) -> AurenFox::core::Expected<string16, errorCode>;
    fn retrieveDrive(const string16& drive) -> AurenFox::core::Expected<uptr, errorCode>;
    fn findPathAfterDrive(const Path& path) -> AurenFox::core::Expected<Path, errorCode>;

    gaossd* _gaossd{nullptr};
    
    public:
        AVFS() = default;
        ~AVFS() = default;

        fn Init() -> void;
        fn mount(const Path& path) -> errorCode;
        fn dismount(const Path& path) -> errorCode;

        fn write(const Path& path, const dynarr<byte>& data) -> errorCode;
        fn read(const Path& path, const dynarr<byte>& data) -> errorCode;
        fn exists(const Path& path) -> errorCode;

        fn attach_gaossd(gaossd* Gaossd) -> errorCode;
        fn gaossd_instance() -> gaossd*;
};

class AlopexOS::AVFS::drive {
    public:
        fn BaseAdress_Get() -> uptr*;
        fn BaseAdress_Get(uptr* base_address) -> uptr*;
        fn BaseAdress_Cpy() -> uptr;
        fn BaseAdress_Set(const uptr& base_address) -> errorCode;

        fn HHDMOffset_Get() -> u64*;
        fn HHDMOffset_Get(u64* hddm_offset) -> u64*;
        fn HHDMOffset_Cpy() -> u64;
        fn HHDMOffset_Set(const u64& hddm_offset) -> errorCode;

        fn DeviceHandle_Get() -> u64*;
        fn DeviceHandle_Get(u64* device_handle) -> u64*;
        fn DeviceHandle_Cpy() -> u64;
        fn DeviceHandle_Set(const u64& device_handle) -> errorCode;

        fn Name_Get() -> string*;
        fn Name_Get(string* name) -> string*;
        fn Name_Cpy() -> string;
        fn Name_Set(const string& name) -> errorCode;

        fn Format_Get() -> string8*;
        fn Format_Get(string8* format) -> string8*;
        fn Format_Cpy() -> string8;
        fn Format_Set(const string8& format) -> errorCode;
    private:
        uptr base_address{};
        u64 hhdm_offset{};
        Handle device_handle{};
        string name{};
        string8 format{};
};

#endif // AlopexOS Virtual File System