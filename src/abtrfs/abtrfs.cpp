#include <AlopexOS/abtrfs/abtrfs.hpp>
#include <AlopexOS/gaossd/gaossd.hpp>
#include <AlopexOS/PCI/nvme/nvme.hpp>

static inline void serial_out(u16 port, u8 val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void serial_print_abtr(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        serial_out(0x3F8, str[i]);
    }
}

namespace AlopexOS {

    auto AbtrFS::mount(uptr base_address, u64 hhdm_offset, Handle device_handle) -> bool {
        _device_base_address = base_address;
        _hhdm_offset = hhdm_offset;
        _device_handle = device_handle;

        serial_print_abtr("[ABTRFS] Attempting mount on device handle...\n");

        AlopexOS_REQUEST_storage req{};
        req.OpCode = AlopexOS_REQUEST_storage::AlopexOS_OPCODE_storage::Read;
        req.device_handle = _device_handle;
        req.lba = 0;
        req.count = 1;
        
        auto& nvme_ctrl = NVMe::Controller::get_instance();
        req.PhysAddr = nvme_ctrl.virt_to_phys(&_partition_header);

        auto& ssd = gaossd::get_instance();
        
        auto sub_res = ssd.submit_request(req);
        if (sub_res != errorCode::Success) {
            serial_print_abtr("[ABTRFS] ERROR: submit_request failed!\n");
            _is_mounted = false;
            return false;
        }

        auto queue_res = ssd.process_queue();
        if (queue_res != errorCode::Success) {
            serial_print_abtr("[ABTRFS] ERROR: process_queue failed!\n");
            _is_mounted = false;
            return false;
        }

        if (!req.completed) {
            serial_print_abtr("[ABTRFS] ERROR: Request not completed!\n");
            _is_mounted = false;
            return false;
        }

        if (req.status != errorCode::Success) {
            serial_print_abtr("[ABTRFS] ERROR: Request completed with error status!\n");
            _is_mounted = false;
            return false;
        }

        if (_partition_header.magic != ABTRFS_MAGIC_64) {
            serial_print_abtr("[ABTRFS] ERROR: Invalid magic number found in partition header!\n");
            _is_mounted = false;
            return false;
        }

        serial_print_abtr("[ABTRFS] Successfully mounted!\n");
        _is_mounted = true;
        return true;
    }

    auto AbtrFS::read_block(u64 block_address, void* buffer) -> bool {
        if (!_is_mounted) {
            return false;
        }

        AlopexOS_REQUEST_storage req{};
        req.OpCode = AlopexOS_REQUEST_storage::AlopexOS_OPCODE_storage::Read;
        req.device_handle = _device_handle;
        req.lba = block_address;
        req.count = 1;
        
        auto& nvme_ctrl = NVMe::Controller::get_instance();
        req.PhysAddr = nvme_ctrl.virt_to_phys(buffer);

        auto& ssd = gaossd::get_instance();
        if (ssd.submit_request(req) != errorCode::Success || ssd.process_queue() != errorCode::Success) {
            return false;
        }

        return req.completed && req.status == errorCode::Success;
    }

    auto AbtrFS::write_block(u64 block_address, const void* buffer) -> bool {
        if (!_is_mounted) {
            return false;
        }

        AlopexOS_REQUEST_storage req{};
        req.OpCode = AlopexOS_REQUEST_storage::AlopexOS_OPCODE_storage::Write;
        req.device_handle = _device_handle;
        req.lba = block_address;
        req.count = 1;
        
        auto& nvme_ctrl = NVMe::Controller::get_instance();
        req.PhysAddr = nvme_ctrl.virt_to_phys(const_cast<void*>(buffer));

        auto& ssd = gaossd::get_instance();
        if (ssd.submit_request(req) != errorCode::Success || ssd.process_queue() != errorCode::Success) {
            return false;
        }

        return req.completed && req.status == errorCode::Success;
    }

    auto AbtrFS::format(uptr base_address, u64 hhdm_offset, Handle device_handle) -> bool {
        _device_base_address = base_address;
        _hhdm_offset = hhdm_offset;
        _device_handle = device_handle;

        serial_print_abtr("[ABTRFS] Formatting device with valid header...\n");

        _partition_header = PARTITION_HEADER{};
        _partition_header.magic = ABTRFS_MAGIC_64;

        AlopexOS_REQUEST_storage req{};
        req.OpCode = AlopexOS_REQUEST_storage::AlopexOS_OPCODE_storage::Write;
        req.device_handle = _device_handle;
        req.lba = 0;
        req.count = 1;
        
        auto& nvme_ctrl = NVMe::Controller::get_instance();
        req.PhysAddr = nvme_ctrl.virt_to_phys(&_partition_header);

        auto& ssd = gaossd::get_instance();
        
        if (ssd.submit_request(req) != errorCode::Success || ssd.process_queue() != errorCode::Success) {
            serial_print_abtr("[ABTRFS] ERROR: Format write submission failed!\n");
            return false;
        }

        if (!req.completed || req.status != errorCode::Success) {
            serial_print_abtr("[ABTRFS] ERROR: Format write request failed!\n");
            return false;
        }

        serial_print_abtr("[ABTRFS] Successfully formatted and wrote partition header!\n");
        return true;
    }
}