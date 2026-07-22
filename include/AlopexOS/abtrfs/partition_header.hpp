#include <primitives.h>

constexpr u64 ABTRFS_PARTITION_HEADER_BLOCK_SIZE = 4096;

constexpr u64 ABTRFS_MAGIC_64 = 
    (static_cast<u64>('A') << 40) |
    (static_cast<u64>('b') << 32) |
    (static_cast<u64>('t') << 24) |
    (static_cast<u64>('r') << 16) |
    (static_cast<u64>('F') << 8)  |
    static_cast<u64>('S');

#if defined(_MSC_VER)
    #define PACKED __pragma(pack(push, 1))
    #define PACKED_END ; __pragma(pack(pop))
#elif defined(__GNUC__) || defined(__clang__)
    #define PACKED
    #define PACKED_END __attribute__((packed));
#endif

PACKED
struct PARTITION_HEADER {
    u64 magic{ABTRFS_MAGIC_64};
    u64 block_size{4096};
    u64 total_blocks{};
    u64 root_node_block{};
    u64 free_blocks_count{};
    u32 version{1};
    char volume_label[32]{};
    u8 padding[4020];
} PACKED_END;

static_assert(sizeof(PARTITION_HEADER) == ABTRFS_PARTITION_HEADER_BLOCK_SIZE, "PARTITION_HEADER must exactly match one 4096-byte block");

#undef PACKED
#undef PACKED_END