#include "application.h"
#include "comp_types.h"
#include "event.h"

static Application app;

Application* app_get(void) {
    return &app;
}

void maple_app_exit(Application* app) {
    arrfree(app->plugins);
    hmfree(app->plugin_all);
    for (i32 i = 0; i < arrlen(app->schedules); i++) {
    	arrfree(app->schedules[i]);
    }
    arrfree(app->schedules);
    for (i32 i = 0; i < hmlen(app->system_sets); i++) {
        arrfree(app->system_sets[i].value);
    }
    hmfree(app->system_sets);
    arrfree(app->next_messages);
    arrfree(app->current_messages);
    for (i32 i = 0; i < hmlen(app->g_observer_reg); i++) {
        arrfree(app->g_observer_reg[i].value);
    }
    hmfree(app->g_observer_reg);
    maple_unreg_comp_all();
    maple_unreg_msg_all();
    maple_unreg_e_observer_all();
    // TODO 可以统计结束时刻占用空间最后输出
}

void app_add_plugin_fn(Application* app, PluginFn plugin) {
    isize i = hmgeti(app->plugin_all, (usize)plugin);
    if (i >= 0) return;
	arrput(app->plugins, plugin);
	hmputs(app->plugin_all, (PluginEntry) {(usize)plugin});
}

void app_add_system_fn(Application* app, Schedule schedule, SystemFn system) {
	if (!system) return;
	arrput(app->schedules[schedule], system);
}

const SystemFn* app_get_system_set_fn(Application* app, const char* name) {
    isize i = shgeti(app->system_sets, name);
    if (i >= 0) {
        return app->system_sets[i].value;
    }
    return nullptr;
}

void app_add_systems_fn(Application* app, Schedule schedule, const SystemFn* systems) {
   	if (!systems) return;
	for (i32 i = 0; systems[i] != nullptr; i++) {
		arrput(app->schedules[schedule], systems[i]);
	}
}
