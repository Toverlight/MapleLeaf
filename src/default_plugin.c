#include <maple/basic_types.h>
#include <maple/checker.h>
#include <maple/query.h>
#include <maple/default_plugin.h>
#include <maple/application.h>
#include <maple/builtin/components.h>
#include <maple/builtin/resources.h>
#include <maple/comp_types.h>
#include <maple/builtin/sdl3_layer.h>
#include <math.h>

void maple_df_event_pumper(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                DLOG(u8"SDL_EVENT_QUIT received, stopping main loop");
                app_get()->condition = false;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                SDL_ConvertEventToRenderCoordinates(app_get()->renderer, &event);
                res_swap_update(Res_InputMouse, x, event.motion.x);
                res_swap_update(Res_InputMouse, y, event.motion.y);
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                SDL_ConvertEventToRenderCoordinates(app_get()->renderer, &event);
                res_swap_update(Res_InputMouse, wheel, event.wheel.y);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                SDL_ConvertEventToRenderCoordinates(app_get()->renderer, &event);
                switch (event.button.button) {
                    case SDL_BUTTON_LEFT:
                        res_swap_update(Res_InputMouse, buttons[MouseButton_Left], event.button.down);
                        break;
                    case SDL_BUTTON_RIGHT:
                        res_swap_update(Res_InputMouse, buttons[MouseButton_Right], event.button.down);
                        break;
                    case SDL_BUTTON_MIDDLE:
                        res_swap_update(Res_InputMouse, buttons[MouseButton_Middle], event.button.down);
                        break;
                }
                break;
                // TODO keyboard and other mouse inputs -> to Res & Signal
            default:
                // DLOG_LIMITED(10, u8"Unsupported event type '%u'. Check if invalid certainly or coming soon", event.type);
                break;
        }
    }
}

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
    if (!w_sum) { // 没有孩子 或 权重全为0（后者理论上不应该存在）
        LOG_WARN_LIMITED(10, u8"Node has no children or weight sum is 0");
        return;
    }
    // TODO 现从左往右。以后可能添加从右往左的
    f32 cur_left = node->computed.rect.center.x - node->computed.rect.half_size.x;
    for (isize i = 0; i < arrlen(node->children); i++) {
        Node* child = node->children[i];
        child->computed.rect.half_size.x = node->computed.rect.half_size.x * (f32)child->weight / (f32)w_sum;
        child->computed.rect.center.x = cur_left + child->computed.rect.half_size.x;
        cur_left += child->computed.rect.half_size.x * 2.0f;
    }
}

static void maple_update_vertical_box(Node* node) {
    u32 w_sum = 0;
    for (isize i = 0; i < arrlen(node->children); i++) {
        w_sum += node->children[i]->weight;
    }
    if (!w_sum) { // 没有孩子 或 权重全为0（后者理论上不应该存在）
        LOG_WARN_LIMITED(10, u8"Node has no children or weight sum is 0");
        return;
    }
    // TODO 现从上往下。以后可能添加从下往上的
    f32 cur_up = node->computed.rect.center.y - node->computed.rect.half_size.y;
    for (isize i = 0; i < arrlen(node->children); i++) {
        Node* child = node->children[i];
        child->computed.rect.half_size.y = node->computed.rect.half_size.y * (f32)child->weight / (f32)w_sum;
        child->computed.rect.center.y = cur_up + child->computed.rect.half_size.y;
        cur_up += child->computed.rect.half_size.y * 2.0f;
    }
}

static void maple_update_none_layout(Node* node) {
    ComputedNode* computed = &node->computed;
    if (!computed->center_owned && node->parent) { // 继承父center
        computed->rect.center.x = node->parent->computed.rect.center.x;
        computed->rect.center.y = node->parent->computed.rect.center.y;
    }
    if (computed->rect.half_size.x < 1e-2f || computed->rect.half_size.y < 1e-2f) {
        computed->rect.half_size.x = node->preferred_half_width;
        computed->rect.half_size.y = node->preferred_half_height;
        DLOG_LIMITED(20, u8"Node (of entity '%d')'s half size is automatically set to preference: (%.1f,%.1f)",
            node->owner, computed->rect.half_size.x, computed->rect.half_size.y);
    }
}

static void maple_node_update_tree(Node* root) {
    Node** nodes = nullptr;
    arrput(nodes, root);
    isize i = 0;
    do {
        DLOG_ONCE(u8"Node's layout is %d", nodes[i]->layout);
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
    QueryIter query = QUERY(
        Q_SELECT(Node),
        Q_OPTION(Transform)
    );
    QUERY_INIT(&query);
    Q_EXEC(&query);
    QueryTarget target;
    while (Q_NEXT(&query, &target)) {
        static bool log_flag_next = false;
        if (!log_flag_next) {
            const utf8* query_next_str = query_to_string_fn(&query);
            DLOG_ONCE(u8"Query advanced the first step: %s", query_next_str);
            arrfree(query_next_str);
            log_flag_next = true;
        }
        Node* node = (Node*)Q_FETCH(&query, target, Node);
        const Transform* transform = (Transform*)Q_FETCH(&query, target, Transform);

        if (transform) {
            node->computed.rect.center.x = transform->px;
            node->computed.rect.center.y = transform->py;
            node->computed.center_owned = true;
        } else {
            node->computed.center_owned = false;
        }

        if (node->parent) continue;
        maple_node_update_tree(node);
    }
    QUERY_FREE(&query);
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
            (Vec2){computed->rect.half_size.x, computed->rect.half_size.y},
            (Vec2){computed->rect.center.x, computed->rect.center.y},
            (Vec2){res_input_mouse->x, res_input_mouse->y}
        );
        if (in_bound) { // State changing
            if (res_input_mouse->buttons[MouseButton_Left]) {
                button->state = BtnState_Pressed;
            } else {
                button->state = BtnState_Hovered;
            }
        } else {
            button->state = BtnState_Idle;
        }
        if (in_bound) { // Click judging
            if (!res_input_mouse->buttons[MouseButton_Left]
                && res_input_mouse->last_buttons[MouseButton_Left]) {
                if (button->callback) button->callback(button->data);
            }
        }
    }
    QUERY_FREE(&query);
}

void maple_df_text_tiles_updater(void) {
    DLOG_ONCE(u8"Updating text tiles...");
    QueryIter query = QUERY(
        Q_SELECT(Text),
        Q_SELECT(Node)
    );
    QUERY_INIT(&query);
    static bool log_flag_init = false;
    if (!log_flag_init) {
        const utf8* query_inited_str = query_to_string_fn(&query);
        DLOG_ONCE(u8"Query just initialized: %s", query_inited_str);
        arrfree(query_inited_str);
        log_flag_init = true;
    }
    Q_EXEC(&query);
    static bool log_flag_exec = false;
    if (!log_flag_exec) {
        const utf8* query_exec_str = query_to_string_fn(&query);
        DLOG_ONCE(u8"Query just executed: %s", query_exec_str);
        arrfree(query_exec_str);
        log_flag_exec = true;
    }
    QueryTarget target;
    while (Q_NEXT(&query, &target)) {
        static bool log_flag_next = false;
        if (!log_flag_next) {
            const utf8* query_next_str = query_to_string_fn(&query);
            DLOG_ONCE(u8"Query advanced the first step: %s", query_next_str);
            arrfree(query_next_str);
            log_flag_next = true;
        }
        Text* text = (Text*)Q_FETCH(&query, target, Text);
        Node* node = (Node*)Q_FETCH(&query, target, Node);
        static bool log_flag_fetch = false;
        if (!log_flag_fetch) {
            const utf8* query_fetch_str = query_to_string_fn(&query);
            DLOG_ONCE(u8"Query just fetched: %s", query_fetch_str);
            arrfree(query_fetch_str);
            log_flag_fetch = true;
        }

        if (text->content_changed) {
            // DLOG_ONCE(u8"Entered branch 'changed'");
            arrsetlen(text->computed.textures, 0);
            arrsetlen(text->computed.aspects, 0);
            Utf8TileItem item = (Utf8TileItem) {
                .textures = text->computed.textures,
                .aspects = text->computed.aspects,
                .font_height = node->font_height,
                .font_name = node->font_name };
            utf8_iter_string(text->content, maple_load_utf8_tile, &item);
            text->computed.textures = item.textures;
            text->computed.aspects = item.aspects;
            text->content_changed = false;
        }

        f32 total_w = 0.0f;
        for (isize i = 0; i < arrlen(text->computed.aspects); i++) {
            total_w += (f32)node->font_height * text->computed.aspects[i];
        }
        f32 half_w = total_w * 0.5f;
        f32 half_h = (f32)node->font_height * 0.5f;
        if (fabsf(half_w - text->computed.rect.half_size.x) < 1e-2f ||
            fabsf(half_h - text->computed.rect.half_size.y) < 1e-2f) {
            text->size_changed = false;
        } else {
            text->size_changed = true;
            text->computed.rect.half_size.x = half_w;
            text->computed.rect.half_size.y = half_h;
        }

        // TODO 文本的锚点、布局等
        // FIXME 布局暂时就是居中。之后改
        text->computed.rect.center.x = node->computed.rect.center.x;
        text->computed.rect.center.y = node->computed.rect.center.y;
    }
    QUERY_FREE(&query);
}

void maple_df_input_resources_syncer(void) {
    // InputMouse
    res_sync_last(Res_InputMouse, x);
    res_sync_last(Res_InputMouse, y);
    res_sync_last(Res_InputMouse, wheel);
    for (i32 i = 0; i < Maple_MouseButtonMax; i++) {
        res_sync_last(Res_InputMouse, buttons[i]);
    }
    // TODO other resources those with Last Fields to be synced
}

void maple_df_render_start(void) {
    RendererHandle renderer = app_get()->renderer;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

static void render_debug_rect(RendererHandle renderer, DebugDisplay* dd, CompId id, Shape_Rect rect) {
    ColorRgba fill_color = dd_get_target_shading_fill_color(dd, id);
    ColorRgba line_color = dd_get_target_shading_line_color(dd, id);
    if (fill_color.a == 0 && line_color.a == 0) return;
    SDL_FRect dst = { rect.center.x - rect.half_size.x, rect.center.y - rect.half_size.y,
                      2.0f * rect.half_size.x, 2.0f * rect.half_size.y };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    if (fill_color.a != 0) {
        SDL_SetRenderDrawColor(renderer, fill_color.r, fill_color.g, fill_color.b, fill_color.a);
        SDL_RenderFillRect(renderer, &dst);
    }
    if (line_color.a != 0) {
        SDL_SetRenderDrawColor(renderer, line_color.r, line_color.g, line_color.b, line_color.a);
        SDL_RenderRect(renderer, &dst);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void maple_df_render_ui(void) {
    QueryIter query = QUERY(
        Q_SELECT(Node),
        Q_OPTION(Text),
        // TODO more ui renderables to be added
        Q_OPTION(DebugDisplay)
    );
    QUERY_INIT(&query);
    Q_EXEC(&query);
    QueryTarget target;
    while (Q_NEXT(&query, &target)) {
        const Node* node = (Node*)Q_FETCH(&query, target, Node);
        // if (node->parent) continue; // 从“根”节点（可能多个）开始

        DebugDisplay* dd = (DebugDisplay*)Q_FETCH(&query, target, DebugDisplay);
        RendererHandle renderer = app_get()->renderer;
        if (dd) {
            render_debug_rect(renderer, dd, comp_id(Node), node->computed.rect);
        }

        // TODO more subjects to render...(note the order)
        const Text* text = (Text*)Q_FETCH(&query, target, Text);
        if (text) {
            // TODO to support text texture rotation in the future, render target may be drawn as the temp one and rotated later...
            SDL_SetRenderTarget(renderer, nullptr);
            f32 cursor_x = text->computed.rect.center.x - text->computed.rect.half_size.x;
            f32 top_y = text->computed.rect.center.y - text->computed.rect.half_size.y;
            for (isize i = 0; i < arrlen(text->computed.textures); i++) {
                SDL_FRect dst;
                dst.w = node->font_height * text->computed.aspects[i];
                dst.h = node->font_height;
                dst.x = cursor_x;
                dst.y = top_y;
                cursor_x += dst.w;
                const ColorRgba* color = &node->font_forecolor;
                SDL_SetTextureColorMod(text->computed.textures[i], color->r, color->g, color->b);
                SDL_SetTextureAlphaMod(text->computed.textures[i], color->a);
                SDL_RenderTexture(renderer, text->computed.textures[i], nullptr, &dst);
                // SDL_SetTextureColorMod(text->computed.textures[i], 255, 255, 255);
                // SDL_SetTextureAlphaMod(text->computed.textures[i], 255);
            }

            if (dd) {
                render_debug_rect(renderer, dd, comp_id(Text), text->computed.rect);
            }
        }
    }
    QUERY_FREE(&query);
}

void maple_df_render_present(void) {
    SDL_RenderPresent(app_get()->renderer);
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
    reg_comp(DebugDisplay, maple_dd_free);

	arrins(app->schedules[PreUpdate], 0, maple_df_event_pumper); // 注册事件泵系统
	arrins(app->schedules[PreUpdate], 1, maple_df_message_buf_swapper); // 注册消息缓冲区交换系统

	arrput(app->schedules[PostUpdate], maple_df_node_computer); // 注册节点实际属性计算系统
	arrput(app->schedules[PostUpdate], maple_df_btn_processor); // 注册按钮处理系统
	arrput(app->schedules[PostUpdate], maple_df_text_tiles_updater); // 注册文本更新系统
	// TODO other PostUpdate systems...
	arrput(app->schedules[PostUpdate], maple_df_input_resources_syncer); // 注册资源滞后域同步系统

	arrput(app->schedules[Render], maple_df_render_start); // 注册渲染开始
	// TODO other rendering systems (must be) before ui
	arrput(app->schedules[Render], maple_df_render_ui); // 注册ui渲染系统
	// TODO register the rendering system
	arrput(app->schedules[Render], maple_df_render_present); // 注册present

	DLOG(u8"For default_plugin:");
	for (isize i = 0; i < arrlen(app->schedules); i++) {
	    DLOG(u8"Schedule %d with df systems num %d", i, arrlen(app->schedules[i]));
	}
}
