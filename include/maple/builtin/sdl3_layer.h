#pragma once

#include <SDL.h>
#include <maple/basic_types.h>
#include <maple/tools.h>

typedef SDL_Texture* TextureHandle;

typedef struct Utf8TileEntry {
    const char* key;
    TextureHandle value;
} Utf8TileEntry, *Utf8TileReg;

DECLARE_HACKER_COPIED(Utf8TileReg, utf8_tile_reg)

typedef struct Utf8TileItem {
    TextureHandle* textures;
    u32 font_height;
    // TODO ...字重等（至于颜色统一白色，后用tint实现文本变色）
} Utf8TileItem;

// 带缓存机制的 utf8 tile 加载
void maple_load_utf8_tile(const utf8* utf8_char, usize bytes, void* data);

typedef void(*Utf8CharFn)(const utf8* utf8_char, usize bytes, void* data);

void utf8_iter_string(const utf8 utf8_string[], Utf8CharFn func, void* data);
