#include "application.h"

void maple_app_exit(Application* app) {
    arrfree(app->plugins);
    hmfree(app->plugin_all);
    for (i32 i = 0; i < arrlen(app->schedules); i++) {
    	arrfree(app->schedules[i]);
    }
    arrfree(app->schedules);
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
