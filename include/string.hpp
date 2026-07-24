#if !defined(STRING_HPP)
#define STRING_HPP

#include <primitives.hpp>
#include <arr.hpp>

namespace AlopexOS {
    class string_view;

    template <u64 SSO_Capacity = 23>
    class basic_string;

    using string = basic_string<23>;
    using string8 = basic_string<8>;
    using string16 = basic_string<16>;
    using string32 = basic_string<32>;
    using string64 = basic_string<64>;
    using string128 = basic_string<128>;
    using string256 = basic_string<256>;
    using string512 = basic_string<512>;
    using string1024 = basic_string<1024>;
    using string2048 = basic_string<2048>;
    using string4096 = basic_string<4096>;
    using string8192 = basic_string<8192>;
    using string16384 = basic_string<16384>;
}

class AlopexOS::string_view {
    public:
        string_view() : _data(nullptr), _length(0) {}
        string_view(const char* cstr) : _data(cstr), _length(0) {
            if (cstr) {
                while (cstr[_length] != '\0') _length++;
            }
        }
        string_view(const char* data, u64 len) : _data(data), _length(len) {}
        
        template <u64 SSO_Capacity>
        string_view(const basic_string<SSO_Capacity>& str) : _data(str.c_str()), _length(str.length()) {}

        auto data() const -> const char* { return _data; }
        auto length() const -> u64 { return _length; }
        auto is_empty() const -> bool { return _length == 0; }

        auto operator[](u64 index) const -> char { return _data[index]; }

    private:
        const char* _data;
        u64 _length;
};

template <u64 SSO_Capacity>
class AlopexOS::basic_string {
    public:
        basic_string() {
            _is_sso = true;
            _length = 0;
            _storage.sso[0] = '\0';
        }

        basic_string(const char* cstr) {
            _is_sso = true;
            _length = 0;
            _storage.sso[0] = '\0';
            if (cstr) {
                u64 len = 0;
                while (cstr[len] != '\0') len++;
                append(cstr, len);
            }
        }

        basic_string(string_view view) {
            _is_sso = true;
            _length = 0;
            _storage.sso[0] = '\0';
            append(view.data(), view.length());
        }

        basic_string(const basic_string& other) {
            _is_sso = true;
            _length = 0;
            _storage.sso[0] = '\0';
            append(other.c_str(), other.length());
        }

        basic_string(basic_string&& other) noexcept {
            _is_sso = other._is_sso;
            _length = other._length;
            if (_is_sso) {
                for (u64 i = 0; i <= _length; ++i) {
                    _storage.sso[i] = other._storage.sso[i];
                }
            } else {
                _storage.heap = other._storage.heap;
            }
            other._is_sso = true;
            other._length = 0;
            other._storage.sso[0] = '\0';
        }

        ~basic_string() {
            clear();
        }

        auto operator=(const basic_string& other) -> basic_string& {
            if (this != &other) {
                clear();
                append(other.c_str(), other.length());
            }
            return *this;
        }

        auto operator=(basic_string&& other) noexcept -> basic_string& {
            if (this != &other) {
                clear();
                _is_sso = other._is_sso;
                _length = other._length;
                if (_is_sso) {
                    for (u64 i = 0; i <= _length; ++i) {
                        _storage.sso[i] = other._storage.sso[i];
                    }
                } else {
                    _storage.heap = other._storage.heap;
                }
                other._is_sso = true;
                other._length = 0;
                other._storage.sso[0] = '\0';
            }
            return *this;
        }

        auto operator+=(const basic_string& other) -> basic_string& {
            append(other.c_str(), other.length());
            return *this;
        }

        auto operator+=(string_view view) -> basic_string& {
            append(view.data(), view.length());
            return *this;
        }

        auto operator+=(const char* cstr) -> basic_string& {
            if (cstr) {
                u64 len = 0;
                while (cstr[len] != '\0') len++;
                append(cstr, len);
            }
            return *this;
        }

        template <u64 N>
        auto operator+=(const char (&arr)[N]) -> basic_string& {
            u64 len = N > 0 && arr[N - 1] == '\0' ? N - 1 : N;
            append(arr, len);
            return *this;
        }

        auto operator+=(char ch) -> basic_string& {
            append(&ch, 1);
            return *this;
        }

        template <typename T>
        auto operator+=(const dynarr<T>& other) -> basic_string&
            requires (sizeof(T) == 1) {
            append(reinterpret_cast<const char*>(other.data()), other.size());
            return *this;
        }

        template <typename T, size_t N>
        auto operator+=(const arr<T, N>& other) -> basic_string&
            requires (sizeof(T) == 1) {
            append(reinterpret_cast<const char*>(other.begin()), N);
            return *this;
        }

        fn c_str() const -> const char* {
            return _is_sso ? _storage.sso : _storage.heap.ptr;
        }

        fn length() const -> u64 {
            return _length;
        }

        fn capacity() const -> u64 {
            return _is_sso ? SSO_Capacity : _storage.heap.capacity;
        }

        fn clear() -> void {
            if (!_is_sso && _storage.heap.ptr) {
                delete[] _storage.heap.ptr;
            }
            _is_sso = true;
            _length = 0;
            _storage.sso[0] = '\0';
        }

        fn empty() const -> bool {
            return _length == 0;
        }

        fn append(const char* str, u64 len) -> void {
            if (len == 0) return;
            u64 new_len = _length + len;
            if (_is_sso) {
                if (new_len <= SSO_Capacity) {
                    const char* src = str;
                    char local_buf[256];
                    if (str >= _storage.sso && str < _storage.sso + SSO_Capacity + 1 && len < sizeof(local_buf)) {
                        for(u64 i=0; i<len; ++i) local_buf[i] = str[i];
                        src = local_buf;
                    }
                    for (u64 i = 0; i < len; ++i) {
                        _storage.sso[_length + i] = src[i];
                    }
                    _length = new_len;
                    _storage.sso[_length] = '\0';
                    return;
                } else {
                    u64 new_cap = new_len * 2;
                    if (new_cap < 32) new_cap = 32;
                    char* new_ptr = new char[new_cap + 1];
                    for (u64 i = 0; i < _length; ++i) {
                        new_ptr[i] = _storage.sso[i];
                    }
                    for (u64 i = 0; i < len; ++i) {
                        new_ptr[_length + i] = str[i];
                    }
                    _length = new_len;
                    new_ptr[_length] = '\0';
                    _storage.heap.ptr = new_ptr;
                    _storage.heap.size = _length;
                    _storage.heap.capacity = new_cap;
                    _is_sso = false;
                    return;
                }
            } else {
                if (new_len <= _storage.heap.capacity) {
                    for (u64 i = 0; i < len; ++i) {
                        _storage.heap.ptr[_length + i] = str[i];
                    }
                    _length = new_len;
                    _storage.heap.ptr[_length] = '\0';
                    _storage.heap.size = _length;
                    return;
                } else {
                    u64 new_cap = new_len * 2;
                    char* new_ptr = new char[new_cap + 1];
                    for (u64 i = 0; i < _length; ++i) {
                        new_ptr[i] = _storage.heap.ptr[i];
                    }
                    for (u64 i = 0; i < len; ++i) {
                        new_ptr[_length + i] = str[i];
                    }
                    delete[] _storage.heap.ptr;
                    _length = new_len;
                    new_ptr[_length] = '\0';
                    _storage.heap.ptr = new_ptr;
                    _storage.heap.size = _length;
                    _storage.heap.capacity = new_cap;
                    return;
                }
            }
        }

        fn append(string_view view) -> void {
            append(view.data(), view.length());
        }

        operator string_view() const {
            return string_view(c_str(), _length);
        }

    private:
        struct heap_repr {
            char* ptr;
            u64 size;
            u64 capacity;
        };

        union storage {
            char sso[SSO_Capacity + 1];
            heap_repr heap;
        };

        storage _storage;
        bool _is_sso = true;
        u64 _length = 0;
};

// Comparison Operators
template <u64 Cap1, u64 Cap2>
inline auto operator==(const AlopexOS::basic_string<Cap1>& lhs, const AlopexOS::basic_string<Cap2>& rhs) -> bool {
    if (lhs.length() != rhs.length()) return false;
    AlopexOS::string_view sv1 = lhs;
    AlopexOS::string_view sv2 = rhs;
    for (u64 i = 0; i < sv1.length(); ++i) {
        if (sv1[i] != sv2[i]) return false;
    }
    return true;
}

template <u64 Cap1, u64 Cap2>
inline auto operator!=(const AlopexOS::basic_string<Cap1>& lhs, const AlopexOS::basic_string<Cap2>& rhs) -> bool {
    return !(lhs == rhs);
}

template <u64 Cap1, u64 Cap2>
inline auto operator<(const AlopexOS::basic_string<Cap1>& lhs, const AlopexOS::basic_string<Cap2>& rhs) -> bool {
    AlopexOS::string_view sv1 = lhs;
    AlopexOS::string_view sv2 = rhs;
    u64 min_len = sv1.length() < sv2.length() ? sv1.length() : sv2.length();
    for (u64 i = 0; i < min_len; ++i) {
        if (sv1[i] < sv2[i]) return true;
        if (sv1[i] > sv2[i]) return false;
    }
    return sv1.length() < sv2.length();
}

template <u64 Cap1, u64 Cap2>
inline auto operator>(const AlopexOS::basic_string<Cap1>& lhs, const AlopexOS::basic_string<Cap2>& rhs) -> bool {
    return rhs < lhs;
}

template <u64 Cap1, u64 Cap2>
inline auto operator<=(const AlopexOS::basic_string<Cap1>& lhs, const AlopexOS::basic_string<Cap2>& rhs) -> bool {
    return !(rhs < lhs);
}

template <u64 Cap1, u64 Cap2>
inline auto operator>=(const AlopexOS::basic_string<Cap1>& lhs, const AlopexOS::basic_string<Cap2>& rhs) -> bool {
    return !(lhs < rhs);
}

#endif