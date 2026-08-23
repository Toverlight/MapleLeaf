#pragma once

#include <maple/basic_types.h>

static inline bool check_if_point_in_rect(Vec2 half_size, Vec2 center, Vec2 point) {
    return (point.x <= center.x + half_size.x
        && point.x >= center.x - half_size.x
        && point.y <= center.y + half_size.y
        && point.y >= center.y - half_size.y);
}
