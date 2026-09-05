#include <maple/path_utils.h>
#include <maple/builtin/sdl3_layer.h>
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <stdio.h>

static PathReg path_reg = nullptr;

const utf8* maple_asset_full(const utf8* rel_path) {
    static const utf8* base_path = nullptr;
    if (!base_path) {
        base_path = (const utf8*)SDL_GetBasePath();
        DLOG(u8"Base path: %s", (const char*)base_path);
    }
    isize i = shgeti(path_reg, (const char*)rel_path);
    if (i >= 0) {
        return path_reg[i].value;
    }
    usize full_path_len = strlen((const char*)base_path) + strlen((const char*)rel_path) + 1;
    utf8* full_path = (utf8*)malloc(sizeof(utf8) * full_path_len);
    snprintf((char*)full_path, full_path_len, "%s%s", (const char*)base_path, (const char*)rel_path);
    DLOG_ONCE(u8"Full path: %s", (const char*)full_path);
    shput(path_reg, (const char*)rel_path, full_path);
    return full_path;
}

void maple_clear_paths(void) {
    for (isize i = 0; i < shlen(path_reg); i++) {
        free(path_reg[i].value);
    }
    shfree(path_reg);
}
