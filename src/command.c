#include "command.h"
#include "comp_types.h"

// TODO multi-command configuration & multi-thread support
Command command;

// FIXME 关系加入后改为级联删除
void ecmd_despawn_fn(struct EntityCommand* self, bool* success) {
	if (!self->target) {
		if (success) *success = false;
	}
	if (maple_entity_despawn(self->target)) {
		if (success) *success = true;
	} else {
		if (success) *success = false;
	}
}

struct EntityCommand cmd_spawn_fn(struct Command* self, bool* success) {
	Entity target = maple_entity_next();
	EntityCommand ecmd = { target, self };
	if (!target) {
		// TODO 加一条限定最大次数的警告或报错，因为这种情况会导致后面的链式调用全部失败
		if (success) *success = false;
	} else {
		if (success) *success = true;
	}
	return ecmd;
}
struct EntityCommand cmd_entity_fn(struct Command* self, Entity target, bool* success) {
	EntityCommand ecmd = { target, self };
	if (!target) {
		// TODO 加一条限定最大次数的警告或报错，因为这种情况会导致后面的链式调用全部失败
		if (success) *success = false;
	} else {
		if (success) *success = true;
	}
	return ecmd;
}
