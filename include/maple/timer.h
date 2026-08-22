#pragma once

#include <maple/builtin/resources.h>

struct Timer;
typedef void(*TimerCallback)(const struct Timer* self, u64* interval_ns, void* data);
typedef struct Timer {
	bool running; // “运行中”状态
	i32 rest_count; // 剩余触发次数。小于0时是无限，0时停止，大于0时预期还会触发该次数
	u64 interval_ns; // 设置的间隔。值可以很小（到0），此时每帧都触发
	u64 start_timepoint_ns; // “启动”或“触发”时更新
	f64 start_timepoint_ms; // ms等价辅助字段
	f64 start_timepoint_s; // s等价辅助字段
	u64 latest_timepoint_ns; // 每次更新计时器时更新
	f64 latest_timepoint_ms; // ms等价辅助字段
	f64 latest_timepoint_s; // s等价辅助字段
	void* data; // 用户自定义数据
	TimerCallback callback;
} Timer;

#define TIMER_INFINITY -1
#define TIMER_STOP 0

Timer* timer_new_fn(const char* name, i32 rest_count, u64 interval_ns, void* data, TimerCallback callback);
Timer* timer_get_or_null_fn(const char* name);
void timer_set_rest_count_fn(Timer* timer, i32 rest_count);
void timer_start_fn(Timer* timer);
void timer_pause_fn(Timer* timer);
void timer_resume_fn(Timer* timer);
void timer_stop_fn(Timer* timer);
bool timer_delete_fn(const char* name);
void timer_update_fn(Timer* timer);
void maple_unreg_timer_all(void);
