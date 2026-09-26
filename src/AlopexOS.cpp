#include <AlopexOS/AlopexOS.hpp>

extern "C" fn AlopexOS::AlopexOS_SysCall(
    usize syscall_number, 
    usize arg1, 
    usize arg2, 
    usize arg3
) noexcept -> usize {
    usize ret;
    
    __asm__ volatile (
        "syscall"
        : "=a" (ret)
        : "a" (syscall_number), 
          "D" (arg1), 
          "S" (arg2), 
          "d" (arg3)
        : "rcx", "r11", "memory"
    );
    
    return ret;
}
