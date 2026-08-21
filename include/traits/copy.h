#pragma once

#define DECLARE_INTERFACE_Copy(type, prefix) \
struct type prefix##_copy_fn(struct type* obj);

#define IMPL_INTERFACE_Copy_Copy_BEGIN(type, prefix) \
struct type prefix##_copy_fn(struct type* obj) { \
    struct type copy = {};
#define IMPL_INTERFACE_Copy_Copy_END(type, prefix) \
    return copy; \
}
