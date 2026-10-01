#pragma once

#include <fennlib/types>
#include <AlopexOS/errorCodes.hpp>
#include <AlopexOS/types.hpp>

namespace AlopexOS {
    enum class FilesystemType {
        Unknown,
        Btrfs
    };

    class i_filesystem {
    protected:
        FilesystemType m_type{FilesystemType::Unknown};
        bool m_mounted{false};

    public:
        i_filesystem(FilesystemType type) : m_type(type) {}
        ~i_filesystem() = default;

        inline auto get_type() const -> FilesystemType { return m_type; }
        inline auto is_mounted() const -> bool { return m_mounted; }
    };
}