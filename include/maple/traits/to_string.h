#pragma once

#include <maple/basic_types.h>
#include <stdio.h>
#include <stb_ds.h>

/// var to be processed named 'utf8_string'
/// heap-allocated result, do arrfree() after use!
#define DECLARE_INTERFACE_ToString(type, prefix) \
const utf8* prefix##_to_string_fn(struct type* obj);

#define IMPL_INTERFACE_ToString_ToString_BEGIN(type, prefix) \
const utf8* prefix##_to_string_fn(struct type* obj) { \
    static bool able_to_stringify = true; \
    static const char* type_string = #type; \
    utf8* utf8_string = nullptr; \
    arrsetcap(utf8_string, 64);
#define IMPL_INTERFACE_ToString_ToString_END(type, prefix) \
    return utf8_string; \
}

// Attention: utf8 fmt!
#define TOSTRING_ITEM(max_bytes, fmt, ...) do { \
    if (able_to_stringify) { \
        char buf[max_bytes]; \
        const char* fmt_plain_ptr = (const char*)(fmt); \
        i32 written = snprintf(buf, (max_bytes), fmt_plain_ptr, ##__VA_ARGS__); \
        if (written >= (max_bytes)) { \
            LOG_ERROR(u8"Failed to stringify item because of buffer overflow. Of type '%s', fmt '%s'. Due to that, the stringify processes later are all disabled. Consider to increase the max_bytes arg", type_string, fmt_plain_ptr); \
            able_to_stringify = false; \
            break; \
        } \
        bool is_terminater_former = (utf8_string[arrlen(utf8_string) - 1] == '\0' && arrlen(utf8_string) != 0); \
        i32 num = written + 1; \
        utf8* start = is_terminater_former ? arraddnptr(utf8_string, num) - 1 : arraddnptr(utf8_string, num); \
        memcpy(start, buf, num); \
    } \
} while(0)
