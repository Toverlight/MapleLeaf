#pragma once

#include "basic_types.h"
#include "stb_ds.h"
#include "event.h"
#include "builtin/resources.h"

struct Application;
typedef void(*PluginFn)(struct Application* app);

typedef struct PluginEntry {
    usize key;
} PluginEntry, *PluginSet;

typedef void(*SystemFn)(void);

typedef enum Schedule : u8 {
	Startup,
	PreUpdate,
	FixedUpdate,
	Update,
	PostUpdate,
	Cleanup,

	Maple_ScheduleMax
} Schedule;

typedef struct SystemSetEntry {
    const char* key;
    const SystemFn* value;
} SystemSetEntry, *SystemSetMap;

typedef struct Application {
	const char* name;
	const char* version;
	int w_real;
	int h_real;
	int w_logic;
	int h_logic;
	PluginFn* plugins;
	PluginSet plugin_all; // 插件去重
	SystemFn** schedules;
	SystemSetMap system_sets;
	Message* next_messages;
	Message* current_messages;
	G_ObserverReg g_observer_reg;
	bool condition;
} Application;

#define APP_START(app_name, version, w_real, h_real, w_logic, h_logic) \
int main(void) { \
	Application* _obj_self = app_get(); \
	*_obj_self = (Application) { (app_name), (version), (w_real), (h_real), (w_logic), (h_logic), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, true }; \
    for (i32 i = 0; i < arrlen(_obj_self->plugins); i++) { \
    	_obj_self->plugins[i](&app); \
    } \
    arrsetlen(_obj_self->schedules, Maple_ScheduleMax); \
    for (i32 i = 0; i < arrlen(_obj_self->schedules); i++) { \
    	_obj_self->schedules[i] = nullptr; \
    } \

#define APP_END() \
maple_app_exit(_obj_self); \
return 0; }

// 获取全局唯一app对象
Application* app_get(void);

void maple_app_exit(Application* app);

void app_add_plugin_fn(Application* app, PluginFn plugin);
#define app_add_plugin(plugin) app_add_plugin_fn(_obj_self, (plugin))

#define SCHEDULE_GLOBAL(phase) \
do { \
	static bool _executed = false; \
	if (!_executed) { \
		_executed = true; \
		SystemFn* systems = _obj_self->schedule[(phase)];\
		for (int i = 0; i < arrlen(systems); i++) { \
			systems[i](); \
		} \
	} \
} while(0)

#define SCHEDULE_FRAME(phase) \
do { \
	SystemFn* systems = _obj_self->schedule[(phase)];\
	for (int i = 0; i < arrlen(systems); i++) { \
		systems[i](); \
	} \
} while(0)

#define SCHEDULE_INTERVAL(phase) \
do { \
	static u64 _lastpoint = get_ticks_ns(); \
	u64 _currentpoint = get_ticks_ns(); \
	SystemFn* systems = _obj_self->schedule[(phase)];\
	while (_currentpoint - _lastpoint > get_fixed_ns()) { \
		for (int i = 0; i < arrlen(systems); i++) { \
			systems[i](); \
		} \
		_lastpoint += get_fixed_ns(); \
	} \
} while(0)

#define MAIN_LOOP(fps) \
do { \
	_obj_self->condition = true; \
	f64 _fps = (f64)(fps); \
	if (_fps < 1) _fps = 165; \
	const u64 nspf = RATIO_S_NS / _fps; \
	res_set(Res_TimeFixed, nspf, nspf); \
	res_set(Res_TimeFixed, mspf, (f64)nspf / RATIO_MS_NS); \
	res_set(Res_TimeFixed, spf, (f64)nspf / RATIO_S_NS); \
	SCHEDULE_GLOBAL(Startup); \
	static u64 last = get_ticks_ns(); \
	while (_obj_self->condition) { \
		u64 current = get_ticks_ns(); \
		u64 delta_ns = current - last; \
		res_set(Res_TimeDelta, dns, delta_ns); \
		res_set(Res_TimeDelta, dms, (f64)delta_ns / RATIO_MS_NS); \
		res_set(Res_TimeDelta, ds, (f64)delta_ns / RATIO_S_NS); \
		SCHEDULE_FRAME(PreUpdate); \
		SCHEDULE_INTERVAL(FixedUpdate); \
		SCHEDULE_FRAME(Update); \
		SCHEDULE_FRAME(PostUpdate); \
		u64 elapsed = get_ticks_ns() - current; \
		if (elapsed < get_fixed_ns()) { \
			DELAY_NS(get_fixed_ns() - elapsed); \
		} \
		last = current; \
	} \
	SCHEDULE_GLOBAL(Cleanup); \
} while(0)

void app_add_system_fn(Application* app, Schedule schedule, SystemFn system);
#define app_add_system(schedule, systemfn) app_add_system_fn(_obj_self, (schedule), (systemfn))

// TODO i >= 0 分支下加上warning: redundant system set named "..."
#define app_system_set(name, ...) \
({ \
	const SystemFn* system_set = (const SystemFn*) { __VA_ARGS__, nullptr }; \
	isize i = shgeti(_obj_self->system_sets, #name); \
	if (i >= 0) { \
        system_set = (const SystemFn*) { nullptr }; \
	} else { \
	    shput(_obj_self->system_sets, #name, system_set); \
	} \
	system_set; \
})
const SystemFn* app_get_system_set_fn(Application* app, const char* name);
#define app_get_system_set(name) app_get_system_set_fn(_obj_self, #name)

void app_add_systems_fn(Application* app, Schedule schedule, const SystemFn* systems);
#define app_add_systems(schedule, systems) app_add_systems_fn(_obj_self, (schedule), (systems))
