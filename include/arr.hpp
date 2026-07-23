#if !defined(ARR_HPP)
#define ARR_HPP

#include <primitives.h>
#include <new.hpp>

namespace internal {
    template <typename T>
    inline void move_construct_or_assign(T* dest, T& src) {
        new (dest) T(static_cast<T&&>(src));
    }
}

template <typename T, size_t N>
class arr {
public:
    using value_type = T;
    using size_type = size_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;

    arr() {
        for (size_type i = 0; i < N; ++i) {
            new (&data_[i]) T();
        }
    }

    ~arr() {
        for (size_type i = 0; i < N; ++i) {
            data_[i].~T();
        }
    }

    arr(const arr& other) {
        for (size_type i = 0; i < N; ++i) {
            new (&data_[i]) T(other.data_[i]);
        }
    }

    arr(arr&& other) noexcept {
        for (size_type i = 0; i < N; ++i) {
            new (&data_[i]) T(static_cast<T&&>(other.data_[i]));
        }
    }

    constexpr size_type size() const { return N; }
    constexpr bool empty() const { return N == 0; }

    reference operator[](size_type i) { return data_[i]; }
    const_reference operator[](size_type i) const { return data_[i]; }

    pointer begin() { return data_; }
    pointer end() { return data_ + N; }
    const_pointer begin() const { return data_; }
    const_pointer end() const { return data_ + N; }

    pointer data() { return data_; }
    const_pointer data() const { return data_; }
private:
    T data_[N];
};

template <typename T>
class dynarr {
public:
    using value_type = T;
    using size_type = size_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;

    dynarr() : data_(nullptr), size_(0), capacity_(0) {}

    explicit dynarr(size_type n) : data_(nullptr), size_(0), capacity_(0) {
        resize(n);
    }

    ~dynarr() {
        clear();
        if (data_) {
            ::operator delete(data_);
        }
    }

    dynarr(const dynarr& other) : data_(nullptr), size_(0), capacity_(0) {
        resize(other.size_);
        for (size_type i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
    }

    dynarr(dynarr&& other) noexcept 
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    dynarr& operator=(const dynarr& other) {
        if (this != &other) {
            clear();
            resize(other.size_);
            for (size_type i = 0; i < size_; ++i) {
                data_[i] = other.data_[i];
            }
        }
        return *this;
    }

    dynarr& operator=(dynarr&& other) noexcept {
        if (this != &other) {
            clear();
            if (data_) ::operator delete(data_);
            
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    void reserve(size_type new_cap) {
        if (new_cap > capacity_) {
            grow(new_cap);
        }
    }

    void push_back(const T& value) {
        if (size_ >= capacity_) {
            grow(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        new (&data_[size_]) T(value);
        ++size_;
    }

    void push_back(T&& value) {
        if (size_ >= capacity_) {
            grow(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        new (&data_[size_]) T(static_cast<T&&>(value));
        ++size_;
    }

    template <typename... Args>
    void push_back(Args&&... args) {
        if (size_ >= capacity_) {
            grow(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        new (&data_[size_]) T(static_cast<Args&&>(args)...);
        ++size_;
    }

    void append(const T& value) {
        push_back(value);
    }

    void append(T&& value) {
        push_back(static_cast<T&&>(value));
    }

    template <typename... Args>
    void append(Args&&... args) {
        push_back(static_cast<Args&&>(args)...);
    }

    void pop_back() {
        if (size_ > 0) {
            --size_;
            data_[size_].~T();
        }
    }

    void resize(size_type n) {
        if (n > capacity_) {
            grow(n);
        }
        if (n > size_) {
            for (size_type i = size_; i < n; ++i) {
                new (&data_[i]) T();
            }
        } else if (n < size_) {
            for (size_type i = n; i < size_; ++i) {
                data_[i].~T();
            }
        }
        size_ = n;
    }

    void clear() {
        for (size_type i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        size_ = 0;
    }

    reference operator[](size_type i) { return data_[i]; }
    const_reference operator[](size_type i) const { return data_[i]; }

    reference at(size_type i) {
        if (i >= size_) while(1);
        return data_[i];
    }

    size_type size() const { return size_; }
    size_type capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }

    pointer data() { return data_; }
    const_pointer data() const { return data_; }

    pointer begin() { return data_; }
    pointer end() { return data_ + size_; }
    const_pointer begin() const { return data_; }
    const_pointer end() const { return data_ + size_; }

private:
    T* data_;
    size_type size_;
    size_type capacity_;

    void grow(size_type new_cap) {
        T* new_data = static_cast<T*>(::operator new(sizeof(T) * new_cap));
        
        for (size_type i = 0; i < size_; ++i) {
            new (&new_data[i]) T(static_cast<T&&>(data_[i]));
        }

        for (size_type i = 0; i < size_; ++i) {
            data_[i].~T();
        }

        if (data_) {
            ::operator delete(data_);
        }

        data_ = new_data;
        capacity_ = new_cap;
    }
};

#endif