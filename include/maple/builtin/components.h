#pragma once

#include <maple/comp_types.h>
#include <maple/traits/serde.h>
#include <maple/traits/default.h>

// prefix 'transform'
DECLARE_COMP_BEGIN(Transform)
    f32 px;
    f32 py;
    f32 r; // in radian
    f32 sx;
    f32 sy;
DECLARE_COMP_END(Transform)
DECLARE_INTERFACE(Transform, Serde, transform)
DEFINE_INTERFACE_BEGIN(Transform, Default, transform)
    transform_df.px = 0.0f;
    transform_df.py = 0.0f;
    transform_df.r = 0.0f;
    transform_df.sx = 1.0f;
    transform_df.sy = 1.0f;
DEFINE_INTERFACE_END(Transform, Default, transform)

typedef enum : u32 {
    LAYOUT_INVALID,

    Layout_HorizontalBox, // left->right
    Layout_VerticalBox, // up->down

    // TODO ...
} Layout;

// prefix 'cnode'
typedef struct ComputedNode {
    f32 half_width;
    f32 half_height;
    f32 center_x;
    f32 center_y;
} ComputedNode;
DEFINE_INTERFACE_BEGIN(ComputedNode, Default, cnode)
    cnode_df.half_width = 0.0f;
    cnode_df.half_height = 0.0f;
    cnode_df.center_x = 0.0f;
    cnode_df.center_y = 0.0f;
DEFINE_INTERFACE_END(ComputedNode, Default, cnode)

// prefix 'node'
DECLARE_COMP_BEGIN(Node)
    struct Node* parent;
    struct Node** children;
    u32 weight;
    Layout layout;
    ComputedNode computed;
    // TODO ...
DECLARE_COMP_END(Node)
DEFINE_INTERFACE_BEGIN(Node, Default, node)
    node_df.parent = nullptr;
    node_df.children = nullptr;
    node_df.weight = 1;
    node_df.layout = LAYOUT_INVALID;
    node_df.computed = cnode_default_fn();
DEFINE_INTERFACE_END(Node, Default, node)
bool node_add_child(Node* node, Node* child);
bool node_remove_child(Node* node, Node* child);
bool node_set_parent(Node* node, Node* parent);

typedef enum : u8 {
    BtnState_Idle,
    BtnState_Hovered,
    BtnState_Pressed,
} BtnState;

typedef void(*BtnCallback)(void);

// prefix 'btn'
DECLARE_COMP_BEGIN(Button)
    BtnState state;
    BtnCallback callback;
DECLARE_COMP_END(Button)
DEFINE_INTERFACE_BEGIN(Button, Default, btn)
    btn_df.state = BtnState_Idle;
    btn_df.callback = nullptr;
DEFINE_INTERFACE_END(Button, Default, btn)
