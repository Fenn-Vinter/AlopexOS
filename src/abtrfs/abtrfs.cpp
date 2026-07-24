#include "AlopexOS/AlopexOS_ErrorCodes.hpp"
#include "arr.hpp"
#include <AlopexOS/abtrfs/abtrfs.hpp>
#include <AlopexOS/gaossd/gaossd.hpp>
#include <AlopexOS/PCI/nvme/nvme.hpp>
#include <expected.hpp>
#include <string.hpp>
#include <AlopexOS/abtrfs/format.hpp>
#include <memory.hpp>

static inline void serial_out(u16 port, u8 val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void serial_print_abtr(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        serial_out(0x3F8, str[i]);
    }
}

fn AlopexOS::AbtrFS::mount(uptr base_address, u64 hhdm_offset, Handle device_handle) -> bool {
    _device_base_address = base_address;
    _hhdm_offset = hhdm_offset;
    _device_handle = device_handle;

    serial_print_abtr("[ABTRFS] Attempting mount on device handle...\n");

    AlopexOS_REQUEST_storage req{};
    req.OpCode = AlopexOS_REQUEST_storage::AlopexOS_OPCODE_storage::Read;
    req.device_handle = _device_handle;
    req.lba = 0;
    req.count = 8; // 4096 bytes / 512 bytes per sector = 8 sectors
        
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

fn AlopexOS::AbtrFS::read_block(u64 block_address, void* buffer) -> bool {
    if (!_is_mounted) {
        return false;
    }

    AlopexOS_REQUEST_storage req{};
    req.OpCode = AlopexOS_REQUEST_storage::AlopexOS_OPCODE_storage::Read;
    req.device_handle = _device_handle;
    req.lba = block_address * 8; // Scale 4096-byte logical block to 512-byte LBA sectors
    req.count = 8;
        
    auto& nvme_ctrl = NVMe::Controller::get_instance();
    req.PhysAddr = nvme_ctrl.virt_to_phys(buffer);

    auto& ssd = gaossd::get_instance();
    if (ssd.submit_request(req) != errorCode::Success || ssd.process_queue() != errorCode::Success) {
        return false;
    }

    return req.completed && req.status == errorCode::Success;
}

fn AlopexOS::AbtrFS::write_block(u64 block_address, const void* buffer) -> bool {
    if (!_is_mounted) {
        return false;
    }

    AlopexOS_REQUEST_storage req{};
    req.OpCode = AlopexOS_REQUEST_storage::AlopexOS_OPCODE_storage::Write;
    req.device_handle = _device_handle;
    req.lba = block_address * 8; // Scale 4096-byte logical block to 512-byte LBA sectors
    req.count = 8;
        
    auto& nvme_ctrl = NVMe::Controller::get_instance();
    req.PhysAddr = nvme_ctrl.virt_to_phys(const_cast<void*>(buffer));

    auto& ssd = gaossd::get_instance();
    if (ssd.submit_request(req) != errorCode::Success || ssd.process_queue() != errorCode::Success) {
        return false;
    }

    return req.completed && req.status == errorCode::Success;
}

fn AlopexOS::AbtrFS::format(uptr base_address, u64 hhdm_offset, Handle device_handle) -> bool {
    _device_base_address = base_address;
    _hhdm_offset = hhdm_offset;
    _device_handle = device_handle;

    serial_print_abtr("[ABTRFS] Formatting device with valid header...\n");

    _partition_header = PARTITION_HEADER{};
    _partition_header.magic = ABTRFS_MAGIC_64;
    _partition_header.total_blocks = 16384;
    _partition_header.free_blocks_count = _partition_header.total_blocks - 1;
    _partition_header.root_node_block = 0;

    AlopexOS_REQUEST_storage req{};
    req.OpCode = AlopexOS_REQUEST_storage::AlopexOS_OPCODE_storage::Write;
    req.device_handle = _device_handle;
    req.lba = 0;
    req.count = 8; // 8 sectors for the 4096-byte partition header block
        
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

[[nodiscard]] fn chopPath(const AlopexOS::Path& path) -> AlopexOS::Expected<dynarr<AlopexOS::string>> {
    dynarr<AlopexOS::string> result{};
    AlopexOS::string obj{};
    
    for (int i = 0; i < 2048 && path[i] != '\0'; i++) {
        char c = static_cast<char>(path[i]);
        if (c == '/') {
            if (!obj.empty()) {
                result.push_back(obj);
                obj.clear();
            }
        } else {
            obj += c;
        }
    }
    
    if (!obj.empty()) {
        result.push_back(obj);
    }

    return AlopexOS::Expected<dynarr<AlopexOS::string>>(result);
}

static fn find_or_create_node(TreeNode*& root, const dynarr<AlopexOS::string>& segments, u64 index) -> AlopexOS::Expected<TreeNode*> {
    if (index >= segments.size()) {
        return AlopexOS::Expected<TreeNode*>(root);
    }

    const auto& target_name = segments[index];
    bool is_last = (index == segments.size() - 1);

    if (!root) {
        root = new TreeNode{target_name, !is_last, nullptr, nullptr};
    }

    if (target_name == root->name) {
        if (is_last) {
            return AlopexOS::Expected<TreeNode*>(root);
        }
        return find_or_create_node(root->left, segments, index + 1);
    } else if (target_name < root->name) {
        return find_or_create_node(root->left, segments, index);
    } else {
        return find_or_create_node(root->right, segments, index);
    }
}

fn AlopexOS::AbtrFS::write_file(const AlopexOS::Path& path, const dynarr<byte>& data) -> AlopexOS::errorCode {
    if (!_is_mounted) return AlopexOS::errorCode::DeviceNotMounted;

    auto choppedPath = chopPath(path);
    if (!choppedPath.has_value()) return choppedPath.error();

    const auto& segments = choppedPath.value();
    if (segments.size() == 0) return AlopexOS::errorCode::InvalidPath;

    auto target_node = find_or_create_node(_root_node, segments, 0);
    if (!target_node.has_value()) {
        return AlopexOS::errorCode::AllocationFailed;
    }

    u64 data_blocks_count = (data.size() + ABTRFS_HEADER_BLOCK_SIZE - 1) / ABTRFS_HEADER_BLOCK_SIZE;
    if (data_blocks_count == 0) {
        data_blocks_count = 1; 
    }

    AbtrFSDiskHeader* file_header = new AbtrFSDiskHeader{};
    file_header->base.magic = 0xABF5;
    file_header->base.flags = AbtrfsDiskFlags::IsFile;
    
    file_header->section_count = data_blocks_count;
    u64 remainder = data.size() % ABTRFS_HEADER_BLOCK_SIZE;
    file_header->size = static_cast<u16>(remainder == 0 && data.size() > 0 ? ABTRFS_HEADER_BLOCK_SIZE : remainder);
    
    file_header->block_size = ABTRFS_HEADER_BLOCK_SIZE;

    int p_idx = 0;
    for (int i = 0; i < 2048 && path[i] != '\0' && p_idx < 255; i++) {
        file_header->path[p_idx++] = static_cast<char>(path[i]);
    }
    file_header->path[p_idx] = '\0';

    u64 assigned_header_block = _partition_header.total_blocks - _partition_header.free_blocks_count;
    if (_partition_header.free_blocks_count < (1 + data_blocks_count)) {
        delete file_header;
        return AlopexOS::errorCode::NoSpaceLeft;
    }

    _partition_header.free_blocks_count -= (1 + data_blocks_count);
    file_header->payload_offset = assigned_header_block + 1;

    if (!write_block(assigned_header_block, file_header)) {
        delete file_header;
        return AlopexOS::errorCode::WriteFailed;
    }

    u64 payload_offset = file_header->payload_offset;
    delete file_header;

    dynarr<byte> block_buffer;
    block_buffer.resize(ABTRFS_HEADER_BLOCK_SIZE);

    for (u64 b = 0; b < data_blocks_count; b++) {
        for (u64 k = 0; k < ABTRFS_HEADER_BLOCK_SIZE; k++) {
            block_buffer[k] = 0;
        }

        u64 chunk_offset = b * ABTRFS_HEADER_BLOCK_SIZE;
        for (u64 k = 0; k < ABTRFS_HEADER_BLOCK_SIZE && (chunk_offset + k) < data.size(); k++) {
            block_buffer[k] = data[chunk_offset + k];
        }

        if (!write_block(payload_offset + b, block_buffer.data())) {
            return AlopexOS::errorCode::WriteFailed;
        }
    }

    if (!write_block(0, &_partition_header)) {
        return AlopexOS::errorCode::WriteFailed;
    }

    return AlopexOS::errorCode::Success;
}

static fn is_path_equal(const AlopexOS::Path& path, const char* header_path) -> bool {
    for (int i = 0; i < 255 && i < 2048; i++) {
        char c1 = static_cast<char>(path[i]);
        char c2 = header_path[i];
        if (c1 != c2) return false;
        if (c1 == '\0') return true;
    }
    return false;
}

static fn find_file_header(AlopexOS::AbtrFS* fs, u64 total_blocks, u64 free_blocks, const AlopexOS::Path& path, AbtrFSDiskHeader& out_header) -> bool {
    u64 used_blocks = total_blocks - free_blocks;
    u64 curr_block = 1;

    AbtrFSDiskHeader* header = new AbtrFSDiskHeader{};

    while (curr_block < used_blocks) {
        if (!fs->read_block(curr_block, header)) {
            delete header;
            return false;
        }

        bool is_file = (header->base.magic == 0xABF5) && (header->base.flags & AbtrfsDiskFlags::IsFile);
        if (is_file && is_path_equal(path, header->path)) {
            out_header = *header;
            delete header;
            return true;
        }

        u64 skip = (header->base.magic == 0xABF5 && header->section_count > 0) ? (1 + header->section_count) : 1;
        curr_block += skip;
    }

    delete header;
    return false;
}

fn AlopexOS::AbtrFS::exists(const AlopexOS::Path& path) -> AlopexOS::errorCode {
    if (!_is_mounted) return AlopexOS::errorCode::DeviceNotMounted;

    auto choppedPath = chopPath(path);
    if (!choppedPath.has_value()) return choppedPath.error();

    const auto& segments = choppedPath.value();
    if (segments.size() == 0) return AlopexOS::errorCode::InvalidPath;

    AbtrFSDiskHeader* header = new AbtrFSDiskHeader{};
    bool found = find_file_header(this, _partition_header.total_blocks, _partition_header.free_blocks_count, path, *header);
    delete header;
    if (!found) return AlopexOS::errorCode::FileNotFound;

    return AlopexOS::errorCode::Success;
}

fn AlopexOS::AbtrFS::read_file(const AlopexOS::Path& path, dynarr<byte>* data) -> AurenFox::core::Expected<dynarr<byte>, AlopexOS::errorCode> {
    if (!_is_mounted) return AlopexOS::errorCode::DeviceNotMounted;

    auto choppedPath = chopPath(path);
    if (!choppedPath.has_value()) return choppedPath.error();

    const auto& segments = choppedPath.value();
    if (segments.size() == 0) return AlopexOS::errorCode::InvalidPath;

    AbtrFSDiskHeader* file_header = new AbtrFSDiskHeader{};
    bool found = find_file_header(this, _partition_header.total_blocks, _partition_header.free_blocks_count, path, *file_header);
    if (!found) {
        delete file_header;
        return AlopexOS::errorCode::FileNotFound;
    }

    u64 data_blocks_count = file_header->section_count;
    u64 total_size = 0;
    if (data_blocks_count > 0) {
        total_size = (data_blocks_count - 1) * ABTRFS_HEADER_BLOCK_SIZE + file_header->size;
    }
    u64 payload_offset = file_header->payload_offset;
    
    serial_print_abtr("[ABTRFS] read_file: section_count=");
    char num_buf[32];
    int npos = 0;
    u64 tmp = data_blocks_count;
    if (tmp == 0) serial_print_abtr("0");
    while (tmp > 0) { num_buf[npos++] = '0' + (tmp % 10); tmp /= 10; }
    for (int i = npos - 1; i >= 0; i--) serial_out(0x3F8, num_buf[i]);
    serial_print_abtr(", size=");
    npos = 0; tmp = file_header->size;
    if (tmp == 0) serial_print_abtr("0");
    while (tmp > 0) { num_buf[npos++] = '0' + (tmp % 10); tmp /= 10; }
    for (int i = npos - 1; i >= 0; i--) serial_out(0x3F8, num_buf[i]);
    serial_print_abtr(", payload_offset=");
    npos = 0; tmp = payload_offset;
    if (tmp == 0) serial_print_abtr("0");
    while (tmp > 0) { num_buf[npos++] = '0' + (tmp % 10); tmp /= 10; }
    for (int i = npos - 1; i >= 0; i--) serial_out(0x3F8, num_buf[i]);
    serial_print_abtr("\n");

    delete file_header;

    dynarr<byte> local_buffer;
    dynarr<byte>* target_data = data;
    if (!target_data) {
        target_data = &local_buffer;
    }
    target_data->resize(total_size);

    dynarr<byte> block_buffer;
    block_buffer.resize(ABTRFS_HEADER_BLOCK_SIZE);

    u64 bytes_read = 0;
    for (u64 b = 0; b < data_blocks_count; b++) {
        u64 block_num = payload_offset + b;
        if (!read_block(block_num, block_buffer.data())) {
            return AlopexOS::errorCode::IOError;
        }

        u64 to_copy = ABTRFS_HEADER_BLOCK_SIZE;
        if (bytes_read + to_copy > total_size) {
            to_copy = total_size - bytes_read;
        }

        for (u64 k = 0; k < to_copy; k++) {
            (*target_data)[bytes_read + k] = block_buffer[k];
        }

        bytes_read += to_copy;
    }

    return AurenFox::core::Expected<dynarr<byte>, AlopexOS::errorCode>(*target_data);
}