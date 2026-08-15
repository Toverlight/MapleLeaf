#include "sparse_set.h"
#include "comp_types.h"
#include <string.h>

bool ecs_has(i32* sparse_set, Entity e) {
	return sparse_set[e] != -1;
}
bool ecs_ins(CompType* type, const void* data, Entity e) {
    if (ecs_has(type->sparse_set, e)) return false; // 一个实体最多拥有一个同种的组件
	type->sparse_set[e] = type->dense_len;
	void* dst = type->dense_set + type->dense_len * type->comp_size;
	memcpy(dst, data, type->comp_size);
	*((Entity*)dst) = e; // 更新所属实体字段
	type->dense_len++;
	return true;
}
bool ecs_del(CompType* type, Entity e, void* out) {
	if (!ecs_has(type->sparse_set, e)) return false;
	if (out) memcpy(out, type->dense_set + type->sparse_set[e] * type->comp_size, type->comp_size);
	type->dense_len--;
	memcpy(type->dense_set + type->sparse_set[e] * type->comp_size, type->dense_set + type->dense_len * type->comp_size, type->comp_size);
	type->sparse_set[e] = -1;
	return true;
}
