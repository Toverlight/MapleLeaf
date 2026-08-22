#include <maple/default_plugin.h>
#include <maple/application.h>
#include <maple/builtin/components.h>
#include <maple/comp_types.h>

void maple_df_message_buf_swapper(void) {
	Application* app = app_get();
	Message* temp = app->next_messages;
	app->next_messages = app->current_messages;
	app->current_messages = temp;
	arrsetlen(app->next_messages, 0);
}

static void maple_update_horizontal_box(Node* node) {
    u32 w_sum = 0;
    for (isize i = 0; i < arrlen(node->children); i++) {
        w_sum += node->children[i]->weight;
    }
    if (!w_sum) return; // 没有孩子 或 权重全为0（后者理论上不应该存在）
    // TODO 现从左往右。以后可能添加从右往左的
    f32 cur_left = node->computed.center_x - node->computed.half_width;
    for (isize i = 0; i < arrlen(node->children); i++) {
        Node* child = node->children[i];
        child->computed.half_width = node->computed.half_width * (f32)child->weight / (f32)w_sum;
        child->computed.center_x = cur_left + child->computed.half_width;
        cur_left += child->computed.half_width * 2.0f;
    }
}

static void maple_update_vertical_box(Node* node) {
    u32 w_sum = 0;
    for (isize i = 0; i < arrlen(node->children); i++) {
        w_sum += node->children[i]->weight;
    }
    if (!w_sum) return; // 没有孩子 或 权重全为0（后者理论上不应该存在）
    // TODO 现从上往下。以后可能添加从下往上的
    f32 cur_up = node->computed.center_y - node->computed.half_height;
    for (isize i = 0; i < arrlen(node->children); i++) {
        Node* child = node->children[i];
        child->computed.half_height = node->computed.half_height * (f32)child->weight / (f32)w_sum;
        child->computed.center_y = cur_up + child->computed.half_height;
        cur_up += child->computed.half_height * 2.0f;
    }
}

static void maple_node_update_tree(Node* root) {
    Node** nodes = nullptr;
    arrput(nodes, root);
    isize i = 0;
    do {
        // ------
        switch (nodes[i]->layout) {
            case Layout_HorizontalBox:
                maple_update_horizontal_box(nodes[i]);
                break;
            case Layout_VerticalBox:
                maple_update_vertical_box(nodes[i]);
                break;
            default: // 默认，即无布局 或 暂未实装的布局
                // TODO 后续添加 prefered，控制特殊节点的属性。目前根节点要手动设置 computed
                break;
        }
        // -------
        for (isize j = 0; j < arrlen(nodes[i]->children); j++) {
            arrput(nodes, nodes[i]->children[j]);
        }
        i++;
    } while(i < arrlen(nodes));
}

void maple_df_node_computer(void) {
    CompTypeReg comp_type_reg = HACKER_COPIED(comp_type_reg);
    isize i = hmgeti(comp_type_reg, comp_id(Node));
    if (i < 0) {
        // TODO error: 未注册的组件Node
        return;
    }
    for (isize j = 0; j < comp_type_reg[i].value.dense_len; j++) {
        Node* node = &comp_type_reg[i].value.dense_set[j * sizeof(Node)];
        if (node->parent) continue; // 从“根”节点（可能多个）开始
        maple_node_update_tree(node);
    }
}

// 默认插件
void default_plugin(struct Application* app) {
    reg_comp(Transform);
    reg_comp(Node);

	arrins(app->schedules[PreUpdate], 0, maple_df_message_buf_swapper); // 注册消息缓冲区交换系统
	arrput(app->schedules[PostUpdate], maple_df_node_computer); // 注册节点实际属性计算系统
}
