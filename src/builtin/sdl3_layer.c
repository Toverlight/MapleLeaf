#include <maple/builtin/sdl3_layer.h>
#include <maple/application.h>
#include <stb_ds.h>
#include <stdio.h>
#include <time.h>

static FontReg font_reg = nullptr;
static Utf8TileReg utf8_tile_reg = nullptr;

IMPL_HACKER_COPIED(FontReg, font_reg)
IMPL_HACKER_COPIED(Utf8TileReg, utf8_tile_reg)

FontHandle maple_load_font(const utf8* utf8_font_path, u32 ptsize) {
    DLOG_LIMITED(20, u8"Loading font '%s' ...", (const char*)utf8_font_path);
    // TODO 考虑ptsize判断，防止加载过大的字体
    char key[256];
    i32 written = snprintf(key, sizeof(key), "%s|%03d", (const char*)utf8_font_path, ptsize);
    if (written >= (i32)sizeof(key)) {
        LOG_ERROR_LIMITED(10, u8"Font load failed: Key string buffer overflow: as for font '%s'", (const char*)utf8_font_path);
        return nullptr;
    }
    isize i = shgeti(font_reg, key);
    if (i >= 0) {
        DLOG_LIMITED(10, u8"Cached font found: path|ptsize : %s", key);
        return font_reg[i].value;
    }
    TTF_Font* font = TTF_OpenFont((const char*)utf8_font_path, ptsize);
    if (!font) {
        LOG_ERROR_LIMITED(10, u8"Font load failed: %s", SDL_GetError());
        return nullptr;
    }
    FontHandle handle = font;
    DLOG_LIMITED(20, u8"Font load successfully: path|ptsize : %s", key);
    shput(font_reg, key, handle);
    return handle;
}

// TODO 对于失败的情况，可以考虑添加对应大小的占位符纹理（就像乱码文本都是不明方块字符一样）
void maple_load_utf8_tile(const utf32 cp, void* data) {
    // TODO font height 判断，防止加载过大的文本纹理
    Utf8TileItem* out_fitted = (Utf8TileItem*)data;
    const utf8* font_name = (out_fitted->font_name) ?
        (out_fitted->font_name) :
        MAPLE_ASSET(u8"fonts/SOURCEHANSANSSC-NORMAL-2.OTF");

    char utf8_char_copied[5];
    mbstate_t state = {0};
    size_t n = c32rtomb(utf8_char_copied, cp, &state);
    if (n == (size_t)-1) {
        LOG_ERROR_LIMITED(10, u8"Code Point U+%04X cannot be represented", (u32)cp);
        return;
    }

    utf8_char_copied[n] = '\0';
    DLOG_LIMITED(10, u8"Loading utf8 tile for char '%s'...", utf8_char_copied);

    u32 key = (u32)cp;
    isize i = hmgeti(utf8_tile_reg, key);
    if (i < 0) {
        // SDL加载该utf8字符到纹理
        // 键的拼接方式：<utf8 char>|<font_height>（暂定u8'|'作为特殊字符）
        // 成功：更新reg；失败：error并返回nullptr
        // FIXME text纹理大小的分级处理由外部完成。类似mipmap多级清晰度的以后优化
        DLOG_LIMITED(10, u8"Loading utf8 tile of key '%u'...", key);
        FontHandle font_handle = maple_load_font(font_name, out_fitted->font_height);
        if (!font_handle) {
            LOG_ERROR_LIMITED(10, u8"Unable to load utf8 tile: caused by font loading failure", utf8_char_copied);
            return;
        }
        SDL_Color color = { 255, 255, 255, 255 };
        SDL_Surface* surface = TTF_RenderGlyph_Blended(font_handle, (u32)cp, color);
        if (!surface) {
            LOG_ERROR_LIMITED(100, u8"Failed to create utf8 tile '%s'", (const char*)utf8_char_copied);
            SDL_DestroySurface(surface);
            // TODO placeholder texture added
            return;
        }
        TextureHandle texture_handle = SDL_CreateTextureFromSurface(app_get()->renderer, surface);
        f32 aspect = (f32)surface->w / (f32)surface->h;
        SDL_DestroySurface(surface);
        Utf8TileValue utf8_tile_value = (Utf8TileValue){ texture_handle, aspect };
        hmput(utf8_tile_reg, key, utf8_tile_value);
        arrput(out_fitted->textures, texture_handle);
        arrput(out_fitted->aspects, aspect);
    } else {
        arrput(out_fitted->textures, utf8_tile_reg[i].value.texture);
        arrput(out_fitted->aspects, utf8_tile_reg[i].value.aspect);
    }
}

void maple_unload_fonts(void) {
    for (isize i = 0; i < shlen(font_reg); i++) {
        TTF_CloseFont(font_reg[i].value);
    }
    shfree(font_reg);
}

void maple_unload_utf8_tiles(void) {
    for (isize i = 0; i < hmlen(utf8_tile_reg); i++) {
        SDL_DestroyTexture(utf8_tile_reg[i].value.texture);
    }
    hmfree(utf8_tile_reg);
}

void utf8_iter_string(const utf8 utf8_string[], Utf8CharFn func, void* data) {
    if (!utf8_string) return;
    DLOG_ONCE(u8"Function entered");
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
            LOG_ERROR_LIMITED(100, u8"Invalid UTF-8 sequence starting at byte index %lld", ptr - utf8_string);
            break;
        } else if (bytes == (size_t)-2) {
            // 序列不完整（输入字符串非预期截断）
            LOG_ERROR_LIMITED(100, u8"Incomplete UTF-8 sequence at end of string");
            break;
        } else if (bytes == (size_t)-3) {
            // 处理多 char8_t 产生单个 char32_t 的后续状态（在极少数复杂 shift 状态下触发）
            // UTF-8 下通常不会触发，但标准规范中可能返回该值
            LOG_ERROR_LIMITED(100, u8"[%zu] Contained Code Point: U+%04X", index++, (unsigned int)cp);
            continue;
        } else {
            // 成功解析出 1 个字符（占用 bytes 个 char8_t 字节）
            func(cp, data);

            // 指针前移对应的字节数
            ptr += bytes;
        }
    }
    DLOG_ONCE(u8"Function exiting");
}

// log enclosure
// TODO log files rolling
static bool log_initialized = false;

// TODO 优化：滚动日志（指定数量和大小）
typedef struct {
    FILE* file;
    SDL_Mutex* mutex;  // SDL3 互斥锁指针
    LogPriority console_priority;
    LogPriority file_priority;
} LogContext;

static LogContext log_ctx = {
    .file = nullptr,
    .mutex = nullptr,
    .console_priority =
#ifdef DEBUG
    LogPriority_Trace,
#else
    LogPriority_Info,
#endif
    .file_priority =
#ifdef DEBUG
    LogPriority_Debug
#else
    LogPriority_Info
#endif
};

// TODO 引入多线程后加上thread信息
static void maple_log_callback(void* userdata, int category, SDL_LogPriority priority, const char* message) {
    LogContext* ctx = (LogContext*)userdata;
    if (!ctx) return;
    if (!(priority >= (SDL_LogPriority)ctx->console_priority) && !(priority >= (SDL_LogPriority)ctx->file_priority && ctx->file)) {
        return;
    }

    SDL_LockMutex(ctx->mutex);

    time_t now = time(nullptr);
    struct tm* local_time = localtime(&now);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);

    const char* priority_str = "INFO";
    switch (priority) {
        case SDL_LOG_PRIORITY_TRACE:    priority_str = "TRCE"; break;
        case SDL_LOG_PRIORITY_VERBOSE:  priority_str = "VERB"; break;
        case SDL_LOG_PRIORITY_DEBUG:    priority_str = "DEBG"; break;
        case SDL_LOG_PRIORITY_WARN:     priority_str = "WARN"; break;
        case SDL_LOG_PRIORITY_ERROR:    priority_str = "ERR "; break;
        case SDL_LOG_PRIORITY_CRITICAL: priority_str = "CRIT"; break;
        default: break;
    }

    if (priority >= (SDL_LogPriority)ctx->console_priority) {
        fprintf(stdout, "[%s] [%s] [Cat:%d] %s\n", time_str, priority_str, category, message);
#ifdef DEBUG
        fflush(stdout);
#endif
    }

    if (priority >= (SDL_LogPriority)ctx->file_priority && ctx->file) {
        fprintf(ctx->file, "[%s] [%s] [Cat:%d] %s\n", time_str, priority_str, category, message);
#ifdef DEBUG
        fflush(ctx->file);
#endif
    }

    SDL_UnlockMutex(ctx->mutex);
}

void log_set_console_priority(LogPriority priority) {
    log_ctx.console_priority = priority;
}

void log_set_file_priority(LogPriority priority) {
    log_ctx.file_priority = priority;
}

void log_init(const utf8* utf8_log_file_path) {
    if (log_initialized) {
        fprintf(stdout, "Log module hasn't been initialized");
        return;
    }
    log_ctx.mutex = SDL_CreateMutex();
    if (!log_ctx.mutex) {
        LOG_ERROR(u8"Failed to create SDL_Mutex: %s", SDL_GetError());
        return;
    }

    if (utf8_log_file_path) {
        log_ctx.file = fopen((const char*)utf8_log_file_path, "a");
        if (!log_ctx.file) {
            SDL_DestroyMutex(log_ctx.mutex);
            log_ctx.mutex = nullptr;
            LOG_ERROR(u8"Cannot open log file: %s", (const char*)utf8_log_file_path);
            return;
        }
    }
    DLOG(u8"Log file opened successfully, view here if needed: %s", (const char*)utf8_log_file_path);
    SDL_SetLogOutputFunction(maple_log_callback, &log_ctx);
    SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_TRACE);
    log_initialized = true;
}

void log_quit(void) {
    fflush(stdout);
    fflush(log_ctx.file);
    SDL_SetLogOutputFunction(nullptr, nullptr);

    // 安全释放锁和文件句柄
    if (log_ctx.mutex) {
        SDL_DestroyMutex(log_ctx.mutex);
        log_ctx.mutex = nullptr;
    }
    if (log_ctx.file) {
        fclose(log_ctx.file);
        log_ctx.file = nullptr;
    }

    log_initialized = false;
}

void log_internal(int category, LogPriority priority, const char* file, const char* func, int line, const utf8* fmt, ...) {
    char message_buf[2048];

    // 拼接文件名与行号元数据
    int prefix_len = snprintf(message_buf, sizeof(message_buf), "[%s:%s:%d] ", file, func, line);

    // 解析可变参数
    va_list args;
    va_start(args, fmt);
    vsnprintf(message_buf + prefix_len, sizeof(message_buf) - prefix_len, (const char*)fmt, args);
    va_end(args);

    // 抛给 SDL3 核心，这会自动触发 maple_log_callback
    SDL_LogMessage(category, (SDL_LogPriority)priority, "%s", message_buf);
}
