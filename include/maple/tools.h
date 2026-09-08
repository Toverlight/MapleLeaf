#pragma once
#define field_size_of(type, field) sizeof(((type*)0)->field)

#define DECLARE_HACKER_ADDR(type, var_name) type* maple_hacker_addr_##var_name(void);
#define DECLARE_HACKER_COPIED(type, var_name) type maple_hacker_copied_##var_name(void);
#define IMPL_HACKER_ADDR(type, var_name) \
type* maple_hacker_addr_##var_name(void) { \
    return &var_name; \
}
#define IMPL_HACKER_COPIED(type, var_name) \
type maple_hacker_copied_##var_name(void) { \
    return var_name; \
}
#define HACKER_ADDR(var_name) maple_hacker_addr_##var_name()
#define HACKER_COPIED(var_name) maple_hacker_copied_##var_name()

#define WITH(obj, ...) \
do { \
    typeof(obj)* _obj_self = &(obj); \
    __VA_ARGS__; \
} while (0)

#define WITH_COPIED(obj, ...) \
do { \
    typeof(obj) _obj_self = (obj); \
    __VA_ARGS__; \
} while (0)

#define DECLARE_INTERFACE(type, interface, prefix) DECLARE_INTERFACE_##interface(type, prefix)
#define IMPL_INTERFACE_BEGIN(type, interface, prefix, trait) IMPL_INTERFACE_##interface##_##trait##_BEGIN(type, prefix)
#define IMPL_INTERFACE_END(type, interface, prefix, trait) IMPL_INTERFACE_##interface##_##trait##_END(type, prefix)
#define DEFINE_INTERFACE_BEGIN(type, interface, prefix) DEFINE_INTERFACE_##interface##_BEGIN(type, prefix)
#define DEFINE_INTERFACE_END(type, interface, prefix) DEFINE_INTERFACE_##interface##_END(type, prefix)

#define LAST_FIELD(type, var) \
type var; \
type last_##var

#define SET_LAST_FIELD(obj, field, value) do { \
    (obj).field = value; \
    (obj).last_##field = value; \
} while(0)
#define SET_LAST_FIELD_PTR(obj, field, value) do { \
    (obj)->field = value; \
    (obj)->last_##field = value; \
} while(0)
#define SWAP_UPDATE_LAST_FIELD(obj, field, value) do { \
    (obj).last_##field = (obj).field; \
    (obj).field = (value); \
} while(0)
#define SWAP_UPDATE_LAST_FIELD_PTR(obj, field, value) do { \
    (obj)->last_##field = (obj)->field; \
    (obj)->field = (value); \
} while(0)
#define SYNC_LAST_FIELD(obj, field) do { \
    (obj).last_##field = (obj).field; \
} while(0)
#define SYNC_LAST_FIELD_PTR(obj, field) do { \
    (obj)->last_##field = (obj)->field; \
} while(0)
