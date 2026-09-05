cmake_minimum_required(VERSION 3.26)

project(maple-leaf C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # 方便语言服务器检查

# ========== SDL3 配置 ==========
find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_image REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)


# ========== thirdparty ==========
set(STB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/stb")
set(SDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/sdl")

# ========== 引擎核心 ==========
file(GLOB_RECURSE ENGINE_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.c"
)
add_library(maple_core STATIC ${ENGINE_SOURCES})

target_include_directories(maple_core PUBLIC
    ${STB_DIR}
    ${SDL_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# SDL3 的 include 由 target 自动处理，无需手动添加

# 资源路径宏
target_compile_definitions(maple_core PUBLIC
    "W_1080P=1920"
    "W_720P=1280"
    "W_360P=640"
    "H_1080P=1080"
    "H_720P=720"
    "H_360P=360"
    "MAPLE_ASSETS_PATH=u8\"assets/engine/\""
    "$<$<CONFIG:Debug>:DEBUG>"
)

include(cmake/MapleDemo.cmake)

# 添加 demos
add_demo(demo1)
# ... 其他 demo
