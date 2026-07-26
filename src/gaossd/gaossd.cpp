#include <AlopexOS/gaossd/gaossd.hpp>
#include <AlopexOS/AlopexOS.hpp>
#include <AlopexOS/PCI/nvme/nvme.hpp>
#include <AlopexOS/AlopexIBus/AlopexIBus.hpp>
#include <AlopexOS/limine_requests.hpp>

namespace {
    inline void serial_out(u16 port, u8 val) {
        asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
    }

    void serial_print(const char* str) {
        for (int i = 0; str[i] != '\0'; i++) {
            serial_out(0x3F8, str[i]);
        }
    }

    auto nvme_submit_thunk(const AlopexOS::AlopexOS_REQUEST_storage& req) -> AlopexOS::errorCode {
        auto& ctrl = AlopexOS::NVMe::Controller::get_instance();
        switch (req.OpCode) {
            case AlopexOS::AlopexOS_REQUEST_storage::AlopexOS_OPCODE_storage::Read:
                return ctrl.read(req.PhysAddr, req.lba, req.count);
            case AlopexOS::AlopexOS_REQUEST_storage::AlopexOS_OPCODE_storage::Write:
                return ctrl.write(req.PhysAddr, req.lba, req.count);
            case AlopexOS::AlopexOS_REQUEST_storage::AlopexOS_OPCODE_storage::Identify:
                return ctrl.identify() && ctrl.identify_namespace() ? 
                       AlopexOS::errorCode::Success : AlopexOS::errorCode::Generic_Error;
            case AlopexOS::AlopexOS_REQUEST_storage::AlopexOS_OPCODE_storage::Format:
                return ctrl.format_namespace() ? 
                       AlopexOS::errorCode::Success : AlopexOS::errorCode::Generic_Error;
            default:
                break;
        }
        return AlopexOS::errorCode::InvalidParameters;
    }

    auto nvme_process_thunk() -> AlopexOS::errorCode {
        return AlopexOS::errorCode::Success;
    }
}

void print_field(const char* label, const char* str, size_t len) {
    serial_print(label);
    for (size_t i = 0; i < len && str[i] != '\0'; i++) {
        serial_out(0x3F8, str[i]);
    }
    serial_print("\n");
}

AlopexOS::gaossd::gaossd() {
    uptr hhdm_offset = 0;
    if (hhdm_request.response != nullptr) {
        hhdm_offset = hhdm_request.response->offset;
        serial_print("[GAOSSD] Limine HHDM offset retrieved successfully.\n");
    } else {
        serial_print("[GAOSSD] WARNING: Limine HHDM response is null!\n");
    }

    auto& IBus = AlopexOS::AlopexIBus::get_instance();
    
    if (IBus.getPCIe()->is_active()) {
        serial_print("[GAOSSD] PCIe Subsystem active! Searching for NVMe BAR0...\n");
        uptr nvme_bar0 = IBus.getPCIe()->find_nvme_bar0();
        if (nvme_bar0) {
            serial_print("[GAOSSD] NVMe Controller found! Initializing NVMe driver...\n");
            
            auto& nvme = AlopexOS::NVMe::Controller::get_instance();
            if (nvme.init(nvme_bar0, hhdm_offset)) {
                serial_print("[GAOSSD] NVMe Controller successfully initialized, READY, and Identified!\n");
                
                const auto& info = nvme.get_identify_data();
                print_field("[GAOSSD] Model Number:  ", info.mn, sizeof(info.mn));
                print_field("[GAOSSD] Serial Number: ", info.sn, sizeof(info.sn));
                print_field("[GAOSSD] Firmware Rev:  ", info.fr, sizeof(info.fr));

                PhysicalStorage storage_dev{};
                storage_dev.submit_fn = nvme_submit_thunk;
                storage_dev.process_fn = nvme_process_thunk;

                size_t copy_len = sizeof(info.mn);
                if (copy_len >= sizeof(storage_dev.name)) {
                    copy_len = sizeof(storage_dev.name) - 1;
                }

                for (size_t i = 0; i < copy_len; i++) {
                    storage_dev.name[i] = info.mn[i];
                }
                storage_dev.name[copy_len] = '\0';

                for (int i = static_cast<int>(copy_len) - 1; i >= 0; i--) {
                    if (storage_dev.name[i] == ' ' || storage_dev.name[i] == '\t' || storage_dev.name[i] == '\r' || storage_dev.name[i] == '\n') {
                        storage_dev.name[i] = '\0';
                    } else {
                        break;
                    }
                }

                Handle h = 0;
                register_device(storage_dev, h);
            } else {
                serial_print("[GAOSSD] ERROR: NVMe Controller initialization or Identify failed!\n");
            }
        } else {
            serial_print("[GAOSSD] PCIe Subsystem initialized, but no NVMe BAR0 found.\n");
        }
    } else {
        serial_print("[GAOSSD] PCIe Subsystem failed to activate.\n");
    }
}

auto AlopexOS::gaossd::get_instance() -> AlopexOS::gaossd& {
    static gaossd instance;
    return instance;
}

auto AlopexOS::gaossd::register_device(const PhysicalStorage& device, Handle& out_handle) -> AlopexOS::errorCode {
    if (device.submit_fn == nullptr || device.process_fn == nullptr) {
        return errorCode::InvalidParameters;
    }

    _storageDevices.push_back(device);
    out_handle = static_cast<Handle>(_storageDevices.size() - 1);
    return errorCode::Success;
}

auto AlopexOS::gaossd::scan_and_initialize_hardware() -> AlopexOS::errorCode {
    return errorCode::Success;
}

auto AlopexOS::gaossd::submit_request(AlopexOS_REQUEST_storage& request) -> errorCode {
    if (request.device_handle >= _storageDevices.size()) {
        return errorCode::InvalidParameters;
    }

    _queue.push_back(&request);
    return errorCode::Success;
}

auto AlopexOS::gaossd::process_queue() -> errorCode {
    if (_queue.empty()) {
        return errorCode::Success;
    }

    for (size_t i = 0; i < _queue.size(); ++i) {
        AlopexOS_REQUEST_storage* req_ptr = _queue[i];
        if (!req_ptr) continue;

        AlopexOS_REQUEST_storage& req = *req_ptr;

        if (req.completed) {
            continue;
        }

        if (req.device_handle >= _storageDevices.size()) {
            req.status = errorCode::InvalidParameters;
            req.completed = true;
            continue;
        }

        PhysicalStorage& device = _storageDevices[req.device_handle];

        req.status = device.submit_fn(req);
        if (req.status == errorCode::Success) {
            req.status = device.process_fn();
        }

        req.completed = true;
    }

    _queue.clear();
    return errorCode::Success;
}