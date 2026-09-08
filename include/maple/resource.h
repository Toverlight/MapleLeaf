#pragma once
#include <string.h>

#define DECLARE_RESOURCE_BEGIN(res_name) typedef struct res_name {
#define DECLARE_RESOURCE_END(res_name) } res_name; \
void maple_res_init_##res_name(res_name res); \
void maple_res_set_##res_name(usize offset, const void* value, usize value_size); \
res_name maple_res_get_##res_name(void); \
const res_name* maple_res_get_addr_##res_name(void);

// TODO warning: 重复调用init警告
#define IMPL_RESOURCE(res_name) \
static res_name res_##res_name; \
static bool initialized_##res_name = false; \
void maple_res_init_##res_name(res_name res) { \
	if (initialized_##res_name) return; \
	res_##res_name = res; \
	initialized_##res_name = true; \
} \
void maple_res_set_##res_name(usize offset, const void* value, usize value_size) { \
	memcpy((void*)&res_##res_name + offset, value, value_size); \
} \
res_name maple_res_get_##res_name(void) { \
	return res_##res_name; \
} \
const res_name* maple_res_get_addr_##res_name(void) { \
	return &res_##res_name; \
}

#define res_set(res_name, field, value) \
do { \
	typeof(((res_name*)0)->field) _tmp = (value); \
	maple_res_set_##res_name(offsetof(res_name, field), &_tmp, sizeof(_tmp)); \
} while(0)
#define res_swap_update(res_name, field, value) \
do { \
	typeof(((res_name*)0)->field) _tmp = (value); \
	typeof(((res_name*)0)->field) _to_be_last_value = res_get(res_name, field); \
	maple_res_set_##res_name(offsetof(res_name, last_##field), &_to_be_last_value, sizeof(_to_be_last_value)); \
	maple_res_set_##res_name(offsetof(res_name, field), &_tmp, sizeof(_tmp)); \
} while(0)
#define res_sync_last(res_name, field) \
do { \
	typeof(((res_name*)0)->field) _to_be_last_value = res_get(res_name, field); \
	maple_res_set_##res_name(offsetof(res_name, last_##field), &_to_be_last_value, sizeof(_to_be_last_value)); \
} while(0)

#define res_get(res_name, field) (maple_res_get_##res_name().field)
#define res_get_full(res_name) maple_res_get_##res_name()
#define res_get_full_addr(res_name) maple_res_get_addr_##res_name()

#define reg_res(res_name, res) maple_res_init_##res_name(res)
