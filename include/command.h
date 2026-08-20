#pragma once

#include "basic_types.h"

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

void ecmd_despawn_fn(struct EntityCommand* self, bool* success);
#define ecmd_despawn(success) ecmd_despawn_fn(_obj_self, (success))
struct EntityCommand cmd_spawn_fn(struct Command* self, bool* success);
#define cmd_spawn(self, success) cmd_spawn_fn((self), (success))
struct EntityCommand cmd_entity_fn(struct Command* self, Entity target, bool* success);
#define cmd_entity(self, target, success) cmd_entity_fn((self), (target), (success))
