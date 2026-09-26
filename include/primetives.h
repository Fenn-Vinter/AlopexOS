#if !defined(PRIMETIVES_H)
#define PRIMETIVES_H

#include <stdbool.h>

typedef signed char i8;
typedef signed short int i16;
typedef signed int i32;
typedef signed long long int i64;

static_assert(sizeof(i8)  == 1, "i8  (signed char)");
static_assert(sizeof(i16) == 2, "i16 (signed short int)");
static_assert(sizeof(i32) == 4, "i32 (signed int)");
static_assert(sizeof(i64) == 8, "i64 (signed long long int)");

typedef unsigned int uint;
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;

static_assert(sizeof(uint) == 4, "uint (unsigned int)");
static_assert(sizeof(u8)   == 1, "i8   (unsigned char)");
static_assert(sizeof(u16)  == 2, "i16  (unsigned short int)");
static_assert(sizeof(u32)  == 4, "i32  (unsigned int)");
static_assert(sizeof(u64)  == 8, "i64  (unsigned long long int)");

typedef unsigned int wild;
typedef unsigned char w8;
typedef unsigned short int w16;
typedef unsigned int w32;
typedef unsigned long long int w64;

static_assert(sizeof(wild) == 4, "wild (unsigned int)");
static_assert(sizeof(w8)  == 1,  "w8   (unsigned char)");
static_assert(sizeof(w16) == 2,  "w16  (unsigned short int)");
static_assert(sizeof(w32) == 4,  "w32  (unsigned int)");
static_assert(sizeof(w64) == 8,  "w64  (unsigned long long int)");

typedef float f32;
typedef double f64;

typedef void* ptr;
typedef unsigned long long uptr;

#define fn auto

#if defined (__SIZE_TYPE__)
typedef __SIZE_TYPE__ size_t;
#else
typedef unsigned long size_t;
#endif

typedef size_t usize;

typedef bool bit;
typedef u8 byte;

typedef char* cstr;

#endif