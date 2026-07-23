#if !defined(ABTRFS_HPP)
#define ABTRFS_HPP

#include "AlopexOS/AlopexOS_ErrorCodes.hpp"
#include "arr.hpp"
#include <primitives.h>
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

    AbtrFS(const AbtrFS&) = delete;
    fn operator=(const AbtrFS&) -> AbtrFS& = delete;
    AbtrFS(AbtrFS&&) = default;
    fn operator=(AbtrFS&&) -> AbtrFS& = default;

    fn mount(uptr base_address, u64 hhdm_offset, Handle device_handle) -> bool;
    fn format(uptr base_address, u64 hhdm_offset, Handle device_handle) -> bool;
    fn read_block(u64 block_address, void* buffer) -> bool;
    fn write_block(u64 block_address, const void* buffer) -> bool;
    
    fn is_mounted() const -> bool { return _is_mounted; }

    fn write_file(AlopexOS::Path path, dynarr<byte> data) -> AlopexOS::errorCode;
    fn read_file(AlopexOS::Path path, dynarr<byte>* data = nullptr) -> AurenFox::core::Expected<dynarr<byte>, AlopexOS::errorCode>;
    fn exists(AlopexOS::Path path) -> AlopexOS::errorCode;
    
private:
    bool _is_mounted{false};
    PARTITION_HEADER _partition_header{};
    u64 _device_base_address{0};
    u64 _hhdm_offset{0};
    Handle _device_handle{InvalidHandle};
    TreeNode* _root_node{nullptr};
};

#endif