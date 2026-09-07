#include "demo1/ui_display.h"

void ui_spawn(void) {
    bool suc_entity = false;
    EntityCommand ecmd = cmd_spawn(COMMAND(), &suc_entity);
    if (suc_entity) DLOG_ONCE(u8"Spawned entity '%d'", ecmd.target);
    Node node = node_default_fn();
    node.preferred_half_width = 500;
    node.preferred_half_height = 100;
    node.font_height = 48;
    node.computed.center_x = 640.0f;
    node.computed.center_y = 360.0f;

    // DLOG_ONCE(u8"Node's layout is %d initially", node.layout);
    Text text = text_new(u8"Hello Maple引擎!");
    WITH(ecmd,
        ecmd_insert(Node, &node, nullptr);
        ecmd_insert(Text, &text, nullptr);
    );
    // CompTypeReg comp_type_reg = HACKER_COPIED(comp_type_reg);
    // isize i = hmgeti(comp_type_reg, comp_id(Node));
    // if (i >= 0) {
    //     DLOG_ONCE(u8"After insert, Node's layout is %d", ((Node*)(comp_type_reg[i].value.dense_set))[0].layout);
    // }
}
