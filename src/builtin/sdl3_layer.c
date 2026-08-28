#include <maple/builtin/sdl3_layer.h>
#include <stb_ds.h>

static Utf8TileReg utf8_tile_reg = nullptr;

IMPL_HACKER_COPIED(Utf8TileReg, utf8_tile_reg)

void maple_load_utf8_tile(const utf8* utf8_char, usize bytes, void* data) {
    Utf8TileItem* out_fitted = (Utf8TileItem*)data;
    isize i = shgeti(utf8_tile_reg, (const char*)utf8_char);
    if (i < 0) {
        // TODO SDL加载该utf8字符到纹理
        // 键的拼接方式：<utf8 char>|<font_height>（暂定u8'|'作为特殊字符）
        // 成功：更新reg；失败：error并返回nullptr
        // FIXME text纹理大小的分级处理由外部完成。类似mipmap多级清晰度的以后优化
        return; // FIXME 删除该行
    }
    arrput(out_fitted->textures, utf8_tile_reg[i].value);
}

void utf8_iter_string(const utf8 utf8_string[], Utf8CharFn func, void* data) {
    if (!utf8_string) return;
    mbstate_t state = {0};
    const utf8 *ptr = utf8_string;
    char32_t cp;
    usize index = 0;

    while (*ptr != u8'\0') {
        // mbrtoc8 将 UTF-8 字节序列转换为 char32_t (UTF-32/Code Point)
        size_t bytes = mbrtoc32(&cp, (const char*)ptr, 4, &state);

        if (bytes == 0) {
            // 解析到了 '\0'
            break;
        } else if (bytes == (size_t)-1) {
            // 遇到了无效的 UTF-8 编码序列
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error: Invalid UTF-8 sequence starting at byte index %lld\n", ptr - utf8_string);
            break;
        } else if (bytes == (size_t)-2) {
            // 序列不完整（输入字符串非预期截断）
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error: Incomplete UTF-8 sequence at end of string\n");
            break;
        } else if (bytes == (size_t)-3) {
            // 处理多 char8_t 产生单个 char32_t 的后续状态（在极少数复杂 shift 状态下触发）
            // UTF-8 下通常不会触发，但标准规范中可能返回该值
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[%zu] Contained Code Point: U+%04X\n", index++, (unsigned int)cp);
            continue;
        } else {
            // 成功解析出 1 个字符（占用 bytes 个 char8_t 字节）
            func(ptr, bytes, data);

            // 指针前移对应的字节数
            ptr += bytes;
        }
    }
}
