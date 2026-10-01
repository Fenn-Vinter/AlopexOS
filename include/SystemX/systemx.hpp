#pragma once
#if !defined(_SYSTEM_X_HPP_)
#define _SYSTEM_X_HPP_

#include <fennlib/types>
#include <fennlib/string>
#include <kernel/DRAMA.hpp>
#include <AlopexOS/types.hpp>
#include <AlopexOS/errorCodes.hpp>

class SystemX {
    AlopexOS::Kernel::DRAMA* p_drama;

    inline auto consoleLog() -> void;
    inline auto malloc(fennlib::types::usize size_bytes, AlopexOS::PID pid) -> fennlib::types::uintptr;

public:
    SystemX() = default;
    ~SystemX() = default;

    SystemX(AlopexOS::Kernel::DRAMA* drama) : p_drama(drama) {}

    inline auto invoke(fennlib::types::uintptr start_ptr, fennlib::types::uintptr end_ptr) -> fennlib::types::usize;

    [[deprecated("This function is not implemented or available yet.")]]
    inline auto execute(fennlib::string64 path)  -> fennlib::types::usize;
};

inline auto SystemX::malloc(fennlib::types::usize size_bytes, AlopexOS::PID pid) -> fennlib::types::uintptr
    { return p_drama->malloc(size_bytes, pid); }

inline auto SystemX::execute(fennlib::string64 path)  -> fennlib::types::usize { (void)path; return 0; }

auto SystemX::invoke(fennlib::types::uintptr start_ptr, fennlib::types::uintptr end_ptr) -> fennlib::types::usize {
    if (end_ptr <= start_ptr) {
        return static_cast<fennlib::types::usize>(-1) - static_cast<fennlib::types::usize>(AlopexOS::error_code::invalid_program_range);
    }
    fennlib::types::usize size = end_ptr - start_ptr;

    AlopexOS::PID pid = p_drama->lastIndex().pid+1;
    fennlib::types::uintptr program_space = this->malloc(size, pid);

    if (program_space == 0) {
        return 0;
    }

    auto* dest = reinterpret_cast<fennlib::types::uintptr*>(program_space);
    auto* src = reinterpret_cast<const fennlib::types::uintptr*>(start_ptr);
    fennlib::types::usize word_count = size / sizeof(fennlib::types::uintptr);

    for (fennlib::types::usize i = 0; i < word_count; ++i) {
        dest[i] = src[i];
    }

    using ProgramEntryPoint = auto (*)() -> fennlib::types::usize;
    ProgramEntryPoint entry_point = reinterpret_cast<ProgramEntryPoint>(program_space);

    return entry_point();
}

#endif