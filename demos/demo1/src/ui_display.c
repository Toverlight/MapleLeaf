#include "demo1/ui_display.h"

static void demo1_btn_callback(void* data) {
    if (!data) return;
    DLOG_LIMITED(10, u8"Button clicked! Data is %d", *(i32*)data);
}

static i32 some_data = 42; // For button test

void demo1_ui_spawn(void) {
    bool suc_entity = false;
    EntityCommand ecmd = cmd_spawn(COMMAND(), &suc_entity);
    if (suc_entity) DLOG_ONCE(u8"Spawned entity '%d'", ecmd.target);
    Node node = node_default_fn();
    node.preferred_half_width = 200;
    node.preferred_half_height = 50;
    node.font_height = 48;

    Text text = text_new(u8"Hello Maple引擎!");

    Transform transform = transform_default_fn();
    transform.px = 640.0f;
    transform.py = 360.0f;

    Button button = btn_default_fn();
    button.callback = demo1_btn_callback;
    button.data = &some_data;

    DebugDisplay dd = dd_default_fn();
    dd_set_target_shading_line_color(&dd, comp_id(Text), (ColorRgba){220, 220, 220, 255});
    dd_set_target_shading_line_color(&dd, comp_id(Node), (ColorRgba){174, 174, 174, 255});
    dd_set_target_shading_fill_color(&dd, comp_id(Text), (ColorRgba){228, 125, 129, 127});
    dd_set_target_shading_fill_color(&dd, comp_id(Node), (ColorRgba){238, 126, 200, 127});

    WITH(ecmd,
        ecmd_insert(Node, &node, nullptr);
        ecmd_insert(Text, &text, nullptr);
        ecmd_insert(Transform, &transform, nullptr);
        ecmd_insert(Button, &button, nullptr);
        ecmd_insert(DebugDisplay, &dd, nullptr);
    );

    ecmd = cmd_spawn(COMMAND(), nullptr);

    Sprite sprite = sprite_new(u8"assets/engine/ui/WinXp/Icons/WinIcons_48.png");
    sprite.rect.half_size = (Vec2){ 48, 48 };

    SpriteFrame sf = sprite_frame_new(FrameType_Square, (IVec2){48, 48}, (IVec2){11, 14}, (IVec2){20, 20});

    Transform tf = transform_default_fn();
    tf.px = app_get()->w_real / 2.0f;
    tf.py = 220.0f;

    DebugDisplay dd2 = dd_default_fn();
    dd_set_target_shading_line_color(&dd2, comp_id(Sprite), (ColorRgba){88, 43, 177, 255});
    dd_set_target_shading_fill_color(&dd2, comp_id(Sprite), (ColorRgba){173, 221, 241, 127});

    WITH(ecmd,
        ecmd_insert(Sprite, &sprite, nullptr);
        ecmd_insert(SpriteFrame, &sf, nullptr);
        ecmd_insert(Transform, &tf, nullptr);
        ecmd_insert(DebugDisplay, &dd2, nullptr);
    );

    // CompTypeReg comp_type_reg = HACKER_COPIED(comp_type_reg);
    // isize i = hmgeti(comp_type_reg, comp_id(Node));
    // if (i >= 0) {
    //     DLOG_ONCE(u8"After insert, Node's layout is %d", ((Node*)(comp_type_reg[i].value.dense_set))[0].layout);
    // }
}
