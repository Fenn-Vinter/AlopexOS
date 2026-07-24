#if !defined(PRIMETIVES_HPP)
#define PRIMETIVES_HPP

using i8  = signed char;
using i16 = signed short int;
using i32 = signed int;
using i64 = signed long long int;

static_assert(sizeof(i8)  == 1, "i8  (signed char)");
static_assert(sizeof(i16) == 2, "i16 (signed short int)");
static_assert(sizeof(i32) == 4, "i32 (signed int)");
static_assert(sizeof(i64) == 8, "i64 (signed long long int)");

using uint = unsigned int;
using u8   = unsigned char;
using u16  = unsigned short int;
using u32  = unsigned int;
using u64  = unsigned long long int;

static_assert(sizeof(uint) == 4, "uint (unsigned int)");
static_assert(sizeof(u8)   == 1, "i8   (unsigned char)");
static_assert(sizeof(u16)  == 2, "i16  (unsigned short int)");
static_assert(sizeof(u32)  == 4, "i32  (unsigned int)");
static_assert(sizeof(u64)  == 8, "i64  (unsigned long long int)");

using wild = unsigned int;
using w8  = unsigned char;
using w16 = unsigned short int;
using w32 = unsigned int;
using w64 = unsigned long long int;

static_assert(sizeof(wild) == 4, "wild (unsigned int)");
static_assert(sizeof(w8)  == 1,  "w8   (unsigned char)");
static_assert(sizeof(w16) == 2,  "w16  (unsigned short int)");
static_assert(sizeof(w32) == 4,  "w32  (unsigned int)");
static_assert(sizeof(w64) == 8,  "w64  (unsigned long long int)");

using f32 = float;
using f64 = double;

using ptr = void*;
using uptr = unsigned long long;

#define fn auto

#if defined (__SIZE_TYPE__)
using size_t =__SIZE_TYPE__;
#else
using size_t = unsigned long;
#endif

using usize = size_t;

using bit = bool;
using byte = u8;

using cstr = char*;

template <typename T>
struct vector2D {
    T x{}, y{};
};

template <typename T>
struct vector3D {
    T x{}, y{}, z{};
};

template <typename T>
struct vector4D {
    T x{}, y{}, z{}, w{};
};

using ivector2D = vector2D<int>;
using ivector3D = vector3D<int>;
using ivector4D = vector4D<int>;

using uvector2D = vector2D<uint>;
using uvector3D = vector3D<uint>;
using uvector4D = vector4D<uint>;

using wvector2D = vector2D<wild>;
using wvector3D = vector3D<wild>;
using wvector4D = vector4D<wild>;

using fvector2D = vector2D<float>;
using fvector3D = vector3D<float>;
using fvector4D = vector4D<float>;

using dvector2D = vector2D<double>;
using dvector3D = vector3D<double>;
using dvector4D = vector4D<double>;

#endif