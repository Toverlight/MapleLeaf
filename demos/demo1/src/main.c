#include <maple.h>
#include "demo1/demo1_plugin.h"

APP_START("Demo1 - test only", "0.1", "com.example.demo1", W_720P, H_720P, W_720P, H_720P)
    DLOG(u8"DEMO_NAME = '%s', ASSETS_PATH = '%s', RESOLUTION = real(%d, %d), logic(%d, %d)",
        DEMO_NAME, ASSETS_PATH, _obj_self->w_real, _obj_self->h_real, _obj_self->w_logic, _obj_self->h_logic);
    app_add_plugin(default_plugin);
    app_add_plugin(demo1_plugin);
    MAIN_LOOP(165);
APP_END()
