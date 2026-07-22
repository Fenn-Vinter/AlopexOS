#if !defined(FORMAT_HPP)
#define FORMAT_HPP

#include <primitives.h>

#if defined(_MSC_VER)
    #define PACKED __pragma(pack(push, 1))
    #define PACKED_END ; __pragma(pack(pop))
#elif defined(__GNUC__) || defined(__clang__)
    #define PACKED
    #define PACKED_END __attribute__((packed));
#endif

constexpr u64 ABTRFS_HEADER_BLOCK_SIZE = 4096;

enum class AbtrfsDiskFlags : u32 {
    None = 0,
    Encrypted = 1 << 0,
    Journaled = 1 << 1,
    ReadOnly = 1 << 2,
    IsFile = 1 << 3
};

constexpr AbtrfsDiskFlags operator|(AbtrfsDiskFlags a, AbtrfsDiskFlags b) {
    return static_cast<AbtrfsDiskFlags>(
        static_cast<u32>(a) | static_cast<u32>(b)
    );
}

constexpr bool operator&(AbtrfsDiskFlags a, AbtrfsDiskFlags b) {
    return (static_cast<u32>(a) & static_cast<u32>(b)) != 0;
}

PACKED
struct AbtrFSBaseHeader {
    u16 magic = 0xABF5;
    AbtrfsDiskFlags flags{};
    u32 checksum{};
    u64 transaction_sequence{};
} PACKED_END;

PACKED
struct AbtrFSJournalEntry {
    u64 transaction_id{};
    u32 checksum{};
    u32 data_length{};
} PACKED_END;

PACKED
struct AbtrFSAccessControl {
    u32 uid{};
    u32 gid{};
    u16 permissions{};
} PACKED_END;

PACKED
struct AbtrFSDiskHeader {
    AbtrFSBaseHeader base{};
    AbtrFSAccessControl acl{};
    char password_hash[64]{};
    u64 section_count{};
    u64 creation_time{};
    u64 modification_time{};
    u64 access_time{};
    char path[256]{};
    char icon[256]{};
    char owner[64]{};
    char group[64]{};
    u64 size{};
    u64 block_size{ABTRFS_HEADER_BLOCK_SIZE};
    u64 payload_offset{};
    char reserved[ABTRFS_HEADER_BLOCK_SIZE - sizeof(AbtrFSBaseHeader) - 770]{};
} PACKED_END;

static_assert(sizeof(AbtrFSDiskHeader) == ABTRFS_HEADER_BLOCK_SIZE, "AbtrFSDiskHeader must exactly match one 4096-byte block");

#undef PACKED
#undef PACKED_END

#endif