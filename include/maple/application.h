#pragma once

#include <maple/basic_types.h>
#include <stb_ds.h>
#include <maple/event.h>
#include <maple/builtin/resources.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_ttf.h>
#include <maple/builtin/sdl3_layer.h>
#include <maple/traits/to_string.h>
#include <maple/path_utils.h>

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
	Render,

	Maple_ScheduleMax
} Schedule;

typedef struct SystemSetEntry {
    const char* key;
    const SystemFn* value;
} SystemSetEntry, *SystemSetMap;

typedef struct Application {
	const char* name;
	const char* version;
	const char* app_identifier;
	i32 w_real;
	i32 h_real;
	i32 w_logic;
	i32 h_logic;

	WindowHandle window;
	RendererHandle renderer;

	PluginFn* plugins;
	PluginSet plugin_all; // 插件去重
	SystemFn** schedules;
	SystemSetMap system_sets;
	Message* next_messages;
	Message* current_messages;
	G_ObserverReg g_observer_reg;
	bool condition; // 运行条件
} Application;

DECLARE_INTERFACE(Application, ToString, app);

#define APP_START(app_name, version, app_identifier, w_real, h_real, w_logic, h_logic) \
int main(void) { \
    SDL_SetAppMetadata((app_name), (version), (app_identifier)); \
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) { \
        fprintf(stdout, "ERROR: SDL Init failed: %s", SDL_GetError()); \
	} \
	if (!TTF_Init()) { \
	    fprintf(stdout, "ERROR: TTF Init failed: %s", SDL_GetError()); \
	} \
	log_init(ASSET(DEMO_NAME u8"_log.txt")); \
	maple_entity_pool_init(); \
	WindowHandle window; \
	RendererHandle renderer; \
	SDL_CreateWindowAndRenderer((app_name), (w_real), (h_real), SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY, &window, &renderer); \
	SDL_SetRenderLogicalPresentation(renderer, (w_logic), (h_logic), SDL_LOGICAL_PRESENTATION_LETTERBOX); \
	Application* _obj_self = app_get(); \
	*_obj_self = (Application) { (app_name), (version), (app_identifier), (w_real), (h_real), (w_logic), (h_logic), (window), (renderer), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, true }; \
	const utf8* _meta_str = app_to_string_fn(_obj_self); \
    LOG_INFO(u8"Application started. %s", (const char*)_meta_str); \
    arrfree(_meta_str); \
    arrsetlen(_obj_self->schedules, Maple_ScheduleMax); \
    for (i32 i = 0; i < arrlen(_obj_self->schedules); i++) { \
    	_obj_self->schedules[i] = nullptr; \
    } \

#define APP_END() \
maple_app_exit(_obj_self); \
SDL_DestroyRenderer(_obj_self->renderer); \
SDL_DestroyWindow(_obj_self->window); \
log_quit(); \
TTF_Quit(); \
SDL_Quit(); \
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
		SystemFn* systems = _obj_self->schedules[(phase)];\
		for (int i = 0; i < arrlen(systems); i++) { \
			systems[i](); \
		} \
	} \
} while(0)

#define SCHEDULE_FRAME(phase) \
do { \
	SystemFn* systems = _obj_self->schedules[(phase)];\
	for (int i = 0; i < arrlen(systems); i++) { \
		systems[i](); \
	} \
} while(0)

#define SCHEDULE_INTERVAL(phase) \
do { \
    static bool _t0 = true; \
    static u64 _lastpoint = 0; \
    if (_t0) { _t0 = false; _lastpoint = get_ticks_ns(); } \
	u64 _currentpoint = get_ticks_ns(); \
	SystemFn* systems = _obj_self->schedules[(phase)];\
	while (_currentpoint - _lastpoint > get_fixed_ns()) { \
		for (int i = 0; i < arrlen(systems); i++) { \
			systems[i](); \
		} \
		_lastpoint += get_fixed_ns(); \
	} \
} while(0)

#define MAIN_LOOP(fps) \
do { \
    for (i32 i = 0; i < arrlen(_obj_self->plugins); i++) { \
        DLOG(u8"plugin %p start...", _obj_self->plugins[i]); \
    	_obj_self->plugins[i](_obj_self); \
        DLOG(u8"plugin %p end.", _obj_self->plugins[i]); \
    } \
	_obj_self->condition = true; \
	f64 _fps = (f64)(fps); \
	if (_fps < 1) _fps = 165; \
	const u64 nspf = RATIO_S_NS / _fps; \
	res_set(Res_TimeFixed, nspf, nspf); \
	res_set(Res_TimeFixed, mspf, (f64)nspf / RATIO_MS_NS); \
	res_set(Res_TimeFixed, spf, (f64)nspf / RATIO_S_NS); \
	DLOG(u8"nspf: %llu, mspf: %.2f, spf: %.3f", res_get(Res_TimeFixed, nspf), res_get(Res_TimeFixed, mspf), res_get(Res_TimeFixed, spf)); \
	SCHEDULE_GLOBAL(Startup); \
	u64 last = get_ticks_ns(); \
	while (_obj_self->condition) { \
		u64 current = get_ticks_ns(); \
		u64 delta_ns = current - last; \
		res_set(Res_TimeDelta, dns, delta_ns); \
		res_set(Res_TimeDelta, dms, (f64)delta_ns / RATIO_MS_NS); \
		res_set(Res_TimeDelta, ds, (f64)delta_ns / RATIO_S_NS); \
	    static u64 frame = 0; \
		static u64 last_cp_ns = 0; \
		if (frame == 0) last_cp_ns = current; \
		frame++; \
		if (frame == 600) { \
		    DLOG(u8"Average milliseconds per frame: %.2f", (f64)(current - last_cp_ns) / (600 * RATIO_MS_NS)); \
		    frame = 0; \
		} \
		DLOG_LIMITED(3, u8"delta_ns: %llu", delta_ns); \
		SCHEDULE_FRAME(PreUpdate); \
		SCHEDULE_INTERVAL(FixedUpdate); \
		SCHEDULE_FRAME(Update); \
		SCHEDULE_FRAME(PostUpdate); \
		SCHEDULE_FRAME(Render); \
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

#define app_system_set(name, ...) \
({ \
	const SystemFn* system_set = (const SystemFn*) { __VA_ARGS__, nullptr }; \
	isize i = shgeti(_obj_self->system_sets, #name); \
	if (i >= 0) { \
        system_set = (const SystemFn*) { nullptr }; \
        LOG_WARN_LIMITED(50, u8"Redundant system set named '%s'", #name); \
	} else { \
	    shput(_obj_self->system_sets, #name, system_set); \
	} \
	system_set; \
})
const SystemFn* app_get_system_set_fn(Application* app, const char* name);
#define app_get_system_set(name) app_get_system_set_fn(_obj_self, #name)

void app_add_systems_fn(Application* app, Schedule schedule, const SystemFn* systems);
#define app_add_systems(schedule, systems) app_add_systems_fn(_obj_self, (schedule), (systems))
