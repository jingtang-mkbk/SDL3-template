cmake_minimum_required(VERSION 3.16)
project(SDL3-template LANGUAGES C)

# 输出目录：确保 DLL 等文件自动放在构建目录下
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# ---- 子模块开关 ----
option(USE_SDL_IMAGE "Build with SDL_image" ON)
option(USE_SDL_MIXER "Build with SDL_mixer" ON)
option(USE_SDL_TTF   "Build with SDL_ttf"   ON)

# ---- SDL3 核心 ----
# add_subdirectory 方式，无需手动编译安装
add_subdirectory(three-party/SDL EXCLUDE_FROM_ALL)

# ---- SDL_image（可选）----
if(USE_SDL_IMAGE)
    # SDL_image 依赖 SDL3::SDL3，需要在 SDL 之后 add_subdirectory
    add_subdirectory(three-party/SDL_image EXCLUDE_FROM_ALL)
endif()

# ---- SDL_mixer（可选）----
if(USE_SDL_MIXER)
    add_subdirectory(three-party/SDL_mixer EXCLUDE_FROM_ALL)
endif()

# ---- SDL_ttf（可选）----
if(USE_SDL_TTF)
    add_subdirectory(three-party/SDL_ttf EXCLUDE_FROM_ALL)
endif()

# ---- 主程序 ----
add_executable(main WIN32
    src/main.c
    src/utils/utils.c
    src/manager/manager.c
    src/manager/font_manager.c
    src/manager/image_manager.c
    src/manager/music_manager.c
    src/render/render.c
    src/event/event.c
    src/ui/text.c
    src/ui/image.c
    src/scenes/main_scene/main_scene.c
    src/scenes/minesweeper_scene/minesweeper_scene.c
    src/platform/platform.c
    three-party/cJSON/cJSON.c
)

target_include_directories(main PRIVATE src three-party three-party/cJSON)

# 链接 SDL3
target_link_libraries(main PRIVATE SDL3::SDL3)

# 链接可选扩展
if(USE_SDL_IMAGE)
    target_link_libraries(main PRIVATE SDL3_image::SDL3_image)
endif()
if(USE_SDL_MIXER)
    target_link_libraries(main PRIVATE SDL3_mixer::SDL3_mixer)
endif()
if(USE_SDL_TTF)
    target_link_libraries(main PRIVATE SDL3_ttf::SDL3_ttf)
endif()

# ---- 复制 DLL 和资源到输出目录 ----
add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        "$<TARGET_FILE_DIR:main>"
)
# 每次 build 都同步 assets（即使 main 没重新编译）
add_custom_target(sync_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/assets"
        "$<TARGET_FILE_DIR:main>/assets"
    DEPENDS main
)
if(USE_SDL_IMAGE)
    add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3_image::SDL3_image>
            "$<TARGET_FILE_DIR:main>"
    )
endif()
if(USE_SDL_MIXER)
    add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3_mixer::SDL3_mixer>
            "$<TARGET_FILE_DIR:main>"
    )
endif()
