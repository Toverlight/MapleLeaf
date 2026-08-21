#pragma once

#include "comp_types.h"
#include "traits/serde.h"

DECLARE_COMP_BEGIN(Transform)
    f32 px;
    f32 py;
    f32 r;
    f32 sx;
    f32 sy;
DECLARE_COMP_END(Transform)
DECLARE_INTERFACE(Transform, Serde, transform)

typedef enum : u8 {
    LAYOUT_INVALID,

    Layout_HorizontalBox,
    Layout_VerticalBox,

    // TODO ...
} Layout;

DECLARE_COMP_BEGIN(Node)
    u32 weight;
    Layout layout;
    // TODO ...
DECLARE_COMP_END(Node)
