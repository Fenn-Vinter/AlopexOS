#pragma once
#include <fennlib/types>
#include <fennlib/string>

namespace AlopexOS::types {
    using DeviceID = fennlib::string;
    using VRAMID = fennlib::types::usize;
    using PID = fennlib::types::u32;
}

namespace AlopexOS {
    using namespace types;
}