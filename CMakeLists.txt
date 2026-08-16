cmake_minimum_required(VERSION 3.26)

project(maple-leaf C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # 方便语言服务器检查

# ========== SDL3 配置 ==========
find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_image REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)


# ========== stb ==========
set(STB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/stb")

# ========== 引擎核心 ==========
file(GLOB_RECURSE ENGINE_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)
add_library(maple_core STATIC ${ENGINE_SOURCES})

target_include_directories(maple_core PUBLIC
    ${STB_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# SDL3 的 include 由 target 自动处理，无需手动添加

# 资源路径宏
target_compile_definitions(maple_core PUBLIC
    "ASSETS_PATH=\"assets/\""
    "WIN_W=1920"
    "WIN_H=1080"
    "STB_DS_IMPLEMENTATION"
    "DEBUG"
)

# ========== 链接 SDL3 ==========
function(target_link_sdl3 target_name)
    target_link_libraries(${target_name} PRIVATE
      SDL3::SDL3
      $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>
      SDL3_ttf::SDL3_ttf)

    # Windows 自动复制 DLL（SDL3 会生成 SDL3.dll）
    if(WIN32)
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:SDL3::SDL3>
                $<TARGET_FILE_DIR:${target_name}>
            COMMENT "Copying SDL3.dll to output directory"
        )
    endif()
endfunction()

# ========== Demo 函数 ==========
function(add_demo demo_name)
    file(GLOB_RECURSE DEMO_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/demos/${demo_name}/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/demos/${demo_name}/**/*.c"
    )
    message(STATUS "DEMO '${demo_name}' SOURCES: ${DEMO_SOURCES}")
    add_executable(${demo_name} ${DEMO_SOURCES})
    target_link_libraries(${demo_name} PRIVATE maple_core)
    target_link_sdl3(${demo_name})

    add_custom_command(TARGET ${demo_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:${demo_name}>/assets"
        COMMENT "Copying assets for ${demo_name} (only changed files)..."
    )
endfunction()

# 添加 demos
# add_demo(demo1)
# ... 其他 demo
