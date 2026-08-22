#pragma once

// object named 'prefix##_df'
#define DEFINE_INTERFACE_Default_BEGIN(type, prefix) \
static inline struct type prefix##_default_fn(void) { \
    struct type prefix##_df = {};
#define DEFINE_INTERFACE_Default_END(type, prefix) \
    return prefix##_df; \
}
