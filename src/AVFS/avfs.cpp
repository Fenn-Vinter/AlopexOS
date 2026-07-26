#include "AlopexOS/AlopexIBus/AlopexIBus.hpp"
#include "AlopexOS/AlopexOS_ErrorCodes.hpp"
#include "AlopexOS/gaossd/gaossd.hpp"
#include "expected.hpp"
#include "primitives.hpp"
#include "string.hpp"
#include <AlopexOS/AVFS/avfs.hpp>
#include <cstddef>
#include <AlopexOS/abtrfs/abtrfs.hpp>

namespace {
    static inline auto serial_out(u16 port, u8 val) -> void {
        asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
    }

    static auto serial_print(const char* str) -> void {
        for (int i = 0; str[i] != '\0'; i++) {
            serial_out(0x3F8, str[i]);
        }
    }

    static auto serial_print_hex(u64 val) -> void {
        char buf[19];
        buf[18] = '\0';
        for (int i = 17; i >= 2; --i) {
            u8 nibble = val & 0xF;
            buf[i] = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
            val >>= 4;
        }
        buf[0] = '0';
        buf[1] = 'x';
        serial_print(buf);
    }
}

AlopexOS::AVFS::AVFS() {
    errorCode err{};
    serial_print("\033[36m[AVFS] Initializing!!!\033[0m\n");
    auto& ibus = AlopexOS::AlopexIBus::get_instance();
    err = ibus.scan_all_buses();
    serial_print("\033[36m[AVFS] scanning busses: \033[0m");
    serial_print(AlopexOS::returnErrorAsCstring(err));
    serial_print("\n");

    if (ibus.get_storage_devices().empty()) {
        serial_print("\033[33m[AVFS] WARNING: No storage devices found by IBus!\033[0m\n");
    } else {
        serial_print("\033[36m[AVFS] IBus reported storage device count: \033[0m");
        serial_print_hex(ibus.get_storage_devices().size());
        serial_print("\n");
    }

    this->GAOSSD = &gaossd::get_instance();
    
    serial_print("\033[36m[AVFS] GAOSSD registered device count: \033[0m");
    serial_print_hex(GAOSSD->device_count());
    serial_print("\n");

    hhdm = ibus.get_hhdm_offset();

    for (size_t i = 0; i < ibus.get_storage_devices().size(); i++) {
        auto& storage_device = ibus.get_storage_devices()[i];

        serial_print("\033[35m[AVFS] --- Inspecting IBus Storage Device [\033[0m");
        serial_print_hex(i);
        serial_print("\033[35m] ---\033[0m\n");
        serial_print("\033[36m[AVFS] IBus Device Base Address: \033[0m");
        serial_print_hex(storage_device.base_address);
        serial_print("\n");

        Handle matched_handle = InvalidHandle;
        const AlopexOS::PhysicalStorage* matched_dev = nullptr;

        for (size_t h = 0; h < GAOSSD->device_count(); ++h) {
            const auto* dev = GAOSSD->get_device(h);
            if (dev) {
                serial_print("\033[36m[AVFS]   -> Comparing against GAOSSD device [\033[0m");
                serial_print_hex(h);
                serial_print("\033[36m] addr: \033[0m");
                serial_print_hex(dev->address);
                serial_print("\033[36m name: \033[0m");
                serial_print(dev->name[0] != '\0' ? dev->name : "<EMPTY>");
                serial_print("\n");

                if (dev->address == storage_device.base_address) {
                    matched_handle = static_cast<Handle>(h);
                    matched_dev = dev;
                    serial_print("\033[32m[AVFS]   ==> MATCH FOUND by base address at handle: \033[0m");
                    serial_print_hex(matched_handle);
                    serial_print("\n");
                    break;
                }
            }
        }

        if (matched_handle == InvalidHandle && GAOSSD->device_count() > 0) {
            matched_handle = 0;
            matched_dev = GAOSSD->get_device(0);
            serial_print("\033[33m[AVFS] WARNING: Fallback to GAOSSD device handle 0\033[0m\n");
        }

        if (!matched_dev) {
            serial_print("\033[31m[AVFS] ERROR: No valid GAOSSD device found to extract name!\033[0m\n");
            continue;
        }

        const char* device_name = (matched_dev->name[0] != '\0') ? matched_dev->name : "ALOPEX_NVME_01";

        auto exists = [&](dynarr<drive> drives) -> bool {
            if (drives.empty()) return false;
            for (auto& d : drives) {
                if (d.Name_Get() && d.Name_Get()->c_str() && d.Name_Get()->c_str() == device_name) return true;
            }
            return false;
        };

        if (exists(this->drives)) {
            serial_print("\033[33m[AVFS] Drive already registered, skipping duplicate.\033[0m\n");
            continue;
        }

        drive d;
        d.BaseAdress_Set(storage_device.base_address);
        d.Name_Set(device_name);
        drives.push_back(d);
        
        serial_print("\033[32m[AVFS] Successfully registered drive with name from GAOSSD\033[0m\n");
    }
}

fn AlopexOS::AVFS::findDrive(const Path& path) -> AurenFox::core::Expected<string16, errorCode> {
    serial_print("\033[36m[AVFS] findDrive entering path lookup...\033[0m\n");
    
    string16 driveName{};
    for (size_t i = 0; i < path.size(); i++) {
        char c = static_cast<char>(path[i]);
        if (c == '\0') {
            break;
        }
        
        if (c == ':') {
            if (i + 1 < path.size() && static_cast<char>(path[i + 1]) == '/') {
                if (i + 2 < path.size() && static_cast<char>(path[i + 2]) == '/') {
                    size_t len = 0;
                    while (driveName.c_str() && driveName.c_str()[len] != '\0') {
                        len++;
                    }
                    while (len > 0 && driveName.c_str()[len - 1] == ' ') {
                        len--;
                    }
                    string16 trimmedDriveName{};
                    for (size_t j = 0; j < len; j++) {
                        trimmedDriveName += driveName.c_str()[j];
                    }

                    serial_print("\033[32m[AVFS] findDrive extracted name: '\033[0m");
                    for (size_t k = 0; k < len; k++) {
                        char ch = static_cast<char>(trimmedDriveName.c_str()[k]);
                        serial_out(0x3F8, ch);
                    }
                    serial_print("\033[32m'\033[0m\n");

                    return AurenFox::core::Expected<string16, errorCode>(trimmedDriveName);
                }
            }
            serial_print("\033[31m[AVFS] findDrive error: colon found but missing '//' sequence.\033[0m\n");
            return errorCode::Generic_Error;
        }
        driveName += c;
    }
    serial_print("\033[31m[AVFS] findDrive failed to locate colon separator in path!\033[0m\n");
    return errorCode::Generic_Error;
}

fn AlopexOS::AVFS::findPathAfterDrive(const Path& path) -> AurenFox::core::Expected<Path, errorCode> {
    serial_print("\033[36m[AVFS] findPathAfterDrive entering path lookup...\033[0m\n");
    
    Path directory{};
    bool found_separator = false;
    size_t start_index = 0;

    for (size_t i = 0; i < path.size(); i++) {
        char c = static_cast<char>(path[i]);
        if (c == '\0') {
            break;
        }

        if (c == ':') {
            if (i + 1 < path.size() && static_cast<char>(path[i + 1]) == '/') {
                if (i + 2 < path.size() && static_cast<char>(path[i + 2]) == '/') {
                    start_index = i + 3;
                    found_separator = true;
                    break;
                }
            }
            serial_print("\033[31m[AVFS] findPathAfterDrive error: colon found but missing '//' sequence.\033[0m\n");
            return errorCode::Generic_Error;
        }
    }

    if (!found_separator) {
        serial_print("\033[31m[AVFS] findPathAfterDrive failed to locate colon separator in path!\033[0m\n");
        return errorCode::Generic_Error;
    }

    size_t dir_index = 0;
    for (size_t i = start_index; i < path.size(); i++) {
        char c = static_cast<char>(path[i]);
        directory[dir_index++] = c;
        if (c == '\0') {
            break;
        }
    }
    
    if (dir_index < path.size()) {
        directory[dir_index] = '\0';
    }

    serial_print("\033[32m[AVFS] findPathAfterDrive extracted subpath successfully.\033[0m\n");
    return AurenFox::core::Expected<Path, errorCode>(directory);
}

fn AlopexOS::AVFS::retrieveDrive(const string16& drive) -> AurenFox::core::Expected<uptr, errorCode> {
    serial_print("\033[36m[AVFS] retrieveDrive searching registered drives list...\033[0m\n");
    for (auto& d : this->drives) {
        if (d.Name_Get() && d.Name_Get()->c_str()) {
            string16 storedName{};
            for (int k = 0; d.Name_Get()->c_str()[k] != '\0'; k++) {
                storedName += d.Name_Get()->c_str()[k];
            }
            if (storedName == drive) {
                serial_print("\033[32m[AVFS] retrieveDrive: Match found for drive!\033[0m\n");
                return AurenFox::core::Expected<uptr, errorCode>(reinterpret_cast<uptr>(&d));
            }
        }
    }
    serial_print("\033[31m[AVFS] retrieveDrive error: Drive not found in registered drives list.\033[0m\n");
    return errorCode::NotFound;
}

fn AlopexOS::AVFS::mount(const Path& path) -> errorCode {
    serial_print("\033[36m[AVFS] mount invoked with path target.\033[0m\n");
    auto driveName = findDrive(path);
    if (!driveName.has_value()) {
        serial_print("\033[31m[AVFS] Coud not find drive from path\033[0m\n");
        return driveName.error();
    }

    auto driveptr = retrieveDrive(driveName.value());
    if (!driveptr.has_value()) {
        serial_print("\033[31m[AVFS] Drive name not registered in AVFS drives list\033[0m\n");
        return driveptr.error();
    }

    AlopexOS::AVFS::drive* drive = reinterpret_cast<AlopexOS::AVFS::drive*>(driveptr.value());

    Handle matched_handle = InvalidHandle;
    for (size_t h = 0; h < GAOSSD->device_count(); ++h) {
        const auto* dev = GAOSSD->get_device(h);
        if (dev && dev->address == *drive->BaseAdress_Get()) {
            matched_handle = static_cast<Handle>(h);
            break;
        }
    }

    if (matched_handle == InvalidHandle && GAOSSD->device_count() > 0) {
            matched_handle = 0;
            serial_print("\033[33m[AVFS] mount warning: Falling back to GAOSSD device handle 0.\033[0m\n");
    }

    if (matched_handle == InvalidHandle) {
        serial_print("\033[31m[AVFS] ERROR: Failed to resolve valid GAOSSD handle for drive mount!\033[0m\n");
        return errorCode::DeviceNotMounted;
    }

    errorCode error{};

    {
        serial_print("\033[36m[AVFS] Attempting AbtrFS mount sequence...\033[0m\n");
        AbtrFS abtrfs;
        abtrfs.mount(*drive->BaseAdress_Get(), hhdm, matched_handle);
        error = abtrfs.is_mounted() ? errorCode::Success : errorCode::DeviceNotMounted;

        if (error == errorCode::Success) {
            serial_print("\033[32m[AVFS] AbtrFS successfully mounted, setting format to 'abtrfs'.\033[0m\n");
            drive->Format_Set("abtrfs");
        } else {
            serial_print("\033[31m[AVFS] AbtrFS mount failed verification check.\033[0m\n");
        }
    }

    return error;
}

fn AlopexOS::AVFS::write(const Path& path, const dynarr<byte>& data) -> errorCode {
    serial_print("\033[36m[AVFS:write] write invoked with path target.\033[0m\n");
    auto driveName = findDrive(path);
    if (!driveName.has_value()) {
        serial_print("\033[31m[AVFS:write] Coud not find drive from path\033[0m\n");
        return driveName.error();
    }

    auto driveptr = retrieveDrive(driveName.value());
    if (!driveptr.has_value()) {
        serial_print("\033[31m[AVFS:write] Drive name not registered in AVFS drives list\033[0m\n");
        return driveptr.error();
    }

    AlopexOS::AVFS::drive* drive = reinterpret_cast<AlopexOS::AVFS::drive*>(driveptr.value());

    Handle matched_handle = InvalidHandle;
    for (size_t h = 0; h < GAOSSD->device_count(); ++h) {
        const auto* dev = GAOSSD->get_device(h);
        if (dev && dev->address == *drive->BaseAdress_Get()) {
            matched_handle = static_cast<Handle>(h);
            break;
        }
    }

    if (matched_handle == InvalidHandle && GAOSSD->device_count() > 0) {
            matched_handle = 0;
            serial_print("\033[33m[AVFS:write] Falling back to GAOSSD device handle 0.\033[0m\n");
    }

    if (matched_handle == InvalidHandle) {
        serial_print("\033[31m[AVFS:write] ERROR: Failed to resolve valid GAOSSD handle for drive!\033[0m\n");
        return errorCode::DeviceNotMounted;
    }

    auto directory = findPathAfterDrive(path);

    if (!directory.has_value()) {
        serial_print("\033[31m[AVFS:write] Could not load the directory from path ->");
        serial_print(returnErrorAsCstring(directory.error()));
        serial_print("\033[0m\n");
        return directory.error();
    }

    // Automatically normalize/sanitize the payload data buffer to guarantee
    // clean physical layout and exact byte sizing for NVMe block alignment.
    dynarr<byte> clean_data{};
    clean_data.reserve(data.size());
    for (size_t i = 0; i < data.size(); i++) {
        if (data[i] != '\0') {
            clean_data.push_back(data[i]);
        }
    }

    AbtrFS abtrfs;
    abtrfs.mount_existing(*drive->BaseAdress_Get(), hhdm, matched_handle);

    return abtrfs.write_file(directory.value(), clean_data);
}

fn AlopexOS::AVFS::read(const Path& path, const dynarr<byte>& data) -> errorCode {
    serial_print("\033[36m[AVFS:read] read invoked with path target.\033[0m\n");
    auto driveName = findDrive(path);
    if (!driveName.has_value()) {
        serial_print("\033[31m[AVFS:read] Could not find drive from path\033[0m\n");
        return driveName.error();
    }

    auto driveptr = retrieveDrive(driveName.value());
    if (!driveptr.has_value()) {
        serial_print("\033[31m[AVFS:read] Drive name not registered in AVFS drives list\033[0m\n");
        return driveptr.error();
    }

    AlopexOS::AVFS::drive* drive = reinterpret_cast<AlopexOS::AVFS::drive*>(driveptr.value());

    Handle matched_handle = InvalidHandle;
    for (size_t h = 0; h < GAOSSD->device_count(); ++h) {
        const auto* dev = GAOSSD->get_device(h);
        if (dev && dev->address == *drive->BaseAdress_Get()) {
            matched_handle = static_cast<Handle>(h);
            break;
        }
    }

    if (matched_handle == InvalidHandle && GAOSSD->device_count() > 0) {
        matched_handle = 0;
        serial_print("\033[33m[AVFS:read] Falling back to GAOSSD device handle 0.\033[0m\n");
    }

    if (matched_handle == InvalidHandle) {
        serial_print("\033[31m[AVFS:read] ERROR: Failed to resolve valid GAOSSD handle for drive!\033[0m\n");
        return errorCode::DeviceNotMounted;
    }

    auto directory = findPathAfterDrive(path);
    if (!directory.has_value()) {
        serial_print("\033[31m[AVFS:read] Could not load directory from path\033[0m\n");
        return directory.error();
    }

    AbtrFS abtrfs;
    abtrfs.mount_existing(*drive->BaseAdress_Get(), hhdm, matched_handle);

    // Pass the address of the buffer view, starting offset 0, and max_read_size = data.size()
    auto* writable_buffer = const_cast<dynarr<byte>*>(&data);
    auto result = abtrfs.read_file(directory.value(), writable_buffer, 0, data.size());

    if (!result.has_value()) {
        return result.error();
    }

    return errorCode::Success;
}

fn AlopexOS::AVFS::exists(const Path& path) -> errorCode {
    serial_print("\033[36m[AVFS:exists] Checking file existence...\033[0m\n");
    auto driveName = findDrive(path);
    if (!driveName.has_value()) {
        return driveName.error();
    }

    auto driveptr = retrieveDrive(driveName.value());
    if (!driveptr.has_value()) {
        return driveptr.error();
    }

    return errorCode::Success;
}

fn AlopexOS::AVFS::dismount(const Path& path) -> errorCode {
    serial_print("\033[36m[AVFS:dismount] Dismounting filesystem target...\033[0m\n");
    auto driveName = findDrive(path);
    if (!driveName.has_value()) {
        return driveName.error();
    }

    auto driveptr = retrieveDrive(driveName.value());
    if (!driveptr.has_value()) {
        return driveptr.error();
    }

    AlopexOS::AVFS::drive* drive = reinterpret_cast<AlopexOS::AVFS::drive*>(driveptr.value());
    drive->Format_Set("");

    return errorCode::Success;
}
