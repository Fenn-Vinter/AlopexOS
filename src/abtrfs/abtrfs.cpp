#include <AlopexOS/abtrfs/abtrfs.hpp>

namespace AlopexOS {

    auto AbtrFS::initialize(DiskDevice* bootDevice) -> AbtrFS& {
        if (!instance) {
            static AbtrFS instance_ref(bootDevice);
            instance = &instance_ref;
        }
        return *instance;
    }

    auto AbtrFS::get_instance() -> AbtrFS& {
        return *instance;
    }

    AbtrFS::AbtrFS(DiskDevice* bootDevice)
        : m_main_drive(bootDevice), m_superblock{}, m_cached_root_node(nullptr) {}

    auto AbtrFS::getRoot() -> AbtrFSNode* {
        return m_cached_root_node;
    }

    auto AbtrFS::mount() -> bool {
        if (!m_main_drive) {
            return false;
        }

        u8 superblock_buffer[sizeof(PARTITION_HEADER)];
        
        // Assuming 512-byte hardware sectors; 4096 bytes = 8 sectors per block.
        constexpr u64 sectors_per_block = 8;
        
        // Read partition superblock from block 0 (sector 0)
        if (!m_main_drive->read_blocks(0, sectors_per_block, superblock_buffer)) {
            return false;
        }

        __builtin_memcpy(&m_superblock, superblock_buffer, sizeof(PARTITION_HEADER));

        // Validate the AbtrFS partition magic signature
        if (m_superblock.magic != ABTRFS_MAGIC_64) {
            return false;
        }

        // Static or otherwise persistent block buffer for the cached root node
        static u8 root_block_buffer[ABTRFS_HEADER_BLOCK_SIZE];
        
        u64 root_sector = m_superblock.root_node_block * sectors_per_block;
        if (!m_main_drive->read_blocks(root_sector, sectors_per_block, root_block_buffer)) {
            return false;
        }

        m_cached_root_node = reinterpret_cast<AbtrFSNode*>(root_block_buffer);

        // Verify the root node block base header magic
        if (m_cached_root_node->base.magic != 0xABF5) {
            return false;
        }

        return true;
    }

}