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
