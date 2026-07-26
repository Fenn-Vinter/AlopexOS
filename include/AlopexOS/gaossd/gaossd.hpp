#if !defined(GENERIC_ALOPEXOS_STORAGE_DRIVER)
#define GENERIC_ALOPEXOS_STORAGE_DRIVER

#include <primitives.hpp>
#include <arr.hpp>
#include <AlopexOS/AlopexOS.hpp>

namespace AlopexOS {
    class gaossd;
    struct AlopexOS_REQUEST_storage;
    struct StorageDeviceInfo;
    struct PhysicalStorage;

    using StorageSubmitHandler = errorCode(*)(const AlopexOS_REQUEST_storage&);
    using StorageProcessHandler = errorCode(*)();
}

struct AlopexOS::PhysicalStorage {
    enum class StorageType : u8 {
        NVME,
        SSD,
        HDD,
        DVD
    };

    PhysicalAddress address{0};
    StorageType type{StorageType::NVME};
    StorageSubmitHandler submit_fn{nullptr};
    StorageProcessHandler process_fn{nullptr};
    char name[64]{0};
};

struct AlopexOS::StorageDeviceInfo {
    BlockCount total_blocks{0};
    u32 block_size{512};
    bool read_only{false};
    char model_name[64]{0};
};

struct AlopexOS::AlopexOS_REQUEST_storage {
    enum class AlopexOS_OPCODE_storage : u8 {
        Read = 0,
        Write,
        Flush,
        Identify,
        Format
    };

    AlopexOS_OPCODE_storage OpCode{AlopexOS_OPCODE_storage::Read};
    Handle device_handle{InvalidHandle};
    LBA lba{0};
    BlockCount count{0};
    PhysicalAddress PhysAddr{0};
    errorCode status{errorCode::Success};
    bool completed{false};
};

class AlopexOS::gaossd {
public:
    static gaossd& get_instance();

    gaossd(const gaossd&) = delete;
    gaossd& operator=(const gaossd&) = delete;
    gaossd(gaossd&&) = delete;
    gaossd& operator=(gaossd&&) = delete;

    errorCode submit_request(AlopexOS_REQUEST_storage& request);

    template <typename... Requests>
    errorCode submit_request(Requests&... requests) {
        errorCode status = errorCode::Success;
        ((status = submit_request(requests)), ...);
        return status;
    }

    const PhysicalStorage* get_device(Handle handle) const {
        if (handle >= _storageDevices.size()) return nullptr;
        return &_storageDevices[handle];
    }

    size_t device_count() const {
        return _storageDevices.size();
    }

    errorCode process_queue();

private:
    gaossd();

    errorCode register_device(const PhysicalStorage& device, Handle& out_handle);
    errorCode scan_and_initialize_hardware();

    dynarr<PhysicalStorage> _storageDevices{};
    dynarr<AlopexOS_REQUEST_storage*> _queue{};
};

#endif