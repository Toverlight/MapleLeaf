#include <maple/command.h>
#include <maple/comp_types.h>
#include <maple/builtin/sdl3_layer.h>

// TODO multi-command configuration & multi-thread support
Command command;

struct EntityCommand cmd_spawn_fn(struct Command* self, bool* success) {
	Entity target = maple_entity_next();
	EntityCommand ecmd = { target, self };
	if (!target) {
		LOG_WARN_LIMITED(100, u8"Target entity spawned as invalid. It can cause continuous failures after this call in the chain calls");
		if (success) *success = false;
	} else {
		if (success) *success = true;
	}
	return ecmd;
}
struct EntityCommand cmd_entity_fn(struct Command* self, Entity target, bool* success) {
	EntityCommand ecmd = { target, self };
	if (!target) {
		LOG_WARN_LIMITED(100, u8"Target entity spawned as invalid. It can cause continuous failures after this call in the chain calls");
		if (success) *success = false;
	} else {
		if (success) *success = true;
	}
	return ecmd;
}

static FiliationReg filiation_reg = nullptr;

void ecmd_with_child_fn(struct EntityCommand* self, Entity child, bool* success) {
	if (!self->target || !child) {
		if (success) *success = false;
		return;
	}
	// 添加子
	isize p_i = hmgeti(filiation_reg, self->target);
	if (p_i < 0) {
		Filiation filiation = { 0, nullptr };
		arrput(filiation.children, child);
		hmput(filiation_reg, self->target, filiation);
	} else {
		arrput(filiation_reg[p_i].value.children, child);
	}
	// 指向父
	isize c_i = hmgeti(filiation_reg, self->target);
	if (c_i < 0) {
		Filiation filiation = { self->target, nullptr };
		hmput(filiation_reg, child, filiation);
	} else {
		filiation_reg[c_i].value.parent = self->target;
	}
	if (success) *success = true;
}

// 检查filiation存在必要性，即若无parent且children为空，则该项无存在必要，删除该项
void maple_entity_test_filiation_fn(Entity self) {
	isize i = hmgeti(filiation_reg, self);
	if (i >= 0) {
		if (!filiation_reg[i].value.parent
			&& !arrlen(filiation_reg[i].value.children))
		{
			arrfree(filiation_reg[i].value.children);
			hmdel(filiation_reg, self);
		}
	}
}
void maple_entity_set_parent_fn(Entity self, Entity parent) {
	if (!self) return;
	// self项
	isize i = hmgeti(filiation_reg, self);
	if (i >= 0) {
	Proc:
		// par_old项
		Entity par_old = filiation_reg[i].value.parent;
		if (par_old == parent) return;
		if (par_old) {
			isize j = hmgeti(filiation_reg, par_old);
			if (j >= 0) {
				for (isize i = 0; i < arrlen(filiation_reg[j].value.children); i++) {
					if (filiation_reg[j].value.children[i] == self) {
						arrdel(filiation_reg[j].value.children, i);
						break;
					}
				}
				maple_entity_test_filiation_fn(par_old);
			}
		}
		// parent项
		filiation_reg[i].value.parent = parent;
		maple_entity_test_filiation_fn(self);
		if (parent) {
			isize k = hmgeti(filiation_reg, parent);
			if (k >= 0) {
				bool existent = false;
				for (isize i = 0; i < arrlen(filiation_reg[k].value.children); i++) {
					if (filiation_reg[k].value.children[i] == self) {
						existent = true;
						break;
					}
				}
				if (!existent)
					arrput(filiation_reg[k].value.children, self);
			}
		}
	} else {
		Filiation filiation = { ENTITY_INVALID, nullptr };
		hmput(filiation_reg, self, filiation);
		goto Proc;
	}
}

void ecmd_despawn_fn(struct EntityCommand* self, bool* success) {
    if (!self->target) {
		if (success) *success = false;
	}
	// 初始化arr
	Entity* arr = nullptr;
	usize cur = 0;
	maple_entity_set_parent_fn(self->target, ENTITY_INVALID);
	arrput(arr, self->target);
	bool failed = false;
	do {
		// 单体删除
		if (!maple_entity_despawn(arr[cur])) {
			failed = true;
			LOG_ERROR_LIMITED(100, u8"Entity '%d' failed to be despawned", self->target);
		}
		isize i = hmgeti(filiation_reg, arr[cur]);
		if (i >= 0) {
			// 添加子实体id到arr
			for (isize j = 0; j < arrlen(filiation_reg[i].value.children); j++) {
				arrput(arr, filiation_reg[i].value.children[j]);
			}
			// 释放children
			arrfree(filiation_reg[i].value.children);
			// 删除该实体键
			hmdel(filiation_reg, self->target);
		}
		cur++;
	} while (cur < arrlen(arr));
	// 释放arr
	arrfree(arr);

	if (failed) {
		if (success) *success = false;
	} else {
		if (success) *success = true;
	}
}
