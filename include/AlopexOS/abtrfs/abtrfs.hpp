#include "AlopexOS/gaossd/gaossd.hpp"
#if !defined(ABTRFS_HPP)
#define ABTRFS_HPP

#include "AlopexOS/AlopexOS_ErrorCodes.hpp"
#include "arr.hpp"
#include <primitives.hpp>
#include "partition_header.hpp"
#include <AlopexOS/AlopexOS.hpp>
#include <expected.hpp>
#include <string.hpp>

namespace AlopexOS {
    class AbtrFS;
}

struct TreeNode {
    AlopexOS::string name;
    bool is_directory;
    TreeNode* left;
    TreeNode* right;
};

class AlopexOS::AbtrFS {
public:
    AbtrFS() = default;
    ~AbtrFS() = default;

    fn mount(uptr base_address, u64 hhdm_offset, Handle device_handle) -> bool;
    fn format(const uptr& base_address, const u64& hhdm_offset, const Handle& device_handle) -> bool;
    fn read_block(u64 block_address, void* buffer) -> bool;
    fn write_block(u64 block_address, const void* buffer) -> bool;
    
    fn is_mounted() const -> bool { return _is_mounted; }

    fn write_file(const AlopexOS::Path& path, const dynarr<byte>& data, u64 write_offset = 0) -> AlopexOS::errorCode;
    fn read_file(const AlopexOS::Path& path, dynarr<byte>* data = nullptr, u64 read_offset = 0, u64 max_read_size = 0) -> AurenFox::core::Expected<dynarr<byte>, AlopexOS::errorCode>;
    
    fn exists(const AlopexOS::Path& path) -> AlopexOS::errorCode;
    
    fn PartitionHeader_Get() const -> const PARTITION_HEADER& { return _partition_header; }
    fn DeviceBaseAddress_Get() const -> u64 { return _device_base_address; }
    fn HhdmOffset_Get() const -> u64 { return _hhdm_offset; }
    fn DeviceHandle_Get() const -> Handle { return _device_handle; }
    fn RootNode_Get() const -> const TreeNode* { return _root_node; }

    fn mount_existing(uptr base_address, u64 hhdm_offset, Handle device_handle) -> void;

    fn attach_gaossd(gaossd* Gaossd) -> errorCode;
    fn gaossd_instance() -> gaossd*;
private:
    bool _is_mounted{false};
    PARTITION_HEADER _partition_header{};
    u64 _device_base_address{0};
    u64 _hhdm_offset{0};
    Handle _device_handle{InvalidHandle};
    TreeNode* _root_node{nullptr};
    gaossd* _gaossd{nullptr};
};

#endif