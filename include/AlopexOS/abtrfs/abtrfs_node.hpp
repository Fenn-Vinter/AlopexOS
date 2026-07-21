#ifndef ABTR_FS_NODE_HPP
#define ABTR_FS_NODE_HPP

#include <primetives.h>
#include "format.hpp"

#if defined(_MSC_VER)
    #define PACKED __pragma(pack(push, 1))
    #define PACKED_END ; __pragma(pack(pop))
#elif defined(__GNUC__) || defined(__clang__)
    #define PACKED
    #define PACKED_END __attribute__((packed));
#endif

namespace AlopexOS {

    enum class AbtrFSNodeType : u8 {
        Internal = 0x01,
        Leaf     = 0x02
    };

    PACKED
    struct AbtrFSBTreeEntry {
        u64 key_hash{};
        u64 pointer{};
    } PACKED_END;

    PACKED
    struct AbtrFSNode {
        AbtrFSBaseHeader base{};
        AbtrFSNodeType node_type{};
        u16 key_count{};
        u64 self_block_addr{};
        
        static constexpr u64 NodeHeaderOverhead = sizeof(AbtrFSBaseHeader) + sizeof(AbtrFSNodeType) + sizeof(u16) + sizeof(u64);
        static constexpr u64 MaxEntries = (ABTRFS_HEADER_BLOCK_SIZE - NodeHeaderOverhead) / sizeof(AbtrFSBTreeEntry);
        
        AbtrFSBTreeEntry entries[MaxEntries]{};
        
        char padding[ABTRFS_HEADER_BLOCK_SIZE - NodeHeaderOverhead - (MaxEntries * sizeof(AbtrFSBTreeEntry))]{};
    } PACKED_END;

    static_assert(sizeof(AbtrFSNode) == ABTRFS_HEADER_BLOCK_SIZE, "AbtrFSNode must exactly match one 4096-byte block");

}

#undef PACKED
#undef PACKED_END

#endif