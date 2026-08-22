#pragma once
#include "basic_types.h"
#include "command.h"
#include "tools.h"
#include "stb_ds.h"
#include <string.h>

typedef u32 CompId;
#define COMP_ID_INVALID 0

#define ENTITY_INVALID 0
#define ENTITY_NUM_MAX 1024

typedef struct CompType {
	void* dense_set;
	u32 dense_len;
	i32* sparse_set;
	usize comp_size;
} CompType;
typedef struct CompTypeEntry {
	CompId key;
	CompType value;
} CompTypeEntry, *CompTypeReg;
typedef struct CompNameEntry {
	const char* key;
	CompId value;
} CompNameEntry, *CompNameReg;

static inline bool maple_entity_has(i32* sparse_set, Entity e) {
    return sparse_set[e] != -1;
}
bool maple_entity_insert(struct CompType* type, const void* data, Entity e);
bool maple_entity_remove(struct CompType* type, Entity e, void* out);

CompId comp_id_fn(const char* name);
CompId reg_comp_fn(const char* name, usize comp_size);
void maple_unreg_comp_all(void);

#define comp_id(comp_name) ((void)sizeof(comp_name), comp_id_fn(#comp_name))
#define reg_comp(comp_name) reg_comp_fn(#comp_name, sizeof(comp_name))

Entity maple_entity_next(void);
bool maple_entity_despawn(Entity e);

DECLARE_HACKER_COPIED(CompTypeReg, comp_type_reg);


#define DECLARE_COMP_BEGIN(comp_name) typedef struct comp_name { \
    Entity owner;
#define DECLARE_COMP_END(comp_name) } comp_name; \
bool maple_entity_insert_##comp_name(CompType* type, Entity e, const comp_name* comp); \
bool maple_entity_remove_##comp_name(CompType* type, Entity e, comp_name* out); \
void ecmd_insert_##comp_name(struct EntityCommand* self, CompId comp_id, const comp_name* comp, bool* success); \
void ecmd_remove_##comp_name(struct EntityCommand* self, CompId comp_id, comp_name* out, bool* success);

// TODO 插入失败打印固定次数error日志
#define IMPL_COMP(comp_name) \
bool maple_entity_insert_##comp_name(CompType* type, Entity e, const comp_name* comp) { \
    if (maple_entity_has(type->sparse_set, e)) return false; \
    if (type->comp_size != sizeof(comp_name)) return false; \
    if (type->comp_size == sizeof(Entity) && comp) return false; \
    if (type->comp_size != sizeof(Entity) && !comp) return false; \
    if (type->dense_len == ENTITY_NUM_MAX) return false; \
	type->sparse_set[e] = type->dense_len; \
	if (comp) { \
	    void* dst = type->dense_set + type->dense_len * type->comp_size + sizeof(comp->owner); \
		memcpy(dst, comp, type->comp_size - sizeof(comp->owner)); \
	} \
	((comp_name*)(type->dense_set + type->dense_len * type->comp_size))->owner = e; \
	type->dense_len++; \
	return true; \
} \
bool maple_entity_remove_##comp_name(CompType* type, Entity e, comp_name* out) { \
	if (!maple_entity_has(type->sparse_set, e)) return false; \
    if (type->comp_size != sizeof(comp_name)) return false; \
	if (out) memcpy(out, type->dense_set + type->sparse_set[e] * type->comp_size, type->comp_size); \
	memcpy(type->dense_set + type->sparse_set[e] * type->comp_size, type->dense_set + (type->dense_len - 1) * type->comp_size, type->comp_size); \
	type->sparse_set[e] = -1; \
	type->dense_len--; \
	return true; \
} \
void ecmd_insert_##comp_name(struct EntityCommand* self, CompId comp_id, const comp_name* comp, bool* success) { \
	if (!self->target) { \
		if (success) *success = false; \
		return; \
	} \
	CompTypeReg comp_type_reg = HACKER_COPIED(comp_type_reg); \
	isize i = hmgeti(comp_type_reg, comp_id); \
	if (i >= 0) { \
		CompType* type = &(comp_type_reg[i].value); \
		if (maple_entity_insert_##comp_name(type, self->target, comp)) { \
			if (success) *success = true; \
		} else { \
			if (success) *success = false; \
		} \
	} else { \
		if (success) *success = false; \
	} \
	return; \
} \
void ecmd_remove_##comp_name(struct EntityCommand* self, CompId comp_id, comp_name* out, bool* success) { \
	if (!self->target) { \
		if (success) *success = false; \
		return; \
	} \
	CompTypeReg comp_type_reg = HACKER_COPIED(comp_type_reg); \
	isize i = hmgeti(comp_type_reg, comp_id); \
	if (i >= 0) { \
		CompType* type = &(comp_type_reg[i].value); \
		if (maple_entity_remove_##comp_name(type, self->target, out)) { \
			if (success) *success = true; \
		} else { \
			if (success) *success = false; \
		} \
	} else { \
		if (success) *success = false; \
	} \
}

#define ecmd_insert(comp_name, comp_data, success) ecmd_insert_fn(_obj_self, comp_id(#comp_name), (comp_data), (success))
#define ecmd_remove(comp_name, out, success) ecmd_remove_fn(_obj_self, comp_id(#comp_name), (out), (success))
