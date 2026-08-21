#pragma once

#define DEFINE_INTERFACE_Default_BEGIN(type, prefix) \
static inline struct type prefix##_default_fn(void) { \
    struct type default = {};
#define DEFINE_INTERFACE_Default_END(type, prefix) \
    return default; \
}
