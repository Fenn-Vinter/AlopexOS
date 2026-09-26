#include "primitives.hpp"
#include "SystemX.hpp"

#if !defined (SYSTEM_X_ABI_HPP)
#define SYSTEM_X_ABI_HPP

template <typename Tag>
struct Register {
    usize value;
    constexpr Register() = default;

    constexpr explicit Register(usize v) : value(v) {}

    template <typename OtherTag>
    constexpr explicit Register(Register<OtherTag> other) : value(other.value) {}

    constexpr operator usize() const { return value; }
};

struct RAX_T {};
struct RBX_T {};
struct RCX_T {};
struct RDX_T {};
struct R8_T {};
struct R9_T {};
struct R10_T {};
struct R11_T {};
struct R12_T {};
struct R13_T {};
struct R14_T {};
struct R15_T {};

using RAX = Register<RAX_T>; // return type
using RBX = Register<RBX_T>; // Syscall identifier / command number (syscall number)
using RCX = Register<RCX_T>; // Error code / secondary return value or flags
using RDX = Register<RDX_T>; // Additional flags, mode bits, or extended options

using R8 = Register<R8_T>;   // arg1
using R9 = Register<R9_T>;   // arg2
using R10 = Register<R10_T>; // arg3
using R11 = Register<R11_T>; // arg4
using R12 = Register<R12_T>; // arg5
using R13 = Register<R13_T>; // arg6
using R14 = Register<R14_T>; // arg7
using R15 = Register<R15_T>; // arg8

namespace SystemX_ABI {
    // Register Accessors
    inline fn Rax() -> RAX;
    inline fn Rcx() -> RCX;

    // Generic Dispatch
    fn syscall(SystemX& kernel, RBX callID, R8 arg1, R9 arg2, R10 arg3) -> void;

    // File & Storage Operations
    fn write(SystemX& kernel, R8 PathBuffer, R9 DataBuffer, R10 DataSize)   -> void /*RAX / RCX*/;
    fn read(SystemX& kernel, R8 PathBuffer, R9 DataBuffer, R10 DataSize)    -> void /*RAX / RCX*/;
    fn exists(SystemX& kernel, R8 PathBuffer)                 -> void /*RCX*/;
    fn mount(SystemX& kernel, R8 PathBuffer)                  -> void /*RCX*/;
    fn dismount(SystemX& kernel, R8 PathBuffer)               -> void /*RCX*/;

    // Process & Task Management
    fn spawn(SystemX& kernel, R8 PathBuffer, R9 Argv)         -> void /*RAX (PID) / RCX*/;
    fn kill(SystemX& kernel, R8 TargetPID)                    -> void /*RCX*/;
    fn get_pid(SystemX& kernel)                             -> void /*RAX*/;
    fn exit(SystemX& kernel)                                -> void /**/;

    // Memory Management (MEMORY_MAP permission)
    fn mmap(SystemX& kernel, R8 Address, R9 Length, R10 Prot) -> void /*RAX (Mapped Addr) / RCX*/;
    fn munmap(SystemX& kernel, R8 Address, R9 Length)         -> void /*RCX*/;

    // Networking (NETWORK permission)
    fn net_send(SystemX& kernel, R8 SocketID, R9 DataBuffer, R10 Length) -> void /*RAX / RCX*/;
    fn net_recv(SystemX& kernel, R8 SocketID, R9 DataBuffer, R10 Length) -> void /*RAX / RCX*/;

    // System Diagnostics & Info
    fn diagnostic(SystemX& kernel, R8 MessageBuffer, R9 Length) -> void /*RCX*/;
    fn os_arch() -> void /*RAX*/;
    fn os_type()   -> void /*RAX*/;
}

#endif