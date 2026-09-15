# ========== SDL 系列运行期依赖清单（唯一事实来源） ==========
# 指向 vcpkg 提供的「具体共享库导入目标」而非 SDL3::SDL3 这类别名：别名自身
# 没有产物文件，$<TARGET_RUNTIME_DLLS:...> 从它推不出 DLL 路径。
# 各目标内部已按配置带好 IMPORTED_LOCATION_<CONFIG>，所以这里写不带配置后缀的
# 通用名字即可，CMake 会按当前构建类型挑对应产物（debug 下 libpng 是
# libpng16d.dll，而 SDL3_image.dll 仍然不带 d）。
# 开发时可在配置期覆盖，例如只想拷 SDL3：
#   -DMAPLE_RUNTIME_DEPS=SDL3::SDL3-shared
set(MAPLE_RUNTIME_DEPS
    "SDL3::SDL3-shared"
    "SDL3_image::SDL3_image-shared"
    "SDL3_ttf::SDL3_ttf-shared"
    CACHE STRING "Imported shared-library targets whose DLLs are deployed next to the executables")

# ========== 把运行期依赖拷到输出目录 ==========
# 有意使用生成器表达式而不是写死 "SDL3.dll"：不同配置下 vcpkg 产出的文件名并
# 不一致，手写死名字迟早会错。闭包（libpng / freetype / zlib / zlib1 等）由
# CMake 依据导入目标的 IMPORTED_* 关系自行推导。
function(target_deploy_runtime_deps target_name)
    if(NOT WIN32 OR NOT MAPLE_RUNTIME_DEPS)
        return()
    endif()

    set(deployable)
    foreach(dep IN LISTS MAPLE_RUNTIME_DEPS)
        if(TARGET ${dep})
            list(APPEND deployable ${dep})
        else()
            message(STATUS "Runtime dep '${dep}' is not available, skipped")
        endif()
    endforeach()

    foreach(dep IN LISTS deployable)
        target_link_libraries(${target_name} PRIVATE ${dep})
    endforeach()

    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${target_name}>
            "$<TARGET_FILE_DIR:${target_name}>"
        COMMAND_EXPAND_LISTS
        COMMENT "Deploying runtime DLLs for ${target_name}")
endfunction()

# ========== 链接 SDL3 ==========
function(target_link_sdl3 target_name)
    target_link_libraries(${target_name} PRIVATE
      SDL3::SDL3
      $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,
          SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>
      SDL3_ttf::SDL3_ttf)

    if(WIN32)
        target_link_libraries(${target_name} PRIVATE psapi)
    endif()
    target_deploy_runtime_deps(${target_name})
endfunction()

# ========== Demo 函数 ==========
function(add_demo demo_name)
    file(GLOB_RECURSE DEMO_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/demos/${demo_name}/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/demos/${demo_name}/**/*.c")
    if(NOT DEMO_SOURCES)
        message(FATAL_ERROR "DEMO '${demo_name}' HAS NO .c SOURCES")
    else()
        message(STATUS "DEMO '${demo_name}' SOURCES: ${DEMO_SOURCES}")
    endif()
    add_executable(${demo_name} ${DEMO_SOURCES})
    target_include_directories(${demo_name} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/demos/${demo_name}/include")
    target_compile_definitions(${demo_name} PRIVATE
        "ASSETS_PATH=u8\"assets/${demo_name}/\""
        "DEMO_NAME=u8\"${demo_name}\"")
    target_link_libraries(${demo_name} PRIVATE maple_core)
    target_link_sdl3(${demo_name})

    # only copy engine's(common for use) and demo's own
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets/${demo_name}")
        add_custom_command(TARGET ${demo_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                "${CMAKE_SOURCE_DIR}/assets/engine"
                "$<TARGET_FILE_DIR:${demo_name}>/assets/engine"
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                "${CMAKE_SOURCE_DIR}/assets/${demo_name}"
                "$<TARGET_FILE_DIR:${demo_name}>/assets/${demo_name}"
            COMMENT "Copying assets for ${demo_name} (only changed files)...")
    endif()

    add_custom_target(run_${demo_name}
        COMMAND ${demo_name}
        WORKING_DIRECTORY $<TARGET_FILE_DIR:${demo_name}>
        DEPENDS ${demo_name}
        USES_TERMINAL)
    # 用法: cmake --build build/debug --target run_demo1
endfunction()
