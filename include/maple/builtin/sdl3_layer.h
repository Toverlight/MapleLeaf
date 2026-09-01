#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_ttf.h>
#include <maple/basic_types.h>
#include <maple/tools.h>

typedef TTF_Font* FontHandle;
typedef SDL_Texture* TextureHandle;
typedef SDL_Window* WindowHandle;
typedef SDL_Renderer* RendererHandle;

typedef struct FontEntry {
    const char* key; // key=u8"<font_path>|<size>"
    FontHandle value;
} FontEntry, *FontReg;

DECLARE_HACKER_COPIED(FontReg, font_reg)

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
// 带缓存机制的 字体 加载
FontHandle maple_load_font(const utf8* utf8_font_path, u32 ptsize);
// 带缓存机制的 utf8 tile 加载
void maple_load_utf8_tile(const utf8* utf8_char, usize bytes, void* data);

typedef void(*Utf8CharFn)(const utf8* utf8_char, usize bytes, void* data);

void utf8_iter_string(const utf8 utf8_string[], Utf8CharFn func, void* data);

// log enclosure

// 作为SDL_Priority的子集，可安全扩张转换，且起到封装作用
typedef enum LogPriority {
    LogPriority_Trace = 1,
    LogPriority_Verbose,
    LogPriority_Debug,
    LogPriority_Info,
    LogPriority_Warn,
    LogPriority_Error,
    LogPriority_Critical,
} LogPriority;

void log_set_console_priority(LogPriority priority);
void log_set_file_priority(LogPriority priority);
void log_init(const utf8* utf8_file_path);
void log_quit(void);
void log_internal(int category, LogPriority priority, const char* file, const char* func, int line, const utf8* fmt, ...);

#define LOG_TRACE(fmt, ...) log_internal(SDL_LOG_CATEGORY_APPLICATION, LogPriority_Trace, __FILE_NAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_VERBOSE(fmt, ...) log_internal(SDL_LOG_CATEGORY_APPLICATION, LogPriority_Verbose, __FILE_NAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) log_internal(SDL_LOG_CATEGORY_APPLICATION, LogPriority_Debug, __FILE_NAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) log_internal(SDL_LOG_CATEGORY_APPLICATION, LogPriority_Info, __FILE_NAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) log_internal(SDL_LOG_CATEGORY_APPLICATION, LogPriority_Warn, __FILE_NAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) log_internal(SDL_LOG_CATEGORY_APPLICATION, LogPriority_Error, __FILE_NAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) log_internal(SDL_LOG_CATEGORY_APPLICATION, LogPriority_Critical, __FILE_NAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_TRACE_ONCE(fmt, ...) do { \
    static bool flag = true; \
    if (flag) { \
        flag = false; \
        LOG_TRACE((fmt), ##__VA_ARGS__); \
    } \
} while(0)
#define LOG_VERBOSE_ONCE(fmt, ...) do { \
    static bool flag = true; \
    if (flag) { \
        flag = false; \
        LOG_VERBOSE((fmt), ##__VA_ARGS__); \
    } \
} while(0)
#define LOG_DEBUG_ONCE(fmt, ...) do { \
    static bool flag = true; \
    if (flag) { \
        flag = false; \
        LOG_DEBUG((fmt), ##__VA_ARGS__); \
    } \
} while(0)
#define LOG_INFO_ONCE(fmt, ...) do { \
    static bool flag = true; \
    if (flag) { \
        flag = false; \
        LOG_INFO((fmt), ##__VA_ARGS__); \
    } \
} while(0)
#define LOG_WARN_ONCE(fmt, ...) do { \
    static bool flag = true; \
    if (flag) { \
        flag = false; \
        LOG_WARN((fmt), ##__VA_ARGS__); \
    } \
} while(0)
#define LOG_ERROR_ONCE(fmt, ...) do { \
    static bool flag = true; \
    if (flag) { \
        flag = false; \
        LOG_ERROR((fmt), ##__VA_ARGS__); \
    } \
} while(0)
#define LOG_CRITICAL_ONCE(fmt, ...) do { \
    static bool flag = true; \
    if (flag) { \
        flag = false; \
        LOG_CRITICAL((fmt), ##__VA_ARGS__); \
    } \
} while(0)

#define LOG_TRACE_LIMITED(max_times, fmt, ...) do { \
    static i32 counter = (max_times); \
    if (counter > 0) { \
        counter--; \
        LOG_TRACE((fmt), ##__VA_ARGS__); \
    } \
} while(0)
#define LOG_VERBOSE_LIMITED(max_times, fmt, ...) do { \
    static i32 counter = (max_times); \
    if (counter > 0) { \
        counter--; \
        LOG_VERBOSE((fmt), ##__VA_ARGS__); \
    } \
} while(0)
#define LOG_DEBUG_LIMITED(max_times, fmt, ...) do { \
    static i32 counter = (max_times); \
    if (counter > 0) { \
        counter--; \
        LOG_DEBUG((fmt), ##__VA_ARGS__); \
    } \
} while(0)
#define LOG_INFO_LIMITED(max_times, fmt, ...) do { \
    static i32 counter = (max_times); \
    if (counter > 0) { \
        counter--; \
        LOG_INFO((fmt), ##__VA_ARGS__); \
    } \
} while(0)
#define LOG_WARN_LIMITED(max_times, fmt, ...) do { \
    static i32 counter = (max_times); \
    if (counter > 0) { \
        counter--; \
        LOG_WARN((fmt), ##__VA_ARGS__); \
    } \
} while(0)
#define LOG_ERROR_LIMITED(max_times, fmt, ...) do { \
    static i32 counter = (max_times); \
    if (counter > 0) { \
        counter--; \
        LOG_ERROR((fmt), ##__VA_ARGS__); \
    } \
} while(0)
#define LOG_CRITICAL_LIMITED(max_times, fmt, ...) do { \
    static i32 counter = (max_times); \
    if (counter > 0) { \
        counter--; \
        LOG_CRITICAL((fmt), ##__VA_ARGS__); \
    } \
} while(0)
