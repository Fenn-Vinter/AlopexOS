#if !defined(ABTRFS_HPP)
#define ABTRFS_HPP

#include <primitives.h>
#include "partition_header.hpp"
#include <AlopexOS/AlopexOS.hpp>

namespace AlopexOS {
    class AbtrFS;
}

class AlopexOS::AbtrFS {
public:
    AbtrFS() = default;
    ~AbtrFS() = default;

    AbtrFS(const AbtrFS&) = delete;
    auto operator=(const AbtrFS&) -> AbtrFS& = delete;
    AbtrFS(AbtrFS&&) = default;
    auto operator=(AbtrFS&&) -> AbtrFS& = default;

    auto mount(uptr base_address, u64 hhdm_offset, Handle device_handle) -> bool;
    auto format(uptr base_address, u64 hhdm_offset, Handle device_handle) -> bool;
    auto read_block(u64 block_address, void* buffer) -> bool;
    auto write_block(u64 block_address, const void* buffer) -> bool;

    auto is_mounted() const -> bool { return _is_mounted; }

private:
    bool _is_mounted{false};
    PARTITION_HEADER _partition_header{};
    u64 _device_base_address{0};
    u64 _hhdm_offset{0};
    Handle _device_handle{InvalidHandle};
};

#endif