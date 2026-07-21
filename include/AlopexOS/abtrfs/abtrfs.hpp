#if !defined(ABTRFS_HPP)
#define ABTRFS_HPP

#include <primetives.h>
#include <arr.hpp>
#include <string.hpp>
#include "disk_device.hpp"
#include "partition_header.hpp"
#include "abtrfs_node.hpp"

#if defined(_MSC_VER)
    #define PACKED __pragma(pack(push, 1))
    #define PACKED_END ; __pragma(pack(pop))
#elif defined(__GNUC__) || defined(__clang__)
    #define PACKED
    #define PACKED_END __attribute__((packed));
#endif

namespace AlopexOS {
    class AbtrFS;
}

class AlopexOS::AbtrFS {
    public:
        static auto initialize(DiskDevice* bootDevice) -> AbtrFS&;
        static auto get_instance() -> AbtrFS&;

        auto getRoot() -> AbtrFSNode*;
        auto mount() -> bool;

    private:
        AbtrFS(DiskDevice* bootDevice);
        inline static AbtrFS* instance = nullptr;

        DiskDevice* m_main_drive;
        PARTITION_HEADER m_superblock;
        AbtrFSNode* m_cached_root_node;
};

#endif