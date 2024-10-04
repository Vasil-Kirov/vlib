#pragma once
#include <stdint.h>

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef uint32_t b32;
typedef float    f32;
typedef double   f64;

#define BIT(x) (1 << (x))

#define KB(N) (((unsigned long long)N) << 10)
#define MB(N) (((unsigned long long)N) << 20)
#define GB(N) (((unsigned long long)N) << 30)

