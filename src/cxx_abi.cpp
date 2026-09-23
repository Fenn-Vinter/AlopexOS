#include <fennlib/types>


extern "C" int __cxa_atexit(void (*destructor)(void*), void* arg, void* dso_handle) {
    (void)destructor;
    (void)arg;
    (void)dso_handle;
    return 0;
}


extern "C" void* __dso_handle = nullptr;


namespace __cxxabiv1 {
    extern "C" int __cxa_guard_acquire(long* guard) {
        
        
        return *guard == 0 ? 1 : 0;
    }

    extern "C" void __cxa_guard_release(long* guard) {
        *guard = 1;
    }

    extern "C" void __cxa_guard_abort(long*) {
        
    }
}