# ========== 链接 SDL3 ==========
function(target_link_sdl3 target_name)
    target_link_libraries(${target_name} PRIVATE
      SDL3::SDL3
      $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,
          SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>
      SDL3_ttf::SDL3_ttf)

    # Windows 自动复制 DLL（SDL3 会生成 SDL3.dll）
    if(WIN32)
        target_link_libraries(${target_name} PRIVATE psapi)
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
