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
    node.preferred_half_width = 500;
    node.preferred_half_height = 100;
    node.font_height = 48;

    Text text = text_new(u8"Hello Maple引擎!");

    Transform transform = transform_default_fn();
    transform.px = 640.0f;
    transform.py = 360.0f;

    Button button = btn_default_fn();
    button.callback = demo1_btn_callback;
    button.data = &some_data;

    WITH(ecmd,
        ecmd_insert(Node, &node, nullptr);
        ecmd_insert(Text, &text, nullptr);
        ecmd_insert(Transform, &transform, nullptr);
        ecmd_insert(Button, &button, nullptr);
    );
    // CompTypeReg comp_type_reg = HACKER_COPIED(comp_type_reg);
    // isize i = hmgeti(comp_type_reg, comp_id(Node));
    // if (i >= 0) {
    //     DLOG_ONCE(u8"After insert, Node's layout is %d", ((Node*)(comp_type_reg[i].value.dense_set))[0].layout);
    // }
}
