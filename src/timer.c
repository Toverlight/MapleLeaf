#include <maple/timer.h>
#include <stb_ds.h>
#include <maple/builtin/sdl3_layer.h>

typedef struct {
	const char* key;
	Timer* value;
} TimerEntry;

static TimerEntry* timer_reg = nullptr;

Timer* timer_new_fn(const char* name, i32 rest_count, u64 interval_ns, void* data, TimerCallback callback) {
	// 计时器以名称为键，同名覆盖会让旧计时器的堆内存失去引用（泄漏），且旧指针
	// 一旦被用户缓存就变成悬垂指针。这里保持「注册幂等」：同名则返回已存在的那个。
	isize existed = shgeti(timer_reg, name);
	if (existed >= 0) {
		LOG_WARN_LIMITED(20, u8"Timer named '%s' already exists, return the existent one instead of making a new one", name);
		return timer_reg[existed].value;
	}

	Timer* timer = (Timer*)malloc(sizeof(Timer));
	timer->running = false;
	timer->rest_count = rest_count;
	timer->interval_ns = interval_ns;
	timer->data = data;
	timer->callback = callback;

	shput(timer_reg, name, timer);
	return timer;
}

Timer* timer_get_or_null_fn(const char* name) {
	isize i = shgeti(timer_reg, name);
	if (i >= 0) {
		return timer_reg[i].value;
	}
	LOG_ERROR_LIMITED(100, u8"Timer named '%s' not found", name);
	return nullptr;
}

void timer_set_rest_count_fn(Timer* timer, i32 rest_count) {
	if (timer) timer->rest_count = rest_count;
}

void timer_start_fn(Timer* timer) {
	if (!timer) return;
	timer->running = true;
	timer->start_timepoint_ns = get_ticks_ns();
	timer->start_timepoint_ms = (f64)timer->start_timepoint_ns / RATIO_MS_NS;
	timer->start_timepoint_s = (f64)timer->start_timepoint_ns / RATIO_S_NS;
}

void timer_pause_fn(Timer* timer) {
	if (timer) timer->running = false;
}

void timer_resume_fn(Timer* timer) {
	if (timer) timer->running = true;
}

void timer_stop_fn(Timer* timer) {
	if (!timer) return;
	timer->running = false;
	timer->rest_count = 0;
}

bool timer_delete_fn(const char* name) {
	isize i = shgeti(timer_reg, name);
	if (i >= 0) {
		free(timer_reg[i].value);
		shdel(timer_reg, name);
		return true;
	}
	LOG_WARN_LIMITED(20, u8"Invalid timer deletion: name '%s'", name);
	return false;
}

void timer_update_fn(Timer* timer) {
	if (!timer) return;
	if (timer->running && timer->rest_count != TIMER_STOP) {
		timer->latest_timepoint_ns = get_ticks_ns();
		timer->latest_timepoint_ms = (f64)timer->latest_timepoint_ns / RATIO_MS_NS;
		timer->latest_timepoint_s = (f64)timer->latest_timepoint_ns / RATIO_S_NS;
		if (timer->latest_timepoint_ns - timer->start_timepoint_ns >= timer->interval_ns) {
			if (timer->callback) timer->callback(timer, &timer->interval_ns, timer->data);
			if (timer->rest_count > 0) timer->rest_count--;
			timer->start_timepoint_ns = get_ticks_ns();
			timer->start_timepoint_ms = (f64)timer->start_timepoint_ns / RATIO_MS_NS;
			timer->start_timepoint_s = (f64)timer->start_timepoint_ns / RATIO_S_NS;
		}
	}
}

void maple_unreg_timer_all(void) {
    for (i32 i = 0; i < shlen(timer_reg); i++) {
        free(timer_reg[i].value);
    }
    shfree(timer_reg);
}
