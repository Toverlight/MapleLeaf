#include <maple/basic_types.h>
#include <maple/checker.h>
#include <maple/query.h>
#include <maple/default_plugin.h>
#include <maple/application.h>
#include <maple/builtin/components.h>
#include <maple/builtin/resources.h>
#include <maple/comp_types.h>
#include <maple/builtin/sdl3_layer.h>

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

static void maple_update_none_layout(Node* node) {
    ComputedNode* computed = &node->computed;
    if (computed->half_width < 1e-2f || computed->half_height < 1e-2f) {
        computed->half_width = node->preferred_half_width;
        computed->half_height = node->preferred_half_height;
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
                maple_update_none_layout(nodes[i]);
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

void maple_df_btn_processor(void) {
    QueryIter query = QUERY(
        Q_SELECT(Button),
        Q_SELECT(Node)
    );
    QUERY_INIT(&query);
    Q_EXEC(&query);
    QueryTarget target;
    while (Q_NEXT(&query, &target)) {
        Button* button = (Button*)Q_FETCH(&query, target, Button);
        Node* node = (Node*)Q_FETCH(&query, target, Node);

        const Res_InputMouse* res_input_mouse = res_get_full_addr(Res_InputMouse);
        ComputedNode* computed = &node->computed;
        bool in_bound = check_if_point_in_rect(
            (Vec2){computed->half_width, computed->half_height},
            (Vec2){computed->center_x, computed->center_y},
            (Vec2){res_input_mouse->x, res_input_mouse->y}
        );
        if (in_bound) {
            if (res_input_mouse->buttons[MouseButton_Left]) {
                button->state = BtnState_Pressed;
            } else {
                button->state = BtnState_Hovered;
                if (res_input_mouse->last_buttons[MouseButton_Left]) {
                    if (button->callback) button->callback();
                }
            }
        } else {
            button->state = BtnState_Idle;
        }
    }
    QUERY_FREE(&query);
}

// TODO 渲染阶段的textures按顺序渲染
void maple_df_text_tile_updater(void) {
    QueryIter query = QUERY(
        Q_SELECT(Text),
        Q_SELECT(Node)
    );
    QUERY_INIT(&query);
    Q_EXEC(&query);
    QueryTarget target;
    while (Q_NEXT(&query, &target)) {
        Text* text = (Text*)Q_FETCH(&query, target, Text);
        Node* node = (Node*)Q_FETCH(&query, target, Node);

        // FIXME 尺寸可能还受其他属性控制
        if (text->computed.half_width == node->computed.half_width
        || text->computed.half_height == node->computed.half_height) {
            text->size_changed = false;
        } else {
            text->computed.half_width = node->computed.half_width;
            text->computed.half_height = node->computed.half_height;
            text->size_changed = true;
        }

        if (text->content_changed || text->size_changed) {
            arrsetlen(text->computed.textures, 0);
            Utf8TileItem item = (Utf8TileItem) { .textures = text->computed.textures, .font_height = node->font_height };
            utf8_iter_string(text->content, maple_load_utf8_tile, &item);
            text->content_changed = false;
            text->size_changed = false;
        }
    }
    QUERY_FREE(&query);
}

// 默认插件
void default_plugin(struct Application* app) {
    reg_res(Res_TimeFixed, res_time_fixed_default_fn());
    reg_res(Res_TimeDelta, res_time_delta_default_fn());
    reg_res(Res_InputMouse, res_input_mouse_default_fn());

    reg_comp(Transform, nullptr);
    reg_comp(Node, maple_node_free);
    reg_comp(Button, nullptr);
    reg_comp(Text, maple_text_free);

	arrins(app->schedules[PreUpdate], 0, maple_df_message_buf_swapper); // 注册消息缓冲区交换系统
	arrput(app->schedules[PostUpdate], maple_df_node_computer); // 注册节点实际属性计算系统
	arrput(app->schedules[PostUpdate], maple_df_btn_processor); // 注册按钮处理系统
	arrput(app->schedules[PostUpdate], maple_df_text_tile_updater); // 注册文本更新系统
}
