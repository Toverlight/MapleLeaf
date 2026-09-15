#include <maple/application.h>
#include <maple/comp_types.h>
#include <maple/event.h>
#include <maple/timer.h>
#include <maple/memory_utils.h>
#include <maple/path_utils.h>

static Application app;

IMPL_INTERFACE_BEGIN(Application, ToString, app, ToString)
    TOSTRING_ITEM(256, u8"Application: name '%s', version '%s', identifier '%s', ", app.name, app.version, app.app_identifier);
    TOSTRING_ITEM(160, u8"real (width %d, height %d), logic (width %d, height %d).", app.w_real, app.h_real, app.w_logic, app.h_logic);
    // TODO (maybe)
IMPL_INTERFACE_END(Application, ToString, app, ToString)

Application* app_get(void) {
    return &app;
}

void maple_app_exit(Application* app) {
    LOG_INFO(u8"Application exiting...");
    print_program_memory();
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
    maple_unreg_timer_all();

    maple_unload_fonts();
    maple_unload_utf8_tiles();

    maple_unload_images();

    maple_clear_paths();
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
