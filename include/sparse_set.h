#pragma once
#include "basic_types.h"
#include <stdbool.h>

// 实体，即ID
typedef u32 Entity;
#define ENTITY_INVALID 0
#define ENTITY_NUM_MAX 1024

struct CompType;
bool ecs_has(i32* sparse_set, Entity e);
bool ecs_ins(struct CompType* type, const void* data, Entity e);
bool ecs_del(struct CompType* type, Entity e, void* out);
