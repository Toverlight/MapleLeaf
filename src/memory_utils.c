#include <maple/memory_utils.h>
#include <maple/builtin/sdl3_layer.h>

// 平台特性宏判断与头文件引入
#if defined(SDL_PLATFORM_WINDOWS)
    #include <windows.h>
    #include <psapi.h>
#elif defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_ANDROID)
    #include <unistd.h>
#elif defined(SDL_PLATFORM_APPLE)
    #include <mach/mach.h>
#endif

size_t get_current_rss(void) {
#if defined(SDL_PLATFORM_WINDOWS)
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return (size_t)pmc.WorkingSetSize;
    }
#elif defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_ANDROID)
    // 通过读取 /proc/self/statm 获取页数，第二项为常驻内存页数
    if (FILE* file = fopen("/proc/self/statm", "r")) {
        long pages = 0;
        // 只读取第二个参数 (rss)
        if (fscanf(file, "%*s %ld", &pages) == 1) {
            fclose(file);
            return (size_t)pages * (size_t)sysconf(_SC_PAGESIZE);
        }
        fclose(file);
    }
#elif defined(SDL_PLATFORM_APPLE)
    struct mach_task_basic_info info;
    mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &infoCount) == KERN_SUCCESS) {
        return (size_t)info.resident_size;
    }
#endif
    return 0; // 暂不支持或获取失败的平台
}

void print_program_memory(void) {
    size_t rss_bytes = get_current_rss();
    if (rss_bytes == 0) {
        LOG_ERROR(u8"Cannot get runtime memory of the app at this platform");
        return;
    }

    double mem_mb = (double)rss_bytes / (1024.0 * 1024.0);
    // 使用 SDL3 的统一日志输出函数，带格式化
    LOG_DEBUG(u8"Current RSS: %.2f MB", mem_mb);
}
