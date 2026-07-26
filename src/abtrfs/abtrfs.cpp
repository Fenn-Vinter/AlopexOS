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

    if (!read_block(0, &_partition_header)) {
        _is_mounted = false;
        return false;
    }

    if (_partition_header.magic != 0xABF4) {
        _is_mounted = false;
        return false;
    }

    _is_mounted = true;
    return true;
}

fn AlopexOS::AbtrFS::mount_existing(uptr base_address, u64 hhdm_offset, Handle device_handle) -> void {
    _device_base_address = base_address;
    _hhdm_offset = hhdm_offset;
    _device_handle = device_handle;
    
    if (read_block(0, &_partition_header)) {
        if (_partition_header.magic == 0xABF4) {
            _is_mounted = true;
        }
    }
}

fn AlopexOS::AbtrFS::read_block(u64 block_address, void* buffer) -> bool {
    if (_device_handle == InvalidHandle) {
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
    if (_device_handle == InvalidHandle) {
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

fn AlopexOS::AbtrFS::format(const uptr& base_address, const u64& hhdm_offset, const Handle& device_handle) -> bool {
    _device_base_address = base_address;
    _hhdm_offset = hhdm_offset;
    _device_handle = device_handle;

    PARTITION_HEADER header{};
    header.magic = 0xABF4;
    header.total_blocks = 204800; // Default partition size bounds
    header.free_blocks_count = header.total_blocks - 1;
    header.block_size = ABTRFS_HEADER_BLOCK_SIZE;

    _partition_header = header;

    if (!write_block(0, &header)) {
        return false;
    }

    _is_mounted = true;
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

fn AlopexOS::AbtrFS::write_file(const AlopexOS::Path& path, const dynarr<byte>& data, u64 write_offset) -> AlopexOS::errorCode {
    if (!_is_mounted) return AlopexOS::errorCode::DeviceNotMounted;

    auto choppedPath = chopPath(path);
    if (!choppedPath.has_value()) return choppedPath.error();

    const auto& segments = choppedPath.value();
    if (segments.size() == 0) return AlopexOS::errorCode::InvalidPath;

    auto target_node = find_or_create_node(_root_node, segments, 0);
    if (!target_node.has_value()) {
        return AlopexOS::errorCode::AllocationFailed;
    }

    AbtrFSDiskHeader file_header{};
    bool file_exists = find_file_header(this, _partition_header.total_blocks, _partition_header.free_blocks_count, path, file_header);

    u64 existing_total_size = 0;
    if (file_exists) {
        if (file_header.section_count > 0) {
            existing_total_size = (file_header.section_count - 1) * ABTRFS_HEADER_BLOCK_SIZE + file_header.size;
        }
    }

    u64 target_end_size = write_offset + data.size();
    u64 final_size = (target_end_size > existing_total_size) ? target_end_size : existing_total_size;
    if (final_size == 0) return AlopexOS::errorCode::InvalidParameters;

    u64 required_blocks = (final_size + ABTRFS_HEADER_BLOCK_SIZE - 1) / ABTRFS_HEADER_BLOCK_SIZE;

    dynarr<byte> expanded_data{};
    expanded_data.resize(final_size);

    if (file_exists) {
        auto old_content = read_file(path, nullptr, 0, 0);
        if (old_content.has_value()) {
            const auto& old_bytes = old_content.value();
            for (u64 i = 0; i < old_bytes.size() && i < final_size; i++) {
                expanded_data[i] = old_bytes[i];
            }
        }
    }

    for (u64 i = 0; i < data.size(); i++) {
        expanded_data[write_offset + i] = data[i];
    }

    u64 data_blocks_count = required_blocks;
    u64 assigned_header_block = _partition_header.total_blocks - _partition_header.free_blocks_count;

    if (!file_exists) {
        if (_partition_header.free_blocks_count < (1 + data_blocks_count) || 
            (assigned_header_block + 1 + data_blocks_count) > _partition_header.total_blocks) {
            return AlopexOS::errorCode::NoSpaceLeft;
        }
        _partition_header.free_blocks_count -= (1 + data_blocks_count);
        file_header.payload_offset = assigned_header_block + 1;
    } else {
        if (data_blocks_count > file_header.section_count) {
            u64 extra_blocks_needed = data_blocks_count - file_header.section_count;
            if (_partition_header.free_blocks_count < extra_blocks_needed) {
                return AlopexOS::errorCode::NoSpaceLeft;
            }
            _partition_header.free_blocks_count -= extra_blocks_needed;
        }
    }

    file_header.base.magic = 0xABF5;
    file_header.base.flags = AbtrfsDiskFlags::IsFile;
    file_header.section_count = data_blocks_count;
    
    u64 remainder = final_size % ABTRFS_HEADER_BLOCK_SIZE;
    file_header.size = static_cast<u16>(remainder == 0 ? ABTRFS_HEADER_BLOCK_SIZE : remainder);
    file_header.block_size = ABTRFS_HEADER_BLOCK_SIZE;

    int p_idx = 0;
    for (int i = 0; i < 2048 && path[i] != '\0' && p_idx < 255; i++) {
        file_header.path[p_idx++] = static_cast<char>(path[i]);
    }
    file_header.path[p_idx] = '\0';

    if (!file_exists) {
        if (!write_block(assigned_header_block, &file_header)) {
            return AlopexOS::errorCode::WriteFailed;
        }
    }

    u64 payload_offset = file_header.payload_offset;
    byte block_buffer[ABTRFS_HEADER_BLOCK_SIZE];
    const byte* raw_data_ptr = expanded_data.data();

    for (u64 b = 0; b < data_blocks_count; b++) {
        for (u64 k = 0; k < ABTRFS_HEADER_BLOCK_SIZE; k++) {
            block_buffer[k] = 0;
        }

        u64 chunk_offset = b * ABTRFS_HEADER_BLOCK_SIZE;
        for (u64 k = 0; k < ABTRFS_HEADER_BLOCK_SIZE && (chunk_offset + k) < final_size; k++) {
            block_buffer[k] = raw_data_ptr[chunk_offset + k];
        }

        if (!write_block(payload_offset + b, block_buffer)) {
            return AlopexOS::errorCode::WriteFailed;
        }
    }

    if (!write_block(0, &_partition_header)) {
        return AlopexOS::errorCode::WriteFailed;
    }

    return AlopexOS::errorCode::Success;
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

fn AlopexOS::AbtrFS::read_file(const AlopexOS::Path& path, dynarr<byte>* data, u64 read_offset, u64 max_read_size) -> AurenFox::core::Expected<dynarr<byte>, AlopexOS::errorCode> {
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
    delete file_header;

    if (read_offset >= total_size) {
        dynarr<byte> empty_result{};
        if (data) *data = empty_result;
        return AurenFox::core::Expected<dynarr<byte>, AlopexOS::errorCode>(empty_result);
    }

    u64 available_size = total_size - read_offset;
    u64 bytes_to_read = available_size;
    
    if (max_read_size > 0 && max_read_size < available_size) {
        bytes_to_read = max_read_size;
    }

    dynarr<byte> local_buffer;
    dynarr<byte>* target_data = data;
    if (!target_data) {
        target_data = &local_buffer;
    }
    target_data->resize(bytes_to_read);

    dynarr<byte> block_buffer;
    block_buffer.resize(ABTRFS_HEADER_BLOCK_SIZE);

    u64 start_block = read_offset / ABTRFS_HEADER_BLOCK_SIZE;
    u64 block_offset_shift = read_offset % ABTRFS_HEADER_BLOCK_SIZE;

    u64 bytes_copied = 0;
    for (u64 b = start_block; b < data_blocks_count && bytes_copied < bytes_to_read; b++) {
        u64 block_num = payload_offset + b;
        if (!read_block(block_num, block_buffer.data())) {
            return AlopexOS::errorCode::IOError;
        }

        u64 k_start = (b == start_block) ? block_offset_shift : 0;
        for (u64 k = k_start; k < ABTRFS_HEADER_BLOCK_SIZE && bytes_copied < bytes_to_read; k++) {
            (*target_data)[bytes_copied++] = block_buffer[k];
        }
    }

    return AurenFox::core::Expected<dynarr<byte>, AlopexOS::errorCode>(*target_data);
}