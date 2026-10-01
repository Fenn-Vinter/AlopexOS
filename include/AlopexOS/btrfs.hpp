#pragma once

#include "AlopexOS/types.hpp"
#include "logger.hpp"
#include <fennlib/types>
#include <fennlib/string>
#include <AlopexOS/errorCodes.hpp>
#include <synapse.hpp>
#include <AlopexOS/filesystem.hpp>

namespace btrfs {
    constexpr fennlib::types::u64 BTRFS_MAGIC = 0x4D5F21656E6F7242ULL;
    constexpr fennlib::types::usize BTRFS_SUPER_INFO_OFFSET = 0x10000;
    constexpr fennlib::types::usize BTRFS_SUPER_INFO_SIZE = 4096;

    struct Superblock {
        unsigned char fsid[16];
        fennlib::types::u64 bytenr;
        fennlib::types::u64 flags;
        char magic[8];
        fennlib::types::u64 generation;
        fennlib::types::u64 root;
        fennlib::types::u64 chunk_root;
        fennlib::types::u64 log_root;
    };

    class FileSystem : public AlopexOS::i_filesystem {
        Superblock m_superblock{};

    public:
        FileSystem() : AlopexOS::i_filesystem(AlopexOS::FilesystemType::Btrfs) {}
        ~FileSystem() = default;

        template<bool Debug = false>
        inline auto reformat(io::Synapse* synapse, AlopexOS::UID uid, io::Logger* logger = nullptr) -> AlopexOS::error_code {
            auto find_desc = [synapse, uid]() -> io::DeviceDescriptor* {
                auto& registry = const_cast<fennlib::sequence<io::DeviceDescriptor>&>(synapse->get_registry());
                for (fennlib::types::u64 i = 0; i < registry.size(); ++i) {
                    if (registry[i].uid == uid) {
                        return &registry[i];
                    }
                }
                return nullptr;
            };

            io::DeviceDescriptor* desc = find_desc();
            if (!desc) {
                synapse->scan_hardware<Debug>(logger);
                desc = find_desc();
                if (!desc) {
                    return AlopexOS::error_code::device_not_found;
                }
            }

            if (!desc->active || desc->mmio_base == 0) {
                return AlopexOS::error_code::device_unresponsive;
            }

            Superblock sb{};
            __builtin_memset(&sb, 0, sizeof(Superblock));

            __builtin_memcpy(sb.magic, &BTRFS_MAGIC, sizeof(BTRFS_MAGIC));
            sb.bytenr = BTRFS_SUPER_INFO_OFFSET;
            sb.generation = 1;

            volatile Superblock* dest = reinterpret_cast<volatile Superblock*>(desc->mmio_base + BTRFS_SUPER_INFO_OFFSET);
            __builtin_memcpy(const_cast<Superblock*>(dest), &sb, sizeof(Superblock));

            if constexpr (Debug) {
                if (logger) {
                    logger->log(io::LogLevel::Info, "BTRFS: Successfully wrote fresh superblock during reformat.");
                }
            }

            return AlopexOS::error_code::Success;
        }

        template<bool Debug = false>
        inline auto mount(io::Synapse* synapse, AlopexOS::UID uid, io::Logger* logger = nullptr) -> AlopexOS::error_code {
            auto find_desc = [synapse, uid]() -> io::DeviceDescriptor* {
                auto& registry = const_cast<fennlib::sequence<io::DeviceDescriptor>&>(synapse->get_registry());
                for (fennlib::types::u64 i = 0; i < registry.size(); ++i) {
                    if (registry[i].uid == uid) {
                        return &registry[i];
                    }
                }
                return nullptr;
            };

            io::DeviceDescriptor* desc = find_desc();
            if (!desc) {
                synapse->scan_hardware<Debug>(logger);
                desc = find_desc();
                if (!desc) {
                    return AlopexOS::error_code::device_not_found;
                }
            }

            if (!desc->active || desc->mmio_base == 0) {
                return AlopexOS::error_code::device_unresponsive;
            }

            volatile Superblock* src = reinterpret_cast<volatile Superblock*>(desc->mmio_base + BTRFS_SUPER_INFO_OFFSET);
            
            Superblock sb{};
            __builtin_memcpy(&sb, const_cast<const Superblock*>(src), sizeof(Superblock));

            fennlib::types::u64 read_magic = 0;
            __builtin_memcpy(&read_magic, sb.magic, sizeof(read_magic));
            if (read_magic != BTRFS_MAGIC) {
                if constexpr (Debug) {
                    if (logger) {
                        logger->log(io::LogLevel::Warning, "BTRFS: Mount failed - invalid magic number found at superblock offset.");
                    }
                }
                return AlopexOS::error_code::invalid_filesystem;
            }
            
            m_superblock = sb;
            m_mounted = true;

            if constexpr (Debug) {
                if (logger) {
                    logger->log(io::LogLevel::Info, "BTRFS: Filesystem successfully mounted.");
                }
            }

            return AlopexOS::error_code::Success;
        }

        auto is_mounted() const -> bool {
            return m_mounted;
        }

        auto get_superblock() const -> const Superblock& {
            return m_superblock;
        }
    };
}