#pragma once

#include <logger.hpp>
#include <synapse.hpp>
#include <fennlib/types>
#include <fennlib/sequence>
#include <AlopexOS/errorCodes.hpp>
#include <fennlib/string>
#include <fennlib/new>
#include <AlopexOS/types.hpp>
#include <AlopexOS/btrfs.hpp>
#include <AlopexOS/filesystem.hpp>

namespace avfs {
    enum class Status {
        error,
        processing,
        success,
    };
    
    class FileBuffer {
        friend class Interface;
        AlopexOS::Ptr m_data_ptr{fennlib::types::nil};
        fennlib::types::usize m_size{0};
        Status m_status{Status::processing};

    public:
        auto data_ptr() const -> AlopexOS::Ptr { return m_data_ptr; }
        auto mutable_data_ptr() -> AlopexOS::Ptr { return m_data_ptr; }
        auto size() const -> fennlib::types::usize { return m_size; }
        auto status() const -> Status { return m_status; }
    };

    class Interface {
        struct DriveAlias {
            AlopexOS::types::UID uid{fennlib::types::nil};
            fennlib::sequence<fennlib::string16> aliases{};
        };

        fennlib::sequence<DriveAlias> p_drive_aliases{};
        fennlib::sequence<AlopexOS::i_filesystem*> p_mounted_filesystems{};
        io::Synapse* p_synapse = fennlib::types::null;

    public:
        Interface(io::Synapse* synapse) : p_synapse(synapse) {}
        
        ~Interface() {
            for (fennlib::types::u64 i = 0; i < p_mounted_filesystems.size(); ++i) {
                auto* fs = p_mounted_filesystems[i];
                if (!fs) continue;

                switch (fs->get_type()) {
                    case AlopexOS::FilesystemType::Btrfs:
                        delete static_cast<btrfs::FileSystem*>(fs);
                        break;
                    default:
                        delete fs;
                        break;
                }
            }
        }

        auto step() -> AlopexOS::error_code;

        template<bool Debug = false>
        inline auto mount(fennlib::string16 DriveAlias, io::Logger* logger) -> AlopexOS::error_code {
            // TODO: Resolve alias to UID and call UID mount
            (void)DriveAlias;
            (void)logger;

            return AlopexOS::error_code::device_not_found;
        }

        template<bool Debug = false>
        inline auto mount(AlopexOS::types::UID uid, io::Logger* logger = fennlib::types::null) -> AlopexOS::error_code {
            auto* btrfs_filesystem = new btrfs::FileSystem();

            AlopexOS::error_code err = btrfs_filesystem->template mount<Debug>(p_synapse, uid, logger);
            if (err != AlopexOS::error_code::Success) {
                delete btrfs_filesystem;
                return err;
            }

            p_mounted_filesystems.push_back(btrfs_filesystem);
            return AlopexOS::error_code::Success;
        }

        auto create_folder(AlopexOS::Path path) -> AlopexOS::error_code { 
            return create_file(path, fennlib::types::nil, fennlib::types::nil); 
        }
        
        auto create_file(AlopexOS::Path path, AlopexOS::Ptr data_start_address, AlopexOS::Ptr data_end_address) -> AlopexOS::error_code;
        auto write_file(AlopexOS::Path path, AlopexOS::Ptr data_start_address, AlopexOS::Ptr data_end_address) -> AlopexOS::error_code;
        auto resize_file(AlopexOS::Path path, fennlib::types::usize new_size) -> AlopexOS::error_code;
        auto splice_file(AlopexOS::Path path, fennlib::types::u64 offset, fennlib::types::usize remove_bytes, 
                         AlopexOS::Ptr data_start_address, AlopexOS::Ptr data_end_address) -> AlopexOS::error_code;

        auto get_file(fennlib::string64 path) -> FileBuffer*;
        auto read_file(fennlib::string64 path) -> FileBuffer* { return get_file(path); }
    };

    inline Interface* g_interface = nullptr;

    inline auto init(io::Synapse* synapse) -> void {
        alignas(Interface) static unsigned char interface_storage[sizeof(Interface)];
        g_interface = new (static_cast<void*>(interface_storage)) Interface(synapse);
    }

    inline auto Interface::step() -> AlopexOS::error_code {
        return AlopexOS::error_code::Success;
    }
}