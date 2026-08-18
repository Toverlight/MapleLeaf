#include "default_plugin.h"
#include "application.h"

void maple_df_message_buf_swapper(void) {
	Application* app = app_get();
	Message* temp = app->next_messages;
	app->next_messages = app->current_messages;
	app->current_messages = temp;
	arrsetlen(app->next_messages, 0);
}

// 默认插件
void default_plugin(struct Application* app) {
	arrins(app->schedules[PreUpdate], 0, maple_df_message_buf_swapper); // 注册消息缓冲区交换系统
}
