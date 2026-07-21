#if !defined(STRING_HPP)
#define STRING_HPP

#include <primitives.h>
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
        string_view();
        string_view(const char* cstr);
        string_view(const char* data, u64 len);
        
        template <u64 SSO_Capacity>
        string_view(const basic_string<SSO_Capacity>& str);

        auto data() const -> const char*;
        auto length() const -> u64;
        auto is_empty() const -> bool;

        auto operator[](u64 index) const -> char;

    private:
        const char* _data;
        u64 _length;
};

template <u64 SSO_Capacity>
class AlopexOS::basic_string {
    public:
        basic_string();
        basic_string(const char* cstr);
        basic_string(string_view view);
        basic_string(const basic_string& other);
        basic_string(basic_string&& other) noexcept;
        ~basic_string();

        auto operator=(const basic_string& other) -> basic_string&;
        auto operator=(basic_string&& other) noexcept -> basic_string&;

        auto c_str() const -> const char*;
        auto length() const -> u64;
        auto capacity() const -> u64;
        auto clear() -> void;
        auto append(const char* str, u64 len) -> void;
        auto append(string_view view) -> void;

        operator string_view() const;

    private:
        struct heap_repr;

        union storage {
            char sso[SSO_Capacity + 1];
            heap_repr heap;
        };

        storage _storage;
        bool _is_sso = true;
        u64 _length = 0;
};

template <u64 SSO_Capacity>
struct AlopexOS::basic_string<SSO_Capacity>::heap_repr {
    char* ptr;
    u64 size;
    u64 capacity;
};

#endif