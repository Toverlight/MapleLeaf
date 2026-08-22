#pragma once

#include <maple/basic_types.h>

// 'cmd' prefix
typedef struct Command {
	// ...
} Command;
// 'ecmd' prefix
typedef struct EntityCommand {
	Entity target;
	struct Command* cmd;
} EntityCommand;

extern Command command;

#define COMMAND() (&command)

struct EntityCommand cmd_spawn_fn(struct Command* self, bool* success);
#define cmd_spawn(self, success) cmd_spawn_fn((self), (success))
struct EntityCommand cmd_entity_fn(struct Command* self, Entity target, bool* success);
#define cmd_entity(self, target, success) cmd_entity_fn((self), (target), (success))

typedef struct Filiation {
	Entity parent;
	Entity* children;
} Filiation;
typedef struct FiliationEntry {
	Entity key;
	Filiation value;
} FiliationEntry, *FiliationReg;

void ecmd_with_child_fn(struct EntityCommand* self, Entity child, bool* success);
#define ecmd_with_child(success, ...) \
do { \
	EntityCommand* _obj_parent = _obj_self; \
	EntityCommand _obj_ecmd = cmd_spawn(_obj_parent->cmd, (success)); \
	EntityCommand* _obj_self = &_obj_ecmd; \
	__VA_ARGS__; \
	ecmd_with_child_fn(_obj_parent, _obj_self->target, (success)); \
} while(0)

void maple_entity_test_filiation_fn(Entity self);
void maple_entity_set_parent_fn(Entity self, Entity parent);

void ecmd_despawn_fn(struct EntityCommand* self, bool* success);
#define ecmd_despawn(success) ecmd_despawn_fn(_obj_self, (success))
