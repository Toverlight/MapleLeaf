#include <maple/comp_types.h>
#include <stdlib.h>
#include <maple/builtin/sdl3_layer.h>

bool maple_entity_insert(CompType* type, const void* data, Entity e) {
    if (maple_entity_has(type->sparse_set, e)) return false; // 一个实体最多拥有一个同种的组件
	type->sparse_set[e] = type->dense_len;
	void* dst = type->dense_set + type->dense_len * type->comp_size;
	memcpy(dst, data, type->comp_size);
	*((Entity*)dst) = e; // 更新所属实体字段
	type->dense_len++;
	return true;
}
bool maple_entity_remove(CompType* type, Entity e, void* out) {
	if (!maple_entity_has(type->sparse_set, e)) return false;
	if (out) memcpy(out, type->dense_set + type->sparse_set[e] * type->comp_size, type->comp_size);
	type->free_fn(type->dense_set + type->sparse_set[e] * type->comp_size);
	type->dense_len--;
	memcpy(type->dense_set + type->sparse_set[e] * type->comp_size, type->dense_set + type->dense_len * type->comp_size, type->comp_size);
	type->sparse_set[e] = -1;
	return true;
}

static CompTypeReg comp_type_reg = nullptr;
IMPL_HACKER_COPIED(CompTypeReg, comp_type_reg);
static CompNameReg comp_name_reg = nullptr;

// 带缓存机制的组件ID生成器
static CompId gen_comp_id(const char* name) {
	static CompId comp_type_id_counter = 1;
	isize i = shgeti(comp_name_reg, name);
	if (i >= 0) {
		return comp_name_reg[i].value;
	}
	shput(comp_name_reg, name, comp_type_id_counter);
	return comp_type_id_counter++; // TODO 如果有上限限制，加超出上限判断，若超出，返回非法值COMP_ID_INVALID
}

CompId comp_id_fn(const char* name) {
    isize i = shgeti(comp_name_reg, name);
	if (i >= 0) {
		return comp_name_reg[i].value;
	}
	return COMP_ID_INVALID;
}

CompId reg_comp_fn(const char* name, usize comp_size, CompFreeFn free_fn) {
    if (!comp_size) [[clang::unlikely]] {
        LOG_ERROR(u8"For component '%s': arg comp_size cannot be 0", name);
        return COMP_ID_INVALID;
    }
    CompId id = comp_id_fn(name);
    if (id != COMP_ID_INVALID) [[clang::unlikely]] {
        LOG_WARN(u8"Attempted to register same component '%s' once more", name);
        return id;
    }
    id = gen_comp_id(name);
    if (id == COMP_ID_INVALID) [[clang::unlikely]] {
        LOG_ERROR(u8"Failed to generate comp id for component '%s' (check if there's a id bound limit which is surpassed)", name);
        return COMP_ID_INVALID;
    }
	isize i = hmgeti(comp_type_reg, id);
	if (i < 0) {
	    CompType type = {
			.dense_set = malloc(ENTITY_NUM_MAX * comp_size),
			.dense_len = 0,
			.sparse_set = malloc(ENTITY_NUM_MAX * sizeof(i32)),
			.comp_size = comp_size,
			.free_fn = free_fn
		};
		memset(type.sparse_set, -1, ENTITY_NUM_MAX * sizeof(i32));
		hmput(comp_type_reg, id, type);
	} else [[clang::unlikely]] {
		LOG_ERROR(u8"Wrongly occupied new comp id, for '%s'", name);
	}
	return id;
}

void maple_unreg_comp_all(void) {
    for (isize i = 0; i < hmlen(comp_type_reg); i++) {
        for (isize j = 0; j < comp_type_reg[i].value.dense_len; j++) {
            comp_type_reg[i].value.free_fn(comp_type_reg[i].value.dense_set + j * comp_type_reg[i].value.comp_size);
        }
        free(comp_type_reg[i].value.dense_set);
        comp_type_reg[i].value.dense_set = nullptr;
        free(comp_type_reg[i].value.sparse_set);
        comp_type_reg[i].value.sparse_set = nullptr;
    }
    hmfree(comp_type_reg);
    hmfree(comp_name_reg);
    // TODO 堆区 entity pool 的内存回收
}

// TODO 之后改成动态数组。改动ENTITY_NUM_MAX时尤其注意不要爆栈
static Entity entity_available_pool[ENTITY_NUM_MAX]; // 空闲实体栈。TODO 初始化
static usize next_available = 0; // 栈顶

void maple_entity_pool_init(void) {
    for (isize i = 0; i < ENTITY_NUM_MAX; i++) {
        entity_available_pool[i] = i + 1;
    }
}

// TODO pool 的free函数

Entity maple_entity_next(void) {
    if (next_available == ENTITY_NUM_MAX) [[clang::unlikely]] {
         return ENTITY_INVALID;
    }
    return entity_available_pool[next_available++];
}

bool maple_entity_despawn(Entity e) {
    for (isize i = 0; i < hmlen(comp_type_reg); i++) {
        CompType* type = &comp_type_reg[i].value;
        maple_entity_remove(type, e, nullptr);
    }
	if (next_available > 0) [[clang::likely]] {
		entity_available_pool[--next_available] = e; // 回收实体
		return true;
	}
	LOG_ERROR_LIMITED(100, u8"Failed to reclaim entity '%d', whose id is unexpectedly redundant", e);
	return false;
}
