#include "comp_types.h"
#include "sparse_set.h"
#include "stb_ds.h"
#include <stdlib.h>

static CompTypeReg comp_type_reg = NULL;
static CompNameReg comp_name_reg = NULL;

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
    CompId id = gen_comp_id(name);
	ptrdiff_t i = hmgeti(comp_type_reg, id);
	if (i < 0) {
	    CompType type = {
			.dense_set = malloc(ENTITY_NUM_MAX * comp_size),
			.dense_len = 0,
			.sparse_set = malloc(ENTITY_NUM_MAX * sizeof(i32)),
			.comp_size = comp_size
		};
		hmput(comp_type_reg, id, type);
	}
	return id;
}
void maple_unreg_comp_all(void) {
    for (isize i = 0; i < hmlen(comp_type_reg); i++) {
        free(comp_type_reg[i].value.dense_set);
        comp_type_reg[i].value.dense_set = NULL;
        free(comp_type_reg[i].value.sparse_set);
        comp_type_reg[i].value.sparse_set = NULL;
    }
}
