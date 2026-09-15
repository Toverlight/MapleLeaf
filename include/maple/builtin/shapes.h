#pragma once

#include <maple/basic_types.h>
#include <maple/traits/default.h>
#include <maple/tools.h>

typedef struct Shape_Rect {
    Vec2 center;
    Vec2 half_size;
} Shape_Rect;
DEFINE_INTERFACE_BEGIN(Shape_Rect, Default, rect)
    rect_df.center = (Vec2){ 0.0f, 0.0f };
    rect_df.half_size = (Vec2){ 0.0f, 0.0f };
DEFINE_INTERFACE_END(Shape_Rect, Default, rect)
