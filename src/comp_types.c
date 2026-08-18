#include "comp_types.h"
#include "sparse_set.h"
#include "stb_ds.h"
#include <stdlib.h>

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

CompId reg_comp_fn(const char* name, usize comp_size) {
    if (!comp_size) [[clang::unlikely]] {
        // TODO error: comp_size 不可以为 0
        return COMP_ID_INVALID;
    }
    CompId id = comp_id_fn(name);
    if (id != COMP_ID_INVALID) [[clang::unlikely]] {
        // TODO warning: 重复注册相同组件
        return id;
    }
    id = gen_comp_id(name);
    if (id == COMP_ID_INVALID) [[clang::unlikely]] {
        // TODO error: 组件id生成失败（可能是超出限制所致。如果有限制的话）
        return COMP_ID_INVALID;
    }
	isize i = hmgeti(comp_type_reg, id);
	if (i < 0) {
	    CompType type = {
			.dense_set = malloc(ENTITY_NUM_MAX * comp_size),
			.dense_len = 0,
			.sparse_set = malloc(ENTITY_NUM_MAX * sizeof(i32)),
			.comp_size = comp_size
		};
		hmput(comp_type_reg, id, type);
	} else [[clang::unlikely]] {
	    // TODO error: 被错误占用的新组件id
	}
	return id;
}

void maple_unreg_comp_all(void) {
    for (isize i = 0; i < hmlen(comp_type_reg); i++) {
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

Entity maple_entity_next(void) {
    if (next_available == ENTITY_NUM_MAX) [[clang::unlikely]] {
         return ENTITY_INVALID;
    }
    return entity_available_pool[next_available++];
}

bool maple_entity_despawn(Entity e) {
    for (isize i = 0; i < hmlen(comp_type_reg); i++) {
        CompType* type = &comp_type_reg[i].value;
        ecs_del(type, e, nullptr);
    }
	if (next_available > 0) [[clang::likely]] {
		entity_available_pool[--next_available] = e; // 回收实体
		return true;
	}
	// TODO error:【固定次数日志】实体回收失败，预料之外的多余实体id
	return false;
}
