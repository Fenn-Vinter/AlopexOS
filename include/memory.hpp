#if !defined (MEMORY_HPP)
#define MEMORY_HPP

// AlopexOS Core Memory Management & Utilities
// Brand: Fenn Vinter / AurenFox.Studio

#ifndef FN
#define fn auto
#endif

namespace AurenFox::memory {

template <typename T>
struct remove_reference { 
    using type = T; 
};

template <typename T>
struct remove_reference<T&> { 
    using type = T; 
};

template <typename T>
struct remove_reference<T&&> { 
    using type = T; 
};

template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

// Custom move semantic primitive
template <typename T>
constexpr fn move(T& arg) noexcept -> remove_reference_t<T>&& {
    return static_cast<remove_reference_t<T>&&>(arg);
}

// Low-level placement construction helper
template <typename T, typename... Args>
constexpr fn construct_at(T* ptr, Args&&... args) -> T* {
    return ::new (static_cast<void*>(ptr)) T(static_cast<Args&&>(args)...);
}

// Low-level explicit destruction helper
template <typename T>
constexpr fn destroy_at(T* ptr) noexcept -> void {
    if constexpr (!__is_trivially_destructible(T)) {
        ptr->~T();
    }
}

} // namespace AurenFox::memory

#endif