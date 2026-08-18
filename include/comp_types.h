#pragma once
#include "basic_types.h"
#include "sparse_set.h"
#include "tools.h"

typedef u32 CompId;
#define COMP_ID_INVALID 0

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

CompId comp_id_fn(const char* name);
CompId reg_comp_fn(const char* name, usize comp_size);
void maple_unreg_comp_all(void);

#define comp_id(comp_name) ((void)sizeof(comp_name), comp_id_fn(#comp_name))
#define reg_comp(comp_name) ((void)sizeof(comp_name), reg_comp_fn(#comp_name, sizeof(comp_name)))

#define DEFINE_COMP_BEGIN(comp_name) typedef struct comp_name { \
    Entity owner;
#define DEFINE_COMP_END(comp_name) } comp_name;

Entity maple_entity_next(void);
bool maple_entity_despawn(Entity e);

DECLARE_HACKER_COPIED(CompTypeReg, comp_type_reg);
