#pragma once

#include <maple/basic_types.h>
#include <stb_ds.h>

typedef struct PathEntry {
    const char* key;
    utf8* value;
} PathEntry, *PathReg;

// Add a base path former
const utf8* maple_asset_full(const utf8* rel_path);

void maple_clear_paths(void);

/// !!Only for specific demo use
/// !!ASSETS_PATH is defined by specific demo, which is invisible to engine
#define ASSET(rel) maple_asset_full(ASSETS_PATH u8"" rel)

#define MAPLE_ASSET(rel) maple_asset_full(MAPLE_ASSETS_PATH u8"" rel)
