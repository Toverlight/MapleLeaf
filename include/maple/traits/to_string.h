#pragma once

#include <maple/basic_types.h>
#include <stdio.h>
#include <stb_ds.h>

/// var to be processed named 'utf8_string'
/// heap-allocated result, do arrfree() after use!
#define DECLARE_INTERFACE_ToString(type, prefix) \
const utf8* prefix##_to_string_fn(const struct type* obj);

#define IMPL_INTERFACE_ToString_ToString_BEGIN(type, prefix) \
const utf8* prefix##_to_string_fn(const struct type* obj) { \
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
            LOG_ERROR_ONCE(u8"Failed to stringify item because of buffer overflow. Of type '%s', fmt '%s'. Due to that, the stringify processes later are all disabled. Consider to increase the max_bytes arg", type_string, fmt_plain_ptr); \
            able_to_stringify = false; \
            break; \
        } \
        bool is_terminater_former = (utf8_string[arrlen(utf8_string) - 1] == '\0' && arrlen(utf8_string) != 0); \
        utf8* start = is_terminater_former ? arraddnptr(utf8_string, written) - 1 : arraddnptr(utf8_string, written + 1); \
        memcpy(start, buf, written + 1); \
    } \
} while(0)

// stringify an array
// function i32 callback(char* buf, i32 index, usize rest, <type>-><field>* elem);
// callback is applied on each element of the specified arr_field
// !!buf, index, rest, &arr_ptr[i] are all the 'callback' part (i.e. the written assignment function-like block) needs
#define TOSTRING_ARRAY_FMT_ELEM(fmt, ...) snprintf(buf + index, rest, (fmt), ##__VA_ARGS__)
#define TOSTRING_ARRAY_BEGIN(max_bytes, arr_field, len) do { \
    if (able_to_stringify) { \
        usize _max_bytes = max_bytes; \
        typeof(obj->arr_field) arr_ptr = obj->arr_field; \
        usize length = (usize)(len); \
        char buf[_max_bytes]; \
        usize index = snprintf(buf, _max_bytes, #arr_field " [ "); \
        if (index >= _max_bytes) { \
            goto ToStrArray_##arr_field##_Err; \
        } \
        for (usize i = 0; i < length; i++) { \
            usize rest = _max_bytes - index; \
            i32 written = (
#define TOSTRING_ARRAY_END(arr_field) \
            ); \
            if (written >= rest) { \
ToStrArray_##arr_field##_Err: \
                LOG_ERROR_ONCE(u8"Failed to stringify array because of buffer overflow. Of type '%s', field '%s'. Due to that, the stringify processes later are all disabled. Consider to increase the max_bytes arg", type_string, #arr_field); \
                able_to_stringify = false; \
                goto ToStrArray_##arr_field##_End; \
            } \
            index += written; \
        } \
        index += snprintf(buf + index, _max_bytes - index, " ] "); \
        if (index >= _max_bytes) { \
            goto ToStrArray_##arr_field##_Err; \
        } \
        bool is_terminater_former = (utf8_string[arrlen(utf8_string) - 1] == '\0' && arrlen(utf8_string) != 0); \
        utf8* start = is_terminater_former ? arraddnptr(utf8_string, index) - 1 : arraddnptr(utf8_string, index + 1); \
        memcpy(start, buf, index + 1); \
    } \
ToStrArray_##arr_field##_End: \
} while(0)
