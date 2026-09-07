#include "demo1/demo1_plugin.h"
#include "demo1/ui_display.h"

void demo1_plugin(Application* app) {
    LOG_INFO(u8"demo1 plugin loading...");
    WITH_COPIED(app,
        app_add_system(Startup, ui_spawn);
    );
}
