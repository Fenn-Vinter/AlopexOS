#pragma once
#include <fennlib/types>
#include <fennlib/string>

namespace AlopexOS::types {
    using DeviceID = fennlib::string;
    using UID = fennlib::types::u64;
    using VRAMID = fennlib::types::usize;
    using PID = fennlib::types::u32;
    using Path = fennlib::string64;
    using Ptr = fennlib::types::uintptr;
}

namespace AlopexOS {
    using namespace types;
}