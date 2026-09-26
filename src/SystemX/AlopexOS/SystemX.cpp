#include "AlopexOS/AlopexOS_ErrorCodes.hpp"
#include "AlopexOS/gaossd/gaossd.hpp"
#include "arr.hpp"
#include "primitives.hpp"
#include <SystemXABI/ABI.hpp>
#include <SystemXABI/SystemX.hpp>

inline fn SystemX_ABI::Rax() -> RAX {
    RAX ret{};
    __asm__ volatile ("" : "=a"(ret));
    return ret;
}

inline fn SystemX_ABI::Rcx() -> RCX {
    RCX ret{};
    __asm__ volatile ("" : "=c"(ret));
    return ret;
}

constexpr usize SYS_WRITE = 0x1;
constexpr usize SYS_READ = 0x2;
constexpr usize SYS_EXISTS = 0x3;
constexpr usize SYS_MOUNT = 0x4;
constexpr usize SYS_DISMOUNT = 0x5;
constexpr usize SYS_SPAWN = 0x6;
constexpr usize SYS_KILL = 0x7;
constexpr usize SYS_GET_PID = 0x8;
constexpr usize SYS_EXIT = 0x9;
constexpr usize SYS_MMAP = 0xA;
constexpr usize SYS_MUNMAP = 0xB;
constexpr usize SYS_NET_SEND = 0xC;
constexpr usize SYS_NET_RECV = 0xD;
constexpr usize SYS_DIAGNOSTIC = 0xE;
constexpr usize SYS_OS_ARCH = 0xF;
constexpr usize SYS_OS_TYPE = 0x10;

fn SystemX_ABI::syscall(SystemX& kernel, RBX callID, R8 arg1, R9 arg2, R10 arg3) -> void {
    switch (callID) {
        case SYS_WRITE: { write(kernel, arg1, arg2, arg3); break; }
        case SYS_READ: { read(kernel, arg1, arg2, arg3); break; }
        case SYS_EXISTS: { exists(kernel, arg1); break; }
        case SYS_MOUNT: { mount(kernel, arg1); break; }
        case SYS_DISMOUNT: { dismount(kernel, arg1); break; }
        case SYS_SPAWN: { spawn(kernel, arg1, arg2); break; }
        case SYS_KILL: { kill(kernel, arg1); break; }
        case SYS_GET_PID: { get_pid(kernel); break; }
        case SYS_EXIT: { exit(kernel); break; }
        case SYS_MMAP: { mmap(kernel, arg1, arg2, arg3); break; }
        case SYS_MUNMAP: { munmap(kernel, arg1, arg2); break; }
        case SYS_NET_SEND: { net_send(kernel, arg1, arg2, arg3); break; }
        case SYS_NET_RECV: { net_recv(kernel, arg1, arg2, arg3); break; }
        case SYS_DIAGNOSTIC: { diagnostic(kernel, arg1, arg2); break; }
        case SYS_OS_ARCH: { os_arch(); break; }
        case SYS_OS_TYPE: { os_type(); break; }
        default: break;
    }
}

fn SystemX::attachAVFS(AlopexOS::AVFS* AVFS) -> AlopexOS::errorCode {
    this->avfs = AVFS;
    return AlopexOS::errorCode::Success;
}

fn SystemX::getAVFSInstance() -> AlopexOS::AVFS* {
    return this->avfs;
}

fn SystemX_ABI::write(SystemX& kernel, R8 PathBuffer, R9 DataBuffer, R10 DataSize) -> void {
    auto avfs = kernel.getAVFSInstance();

    const auto* path = reinterpret_cast<const AlopexOS::Path*>(PathBuffer.value);
    const byte* dataPtr = reinterpret_cast<const byte*>(DataBuffer.value);

    usize dataSize = static_cast<usize>(DataSize);
    dynarr<byte> dataView(dataPtr, dataSize);

    __asm__ volatile ("mov %0, %%rcx" : : "r"(static_cast<usize>(avfs->write(*path, dataView))) : "rcx");
}

fn SystemX_ABI::read(SystemX& kernel, R8 PathBuffer, R9 DataBuffer, R10 DataSize) -> void {
    auto avfs = kernel.getAVFSInstance();

    const auto* path = reinterpret_cast<const AlopexOS::Path*>(PathBuffer.value);
    byte* dataPtr = reinterpret_cast<byte*>(DataBuffer.value);

    usize dataSize = static_cast<usize>(DataSize);
    dynarr<byte> dataView(dataPtr, dataSize);

    __asm__ volatile ("mov %0, %%rcx" : : "r"(static_cast<usize>(avfs->read(*path, dataView))) : "rcx");
}

fn SystemX_ABI::exists(SystemX& kernel, R8 PathBuffer) -> void {
    __asm__ volatile ("mov %0, %%rcx" : : "r"(
        static_cast<usize>(kernel.getAVFSInstance()->exists(
            *reinterpret_cast<const AlopexOS::Path*>(PathBuffer.value)
        ))
    ) : "rcx"); 
}

fn SystemX_ABI::mount(SystemX& kernel, R8 PathBuffer) -> void {
    __asm__ volatile ("mov %0, %%rcx" : : "r"(
        static_cast<usize>(kernel.getAVFSInstance()->mount(
            *reinterpret_cast<const AlopexOS::Path*>(PathBuffer.value)
        ))
    ) : "rcx"); 
}

fn SystemX_ABI::dismount(SystemX& kernel, R8 PathBuffer) -> void {
    __asm__ volatile ("mov %0, %%rcx" : : "r"(
        static_cast<usize>(kernel.getAVFSInstance()->dismount(
            *reinterpret_cast<const AlopexOS::Path*>(PathBuffer.value)
        ))
    ) : "rcx"); 
}

constexpr const char* arch = __CURRENT_ARCH__;
constexpr const char* os = __CURRENT_OS__;

fn SystemX_ABI::os_arch() -> void {
    __asm__ volatile ("mov %0, %%rcx" : : "r"(reinterpret_cast<usize>(arch)) : "rcx");
}

fn SystemX_ABI::os_type() -> void {
    __asm__ volatile ("mov %0, %%rcx" : : "r"(reinterpret_cast<usize>(os)) : "rcx");
}

fn SystemX_ABI::spawn(SystemX& kernel, R8 PathBuffer, R9 Argv)           -> void {}
fn SystemX_ABI::kill(SystemX& kernel, R8 TargetPID)                  -> void {}
fn SystemX_ABI::get_pid(SystemX& kernel)                             -> void {}
fn SystemX_ABI::exit(SystemX& kernel)                                -> void {}
    
fn SystemX_ABI::mmap(SystemX& kernel, R8 Address, R9 Length, R10 Prot) -> void {}
fn SystemX_ABI::munmap(SystemX& kernel, R8 Address, R9 Length)         -> void {}

fn SystemX_ABI::net_send(SystemX& kernel, R8 SocketID, R9 DataBuffer, R10 Length) -> void {}
fn SystemX_ABI::net_recv(SystemX& kernel, R8 SocketID, R9 DataBuffer, R10 Length) -> void {}
fn SystemX_ABI::diagnostic(SystemX &kernel, R8 MessageBuffer, R9 Length) -> void {}

fn SystemX::executeProgram(const AlopexOS::Path& path) -> isize { return 0; }