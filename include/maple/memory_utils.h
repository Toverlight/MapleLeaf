#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <SDL3/SDL.h>

/**
 * 获取当前程序运行时的物理内存占用（常驻内存 Resident Set Size）
 * @return 占用字节数 (bytes)，若获取失败则返回 0
 */
size_t get_current_rss(void);

/**
 * 友好地格式化并打印当前内存占用
 */
void print_program_memory(void);
