#pragma once

#include <maple/comp_types.h>
#include <maple/traits/serde.h>
#include <maple/traits/default.h>
#include <maple/builtin/sdl3_layer.h>

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
    LAYOUT_NONE,

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
    bool center_owned;
} ComputedNode;
DEFINE_INTERFACE_BEGIN(ComputedNode, Default, cnode)
    cnode_df.half_width = 0.0f;
    cnode_df.half_height = 0.0f;
    cnode_df.center_x = 0.0f;
    cnode_df.center_y = 0.0f;
    cnode_df.center_owned = false;
DEFINE_INTERFACE_END(ComputedNode, Default, cnode)

typedef struct ColorRgba {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} ColorRgba;

/// prefix 'node'
///
/// Now Node's center is determined by as follows, 3 priorities:
///
/// 1. Its parent has a certain layout. By its parent.
///
/// 2. It has no parent or is root. And has a Transform component. By Transform.
///
/// 3. Else case: as child, by parent center; as root, by default value.
///
DECLARE_COMP_BEGIN(Node)
    ComputedNode computed;
    struct Node* parent;
    struct Node** children;
    u32 weight;
    Layout layout;
    f32 preferred_half_width;
    f32 preferred_half_height;
    // font
    const utf8* font_name;
    u32 font_height;
    ColorRgba font_forecolor;

    // TODO border properties. And for button, if no border properties or images, set default border
    // TODO ...
DECLARE_COMP_END(Node)
DEFINE_INTERFACE_BEGIN(Node, Default, node)
    node_df.computed = cnode_default_fn();
    node_df.parent = nullptr;
    node_df.children = nullptr;
    node_df.weight = 1;
    node_df.layout = LAYOUT_NONE;
    node_df.preferred_half_width = 0.0f;
    node_df.preferred_half_height = 0.0f;
    node_df.font_name = nullptr;
    node_df.font_height = 12;
    node_df.font_forecolor = (ColorRgba){248, 92, 57, 255}; // FIXME now orange-like
DEFINE_INTERFACE_END(Node, Default, node)
bool node_add_child(Node* node, Node* child);
bool node_remove_child(Node* node, Node* child);
bool node_set_parent(Node* node, Node* parent);
void maple_node_free(void* comp);

typedef enum : u8 {
    BtnState_Idle,
    BtnState_Hovered,
    BtnState_Pressed,
} BtnState;

typedef void(*BtnCallback)(void* data);

// prefix 'btn'
DECLARE_COMP_BEGIN(Button)
    BtnState state;
    BtnCallback callback;
    void* data;
DECLARE_COMP_END(Button)
DEFINE_INTERFACE_BEGIN(Button, Default, btn)
    btn_df.state = BtnState_Idle;
    btn_df.callback = nullptr;
    btn_df.data = nullptr;
DEFINE_INTERFACE_END(Button, Default, btn)

// prefix 'ctext'
// TODO ComputedText的尺寸之后再提供更新方式（width,height是否必要尚不确定)
typedef struct ComputedText {
    // utf8 characters (font atlas) sequence
    // FIXME not only left to right? multi-lines and etc?
    TextureHandle* textures;
    f32* aspects;
    f32 half_width;
    f32 half_height;
} ComputedText;
DEFINE_INTERFACE_BEGIN(ComputedText, Default, ctext)
    ctext_df.textures = nullptr;
    ctext_df.aspects = nullptr;
    ctext_df.half_width = 0.0f;
    ctext_df.half_height = 0.0f;
DEFINE_INTERFACE_END(ComputedText, Default, ctext)

// prefix 'text'
DECLARE_COMP_BEGIN(Text)
    ComputedText computed;
    const utf8* content;
    bool content_changed;
    bool size_changed;
DECLARE_COMP_END(Text)
DEFINE_INTERFACE_BEGIN(Text, Default, text)
    text_df.computed = ctext_default_fn();
    text_df.content = nullptr;
    text_df.content_changed = false;
    text_df.size_changed = false; // FIXME In test
DEFINE_INTERFACE_END(Text, Default, text)
// Text的content修改须通过该方法，以确保纹理tiles被正常更新
static inline void text_set_content(Text* text, const utf8* utf8_string) {
    text->content = utf8_string;
    text->content_changed = true;
}
// 有内容的Text初始化须通过该方法，以确保纹理tiles被正常更新
static inline Text text_new(const utf8* utf8_string) {
    Text text = text_default_fn();
    text_set_content(&text, utf8_string);
    return text;
}
void maple_text_free(void* comp);
