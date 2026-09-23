#pragma once

#include <fennlib/sequence>
#include <fennlib/string>

namespace AlopexOS {
    using path = fennlib::string1024;
    class FILE;
    struct vfs_packet;
    class vfs;
}

class AlopexOS::vfs {
    
public:
    vfs()  = default;
    ~vfs() = default;

    auto createFile(path file_path) -> fennlib::uint;
    auto createFolder(path folder_path) -> fennlib::uint;
    auto exists(path directory) -> bool;

};
