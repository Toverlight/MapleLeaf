#pragma once

#include <maple/resource.h>
#include <maple/basic_types.h>
#include <SDL.h>

DECLARE_RESOURCE_BEGIN(Res_TimeFixed)
	u64 nspf;
	f64 mspf;
	f64 spf;
DECLARE_RESOURCE_END(Res_TimeFixed)

DECLARE_RESOURCE_BEGIN(Res_TimeDelta)
    u64 dns;
	f64 dms;
	f64 ds;
DECLARE_RESOURCE_END(Res_TimeDelta)

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
