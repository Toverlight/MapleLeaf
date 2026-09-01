#pragma once

#include <maple/resource.h>
#include <maple/basic_types.h>
#include <maple/tools.h>
#include <maple/traits/default.h>
#include <SDL3/SDL.h>

DECLARE_RESOURCE_BEGIN(Res_TimeFixed)
	u64 nspf;
	f64 mspf;
	f64 spf;
DECLARE_RESOURCE_END(Res_TimeFixed)
DEFINE_INTERFACE_BEGIN(Res_TimeFixed, Default, res_time_fixed)
    res_time_fixed_df.nspf = 0;
    res_time_fixed_df.mspf = 0.0f;
    res_time_fixed_df.spf = 0.0f;
DEFINE_INTERFACE_END(Res_TimeFixed, Default, res_time_fixed)

DECLARE_RESOURCE_BEGIN(Res_TimeDelta)
    u64 dns;
	f64 dms;
	f64 ds;
DECLARE_RESOURCE_END(Res_TimeDelta)
DEFINE_INTERFACE_BEGIN(Res_TimeDelta, Default, res_time_delta)
    res_time_delta_df.dns = 0;
    res_time_delta_df.dms = 0.0f;
    res_time_delta_df.ds = 0.0f;
DEFINE_INTERFACE_END(Res_TimeDelta, Default, res_time_delta)

#define get_fixed_ns() res_get(Res_TimeFixed, nspf)
#define get_fixed_ms() res_get(Res_TimeFixed, mspf)
#define get_fixed_s() res_get(Res_TimeFixed, spf)

#define get_delta_ns() res_get(Res_TimeDelta, dns)
#define get_delta_ms() res_get(Res_TimeDelta, dms)
#define get_delta_s() res_get(Res_TimeDelta, ds)

// TODO 获取时间失败error日志
#define get_current_ns() \
({ \
    SDL_Time ticks = {}; \
    if (!SDL_GetCurrentTime(&ticks)) { \
         \
    } \
    ticks; \
})
#define get_current_ms() (get_current_ns() / RATIO_MS_NS)
#define get_current_s() (get_current_ns() / RATIO_S_NS)

#define get_ticks_ns() (SDL_GetTicksNS())
#define get_ticks_ms() (get_ticks_ns() / RATIO_MS_NS)
#define get_ticks_s() (get_ticks_ns() / RATIO_S_NS)

#define RATIO_MS_NS 1'000ll
#define RATIO_S_NS 1'000'000'000ll

#define DELAY_NS(ns) SDL_DelayPrecise(ns)


typedef enum : u8 {
    MouseButton_Left,
    MouseButton_Right,
    MouseButton_Middle,

    Maple_MouseButtonMax
} MouseButton;

// TODO *,last_*的交换更新。待接入SDL
DECLARE_RESOURCE_BEGIN(Res_InputMouse)
    f32 x;
    f32 y;
    f32 last_x;
    f32 last_y;
    i32 wheel;
    i32 last_wheel;
    bool buttons[Maple_MouseButtonMax];
    bool last_buttons[Maple_MouseButtonMax];
DECLARE_RESOURCE_END(Res_InputMouse)
DEFINE_INTERFACE_BEGIN(Res_InputMouse, Default, res_input_mouse)
    res_input_mouse_df.x = 0.0f;
    res_input_mouse_df.y = 0.0f;
    res_input_mouse_df.last_x = 0.0f;
    res_input_mouse_df.last_y = 0.0f;
    res_input_mouse_df.wheel = 0;
    res_input_mouse_df.last_wheel = 0;
    for (i32 i = 0; i < Maple_MouseButtonMax; i++) {
        res_input_mouse_df.buttons[i] = false;
    }
    for (i32 i = 0; i < Maple_MouseButtonMax; i++) {
        res_input_mouse_df.last_buttons[i] = false;
    }
DEFINE_INTERFACE_END(Res_InputMouse, Default, res_input_mouse)
