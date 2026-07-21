#include <primitives.h>

extern "C" {

void* memset(void* dest, int c, usize n) {
    u8* p = static_cast<u8*>(dest);
    while (n--) {
        *p++ = static_cast<u8>(c);
    }
    return dest;
}

void* memcpy(void* dest, const void* src, usize n) {
    u8* d = static_cast<u8*>(dest);
    const u8* s = static_cast<const u8*>(src);
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

int __cxa_guard_acquire(i64* guard) {
    return !(*reinterpret_cast<volatile u8*>(guard));
}

void __cxa_guard_release(i64* guard) {
    *reinterpret_cast<volatile u8*>(guard) = 1;
}

void __cxa_guard_abort(i64*) {}

int __cxa_atexit(void (*)(void*), void*, void*) {
    return 0;
}

void* __dso_handle = nullptr;

}