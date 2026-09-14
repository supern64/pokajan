cmake_minimum_required(VERSION 3.20)
project(pokajan C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Default to standard installation locations
set(RAYLIB_PATH "C:/raylib/raylib")
set(MOSQUITTO_PATH "C:/Program Files/Mosquitto/devel")

add_compile_options(-Wall -Wshadow -Wunused-parameter)
add_compile_definitions(PLATFORM_DESKTOP)

include_directories(
        ${RAYLIB_PATH}/src
        ${MOSQUITTO_PATH}
        ${CMAKE_SOURCE_DIR}/incl
)

link_directories(
        ${CMAKE_SOURCE_DIR}
        ${MOSQUITTO_PATH}
)

set(CJSON_SOURCES incl/cjson/cjson.c)

set(POKAJAN_SOURCES
        src/main.c
        src/component/component_card.c
        src/component/component_hud.c
        src/sound/sound.c
        src/network/network.c
        src/network/bridge.c
        src/pokajan_core/cards.c
        src/pokajan_core/pokajan.c
        src/scene/scene_title.c
        src/scene/scene_game.c
        src/scene/scene_setup.c
        src/scene/overlay_card_instructions.c
        src/scene/scene_manager.c
        src/utils/text.c
        src/utils/input.c
)

set(PLAYER_UNIT_SIM_SOURCES
        src/player_unit_sim.c
        src/component/component_card.c
)

option(BUILD_POKAJAN "Build main executable" ON)
option(BUILD_PLAYER_UNIT_SIM "Build player unit simulator executable" ON)
option(BUILD_DRM "Build the DRM variant" OFF)

if (BUILD_DRM)
    set(BUILD_POKAJAN OFF)
    set(BUILD_PLAYER_UNIT_SIM OFF)
endif()

set(DESKTOP_TARGETS "")

if(BUILD_POKAJAN)
    add_executable(pokajan ${POKAJAN_SOURCES} ${CJSON_SOURCES})
    list(APPEND DESKTOP_TARGETS pokajan)
endif()

if(BUILD_PLAYER_UNIT_SIM)
    add_executable(player_unit_sim ${PLAYER_UNIT_SIM_SOURCES} ${CJSON_SOURCES})
    list(APPEND DESKTOP_TARGETS player_unit_sim)
endif()

foreach(tgt ${DESKTOP_TARGETS})
    set_target_properties(${tgt} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
    target_link_libraries(${tgt} PRIVATE raylib opengl32 gdi32 winmm shcore mosquitto)
    target_link_options(${tgt} PRIVATE -static -s)
endforeach()

if(BUILD_DRM)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
        message(FATAL_ERROR "BUILD_DRM_TARGET requires a Linux/ARM cross-compiler toolchain file")
    endif()

    set(RAYLIB_DRM_PATH "${CMAKE_SOURCE_DIR}/../raylib/src")

    add_executable(pokajan_drm ${POKAJAN_SOURCES})

    target_include_directories(pokajan_drm PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${RAYLIB_DRM_PATH}
    )

    target_compile_definitions(pokajan_drm PRIVATE
        PLATFORM_DRM
        $<$<CONFIG:Debug>:DEBUG>
        $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    )

    target_link_libraries(pokajan_drm PRIVATE raylib m pthread GLESv2 EGL gbm drm)
endif()