#pragma once

#include <maple/comp_types.h>
#include <maple/traits/serde.h>
#include <maple/traits/default.h>
#include <maple/builtin/sdl3_layer.h>
#include <maple/builtin/shapes.h>

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
    Shape_Rect rect;
    bool center_owned;
} ComputedNode;
DEFINE_INTERFACE_BEGIN(ComputedNode, Default, cnode)
    cnode_df.rect = rect_default_fn();
    cnode_df.center_owned = false;
DEFINE_INTERFACE_END(ComputedNode, Default, cnode)

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
    // original colors
    // ColorRgba bgcolor;
    // border
    // f32 border_thickness;
    // f32 border_radius;
    // ColorRgba border_color;

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
    Shape_Rect rect;
} ComputedText;
DEFINE_INTERFACE_BEGIN(ComputedText, Default, ctext)
    ctext_df.textures = nullptr;
    ctext_df.aspects = nullptr;
    ctext_df.rect = rect_default_fn();
DEFINE_INTERFACE_END(ComputedText, Default, ctext)

// prefix 'text'
DECLARE_COMP_BEGIN(Text)
    ComputedText computed;
    const utf8* content;
    bool content_changed; // Detect whether tile sequence has been changed
    bool size_changed; // Detect whether computed rect has been changed
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

typedef struct DDSCEntry {
    CompId key;
    ColorRgba value;
} DDSCEntry, *DDSCMap;

/// prefix 'dd'
///
/// Shading color controls the draw color of the line or inner part. Shading is disabled when the corresponding alpha is 0.
///
/// This display layer is above the debug target. Sometimes consider make semi-transparent to let the both show.
DECLARE_COMP_BEGIN(DebugDisplay)
    DDSCMap shading_line_colors;
    DDSCMap shading_fill_colors;
DECLARE_COMP_END(DebugDisplay)
DEFINE_INTERFACE_BEGIN(DebugDisplay, Default, dd)
    dd_df.shading_line_colors = nullptr;
    ColorRgba color_df = (ColorRgba){ 0, 0, 0, 0 }; // Transparent color. Alpha 0 also symbolizes it 'invalid'
    hmdefault(dd_df.shading_line_colors, color_df);
    dd_df.shading_fill_colors = nullptr;
    hmdefault(dd_df.shading_fill_colors, color_df);
DEFINE_INTERFACE_END(DebugDisplay, Default, dd)
static inline void dd_set_target_shading_line_color(DebugDisplay* dd, CompId comp_id, ColorRgba color) {
    hmput(dd->shading_line_colors, comp_id, color);
}
static inline void dd_set_target_shading_fill_color(DebugDisplay* dd, CompId comp_id, ColorRgba color) {
    hmput(dd->shading_fill_colors, comp_id, color);
}
static inline ColorRgba dd_get_target_shading_line_color(DebugDisplay* dd, CompId comp_id) {
    return hmget(dd->shading_line_colors, comp_id);
}
static inline ColorRgba dd_get_target_shading_fill_color(DebugDisplay* dd, CompId comp_id) {
    return hmget(dd->shading_fill_colors, comp_id);
}
void maple_dd_free(void* comp);

DECLARE_COMP_BEGIN(Sprite)
    TextureHandle texure;
    Shape_Rect rect; // .center is the offset from transform coordinates .half_size as its drawn size
DECLARE_COMP_END(Sprite)
DEFINE_INTERFACE_BEGIN(Sprite, Default, sprite)
    sprite_df.texure = nullptr;
    sprite_df.rect = rect_default_fn();
DEFINE_INTERFACE_END(Sprite, Default, sprite)
static inline Sprite sprite_new(const utf8* image_path) {
    Sprite sprite = sprite_default_fn();
    sprite.texure = maple_load_image(image_path);
    return sprite;
}

typedef enum : u8 {
    FrameType_Horizontal,
    FrameType_Vertical,
    FrameType_Square,
} FrameType;

DECLARE_COMP_BEGIN(SpriteFrame)
    FrameType frame_type;
    IVec2 size; // must be set
    IVec2 margin; // optional, only set when existent
    IVec2 index; // from 0..
    IVec2 total; // must be set and greater than index
DECLARE_COMP_END(SpriteFrame)
DEFINE_INTERFACE_BEGIN(SpriteFrame, Default, sprite_frame)
    sprite_frame_df.frame_type = FrameType_Square;
    sprite_frame_df.size = (IVec2){ 0, 0 };
    sprite_frame_df.margin = (IVec2){ 0, 0 };
    sprite_frame_df.index = (IVec2){ 0, 0 };
    sprite_frame_df.total = (IVec2){ 0, 0 };
DEFINE_INTERFACE_END(SpriteFrame, Default, sprite_frame)
static inline SpriteFrame sprite_frame_new(FrameType frame_type, IVec2 size, IVec2 index, IVec2 total) {
    SpriteFrame sprite_frame = sprite_frame_default_fn();
    sprite_frame.frame_type = frame_type;
    sprite_frame.size = size;
    sprite_frame.index = index;
    sprite_frame.total = total;
    if (sprite_frame.index.x >= sprite_frame.total.x) sprite_frame.index.x = sprite_frame.total.x - 1;
    if (sprite_frame.index.y >= sprite_frame.total.y) sprite_frame.index.y = sprite_frame.total.y - 1;
    return sprite_frame;
}
