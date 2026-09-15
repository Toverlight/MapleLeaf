#include <maple/event.h>
#include <maple/application.h>

static MsgNameReg msg_name_reg = nullptr;

void maple_unreg_msg_all(void) {
    hmfree(msg_name_reg);
}
// 带缓存机制的消息ID生成器
static MsgId gen_msg_id(const char* name) {
	static MsgId msg_id_counter = 1;
	isize i = shgeti(msg_name_reg, name);
	if (i >= 0) {
		return msg_name_reg[i].value;
	}
	shput(msg_name_reg, name, msg_id_counter);
	return msg_id_counter++; // TODO 如果有上限限制，加超出上限判断，若超出，返回非法值MSG_ID_INVALID
}

MsgId msg_id_fn(const char* name) {
    isize i = shgeti(msg_name_reg, name);
	if (i >= 0) {
		return msg_name_reg[i].value;
	}
	return MSG_ID_INVALID;
}

MsgId reg_msg_fn(const char* name) {
    MsgId id = msg_id_fn(name);
    if (id != MSG_ID_INVALID) [[clang::unlikely]] {
        LOG_WARN_LIMITED(20, u8"Redundant register action for message type '%s'", name);
        return id;
    }
    id = gen_msg_id(name);
    if (id == MSG_ID_INVALID) [[clang::unlikely]] {
        LOG_ERROR_LIMITED(20, u8"Failed to generate message type id: of name '%s'. Be careful of the probably existing id num limit", name);
        return MSG_ID_INVALID;
    }
    return id;
}

Message message_receive_fn(struct Application* app, const char* message_name) {
	MsgId msg_id = msg_id_fn(message_name);
	if (msg_id == MSG_ID_INVALID) goto End;
	for (i32 i = 0; i < arrlen(app->current_messages); i++) {
		if (app->current_messages[i].msg_id != MSG_ID_INVALID
			&& app->current_messages[i].msg_id == msg_id) {
			Message msg = app->current_messages[i];
			app->current_messages[i].msg_id = MSG_ID_INVALID; // 设为非法值，即将队列中的消息设为无效。
			return msg;
		}
	}
	End:
	return (Message) { .source = 0, .msg_id = MSG_ID_INVALID, .data = nullptr };
}

void signal_add_global_observer_fn(Application* app, SigId sig_id, G_ObserverFn g_fn) {
	if (sig_id == SIG_INVALID) {
	    LOG_WARN_LIMITED(20, u8"Invalid signal type id");
		return;
	}
	isize i = hmgeti(app->g_observer_reg, sig_id);
	if (i >= 0) {
		arrput(app->g_observer_reg[i].value, g_fn);
	} else {
		G_ObserverFn* g_fns = nullptr;
		arrput(g_fns, g_fn);
		hmput(app->g_observer_reg, sig_id, g_fns);
	}
}

static E_ObserverReg e_observer_reg = nullptr;

void maple_unreg_e_observer_all(void) {
    for (isize i = 0; i < hmlen(e_observer_reg); i++) {
        for (isize j = 0; j < hmlen(e_observer_reg[i].value); j++) {
            arrfree(e_observer_reg[i].value[j].value);
        }
        hmfree(e_observer_reg[i].value);
    }
    hmfree(e_observer_reg);
    e_observer_reg = nullptr;
}

/// 解除某实体在所有信号上的观察者绑定。必须在实体销毁前调用：实体 id 会被
/// 复用，残留的观察者会被下一个占用该 id 的实体错误地继承。
    void maple_remove_e_observers_of_fn(Entity entity) {
    if (!entity || !e_observer_reg) return;
    isize removed = 0;
    for (isize i = 0; i < hmlen(e_observer_reg); i++) {
        E_ObserverEntry* e_observers = e_observer_reg[i].value;
        isize j = hmgeti(e_observers, entity);
        if (j < 0) continue;
        arrfree(e_observers[j].value);
        hmdel(e_observers, entity);
        removed++;
    }
    if (!removed) return;
    // 顺带回收已经没有任何实体绑定的信号条目，避免注册表随销毁不断膨胀
    for (isize i = hmlen(e_observer_reg) - 1; i >= 0; i--) {
        if (hmlen(e_observer_reg[i].value) == 0) {
            SigId sig_id = e_observer_reg[i].key;
            hmfree(e_observer_reg[i].value);
            hmdel(e_observer_reg, sig_id);
        }
    }
    DLOG_LIMITED(50, u8"Removed %lld entity observer binding(s) of entity '%u'", removed, entity);
}

void signal_add_entity_observer_fn(Entity entity, SigId sig_id, E_ObserverFn e_fn) {
	if (sig_id == SIG_INVALID) {
	    LOG_WARN_LIMITED(20, u8"Invalid signal type id");
		return;
	}
	isize i = hmgeti(e_observer_reg, sig_id);
	if (i >= 0) {
		isize j = hmgeti(e_observer_reg[i].value, entity);
		if (j >= 0) {
			arrput(e_observer_reg[i].value[j].value, e_fn);
		} else {
			E_ObserverFn* e_fns = NULL;
			arrput(e_fns, e_fn);
			hmput(e_observer_reg[i].value, entity, e_fns);
		}
	} else {
		E_ObserverFn* e_fns = NULL;
		arrput(e_fns, e_fn);
		E_ObserverEntry* e_observers = NULL;
		hmput(e_observers, entity, e_fns);
		hmput(e_observer_reg, sig_id, e_observers);
	}
}

void signal_global_trigger_fn(Application* app, Signal signal) {
	if (signal.sig_id == SIG_INVALID) {
	    LOG_WARN_ONCE(u8"Invalid signal type id");
		return;
	}
	// 全局挂载触发
	do {
		isize i = hmgeti(app->g_observer_reg, signal.sig_id);
		if (i >= 0) {
			G_ObserverFn* g_fns = app->g_observer_reg[i].value;
			for (i32 j = 0; j < arrlen(g_fns); j++) {
				g_fns[j](app, signal);
			}
		}
	} while(0);
	// 实体挂载触发
	do {
		isize i = hmgeti(e_observer_reg, signal.sig_id);
		if (i >= 0) {
			E_ObserverEntry* e_observers = e_observer_reg[i].value;
			for (i32 j = 0; j < hmlen(e_observers); j++) {
				Entity entity = e_observers[j].key;
				E_ObserverFn* e_fns = e_observers[j].value;
				for (i32 k = 0; k < arrlen(e_fns); k++) {
					e_fns[k](entity, signal);
				}
			}
		}
	} while(0);
}

void signal_entity_trigger_fn(Entity entity, Signal signal) {
	if (signal.sig_id == SIG_INVALID) {
	    LOG_WARN_ONCE(u8"Invalid signal type id");
		return;
	}
	isize i = hmgeti(e_observer_reg, signal.sig_id);
	if (i >= 0) {
		isize j = hmgeti(e_observer_reg[i].value, entity);
		if (j >= 0) {
			E_ObserverFn* e_fns = e_observer_reg[i].value[j].value;
			for (i32 k = 0; k < arrlen(e_fns); k++) {
				e_fns[k](entity, signal);
			}
		}
	}
}
