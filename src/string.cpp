#include <string.hpp>

AlopexOS::string_view::string_view() 
    : _data(nullptr), _length(0) {}

AlopexOS::string_view::string_view(const char* cstr) 
    : _data(cstr), _length(0) {
    if (_data != nullptr) {
        while (_data[_length] != '\0') {
            _length++;
        }
    }
}

AlopexOS::string_view::string_view(const char* data, u64 len) 
    : _data(data), _length(len) {}

template <u64 SSO_Capacity>
AlopexOS::string_view::string_view(const basic_string<SSO_Capacity>& str) 
    : _data(str.c_str()), _length(str.length()) {}

auto AlopexOS::string_view::data() const -> const char* {
    return _data;
}

auto AlopexOS::string_view::length() const -> u64 {
    return _length;
}

auto AlopexOS::string_view::is_empty() const -> bool {
    return _length == 0;
}

auto AlopexOS::string_view::operator[](u64 index) const -> char {
    return _data[index];
}

template <u64 SSO_Capacity> 
AlopexOS::basic_string<SSO_Capacity>
::basic_string() {
    _storage.sso[0] = '\0';
    _is_sso = true;
    _length = 0;
}

template <u64 SSO_Capacity> 
AlopexOS::basic_string<SSO_Capacity>
::basic_string(const char* cstr) {
    u64 len = 0;
    while (cstr[len] != '\0') {
        len++;
    }
                
    if (len <= SSO_Capacity) {
        for (u64 i = 0; i <= len; i++) {
            _storage.sso[i] = cstr[i];
        }
        _is_sso = true;
        _length = len;
    } else {
        _storage.heap.ptr = new char[len + 1];
        for (u64 i = 0; i <= len; i++) {
            _storage.heap.ptr[i] = cstr[i];
        }
        _storage.heap.size = len;
        _storage.heap.capacity = len;
        _is_sso = false;
        _length = len;
    }
}

template <u64 SSO_Capacity>
AlopexOS::basic_string<SSO_Capacity>
::basic_string(string_view view) {
    u64 len = view.length();
    if (len <= SSO_Capacity) {
        const char* data = view.data();
        for (u64 i = 0; i < len; i++) {
            _storage.sso[i] = data[i];
        }
        _storage.sso[len] = '\0';
        _is_sso = true;
        _length = len;
    } else {
        _storage.heap.ptr = new char[len + 1];
        const char* data = view.data();
        for (u64 i = 0; i < len; i++) {
            _storage.heap.ptr[i] = data[i];
        }
        _storage.heap.ptr[len] = '\0';
        _storage.heap.size = len;
        _storage.heap.capacity = len;
        _is_sso = false;
        _length = len;
    }
}

template <u64 SSO_Capacity> 
AlopexOS::basic_string<SSO_Capacity>
::basic_string(const basic_string& other) {
    if (other._is_sso) {
        for (u64 i = 0; i <= other._length; i++) {
            _storage.sso[i] = other._storage.sso[i];
        }
        _is_sso = true;
        _length = other._length;
    } else {
        _storage.heap.ptr = new char[other._length + 1];
        for (u64 i = 0; i <= other._length; i++) {
            _storage.heap.ptr[i] = other._storage.heap.ptr[i];
        }
        _storage.heap.size = other._length;
        _storage.heap.capacity = other._length;
        _is_sso = false;
        _length = other._length;
    }
}

template <u64 SSO_Capacity> 
AlopexOS::basic_string<SSO_Capacity>
::basic_string(basic_string&& other) noexcept {
    _is_sso = other._is_sso;
    _length = other._length;
    if (_is_sso) {
        for (u64 i = 0; i <= _length; i++) {
            _storage.sso[i] = other._storage.sso[i];
        }
    } else {
        _storage.heap = other._storage.heap;
        other._storage.heap.ptr = nullptr;
        other._storage.heap.size = 0;
        other._storage.heap.capacity = 0;
    }
    other._is_sso = true;
    other._length = 0;
    other._storage.sso[0] = '\0';
}

template <u64 SSO_Capacity> 
AlopexOS::basic_string<SSO_Capacity>
::~basic_string<SSO_Capacity>() {
    if (!_is_sso && _storage.heap.ptr != nullptr) {
        delete[] _storage.heap.ptr;
    }
}

template <u64 SSO_Capacity> 
auto AlopexOS::basic_string<SSO_Capacity>
::operator=(const basic_string& other) -> basic_string& {
    if (this == &other) {
        return *this;
    }
                
    if (!_is_sso) {
        delete[] _storage.heap.ptr;
    }

    _is_sso = other._is_sso;
    _length = other._length;

    if (_is_sso) {
        for (u64 i = 0; i <= _length; i++) {
            _storage.sso[i] = other._storage.sso[i];
        }
    } else {
        _storage.heap.ptr = new char[_length + 1];
        for (u64 i = 0; i <= _length; i++) {
            _storage.heap.ptr[i] = other._storage.heap.ptr[i];
        }
        _storage.heap.size = _length;
        _storage.heap.capacity = _length;
    }
    return *this;
}

template <u64 SSO_Capacity> 
auto AlopexOS::basic_string<SSO_Capacity>
::operator=(basic_string&& other) noexcept -> basic_string& {
    if (this == &other) {
        return *this;
    }

    if (!_is_sso) {
        delete[] _storage.heap.ptr;
    }

    _is_sso = other._is_sso;
    _length = other._length;

    if (_is_sso) {
        for (u64 i = 0; i <= _length; i++) {
            _storage.sso[i] = other._storage.sso[i];
        }
    } else {
        _storage.heap = other._storage.heap;
        other._storage.heap.ptr = nullptr;
        other._storage.heap.size = 0;
        other._storage.heap.capacity = 0;
    }

    other._is_sso = true;
    other._length = 0;
    other._storage.sso[0] = '\0';

    return *this;
}

template <u64 SSO_Capacity> 
auto AlopexOS::basic_string<SSO_Capacity>
::c_str() const -> const char* {
    if (_is_sso) {
        return _storage.sso;
    }
    return _storage.heap.ptr;
}

template <u64 SSO_Capacity> 
auto AlopexOS::basic_string<SSO_Capacity>
::length() const -> u64 { return _length; }

template <u64 SSO_Capacity> 
auto AlopexOS::basic_string<SSO_Capacity>
::capacity() const -> u64 {
    if (_is_sso) {
        return SSO_Capacity;
    }
    return _storage.heap.capacity;
}

template <u64 SSO_Capacity> 
auto AlopexOS::basic_string<SSO_Capacity>
::clear() -> void {
    if (!_is_sso) {
        delete[] _storage.heap.ptr;
        _is_sso = true;
    }
    _length = 0;
    _storage.sso[0] = '\0';
}

template <u64 SSO_Capacity> 
auto AlopexOS::basic_string<SSO_Capacity>
::append(const char* str, u64 len) -> void {
    u64 new_len = _length + len;
    if (_is_sso && new_len <= SSO_Capacity) {
        for (u64 i = 0; i < len; i++) {
            _storage.sso[_length + i] = str[i];
        }
        _length = new_len;
        _storage.sso[_length] = '\0';
    } else {
        u64 cap = _is_sso ? SSO_Capacity : _storage.heap.capacity;
        while (cap < new_len) {
            cap = cap * 2;
        }
        
        char* new_ptr = new char[cap + 1];
        const char* old_ptr = c_str();
        
        for (u64 i = 0; i < _length; i++) {
            new_ptr[i] = old_ptr[i];
        }
        for (u64 i = 0; i < len; i++) {
            new_ptr[_length + i] = str[i];
        }
        new_ptr[new_len] = '\0';

        if (!_is_sso) {
            delete[] _storage.heap.ptr;
        }

        _storage.heap.ptr = new_ptr;
        _storage.heap.size = new_len;
        _storage.heap.capacity = cap;
        _is_sso = false;
        _length = new_len;
    }
}

template <u64 SSO_Capacity>
auto AlopexOS::basic_string<SSO_Capacity>
::append(string_view view) -> void {
    append(view.data(), view.length());
}

template <u64 SSO_Capacity>
AlopexOS::basic_string<SSO_Capacity>::operator string_view() const {
    return string_view(c_str(), _length);
}