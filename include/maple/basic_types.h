#pragma once
#include <stdint.h>
#include <stddef.h>
#include <uchar.h>

// C23标准有但是mingw尚未跟上的char8_t，在这里定义。之后标准库header更新后也不用动，因为C11起允许重复typedef定义同一类型。
typedef unsigned char char8_t;

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef ptrdiff_t isize;
typedef size_t usize;
typedef float f32;
typedef double f64;
// 注意，该类型在utf8时表示utf8码元（一个字节），而完整的字符/字符串都需要用utf8[]或utf8*表示
typedef char8_t utf8;
typedef char16_t utf16;
typedef char32_t utf32;

// 实体，即ID
typedef u32 Entity;

typedef struct Vec2 {
    f32 x;
    f32 y;
} Vec2;
