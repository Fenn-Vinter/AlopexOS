#include <fennlib/types>

using namespace fennlib::types;

u8 heap_pool[16 * 1024 * 1024];
usize heap_offset = 0;

extern "C" {
    auto memset(void* dest, int c, usize n) -> void* {
        auto* p = static_cast<unsigned char*>(dest);
        for (usize i = 0; i < n; ++i) {
            p[i] = static_cast<unsigned char>(c);
        }
        return dest;
    }

    auto memcpy(void* dest, const void* src, usize n) -> void* {
        auto* d = static_cast<unsigned char*>(dest);
        const auto* s = static_cast<const unsigned char*>(src);
        for (usize i = 0; i < n; ++i) {
            d[i] = s[i];
        }
        return dest;
    }

    auto kmalloc(usize size) -> void* {
        if (heap_offset + size > sizeof(heap_pool)) return nullptr;
        void* ptr = &heap_pool[heap_offset];
        heap_offset += (size + 7) & ~7ul;
        return ptr;
    }

    auto kfree(void* ptr) -> void {
        (void)ptr;
    }
}

auto operator new(usize size) -> void* {
    return kmalloc(size);
}

auto operator new[](usize size) -> void* {
    return kmalloc(size);
}

auto operator delete(void* ptr) noexcept -> void {
    kfree(ptr);
}

auto operator delete(void* ptr, usize size) noexcept -> void {
    (void)size;
    kfree(ptr);
}

auto operator delete[](void* ptr) noexcept -> void {
    kfree(ptr);
}