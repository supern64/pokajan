cmake_minimum_required(VERSION 3.20)
project(pokajan C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# === Configuration (mirrors your .bat) ===
set(RAYLIB_PATH "D:/Programming/Other/raylib/raylib")
set(MOSQUITTO_PATH "C:/Program Files/Mosquitto/devel")

add_compile_options(-Wall -Wshadow -Wunused-parameter)
add_compile_definitions(PLATFORM_DESKTOP)

include_directories(
        ${RAYLIB_PATH}/src
        ${MOSQUITTO_PATH}
        ${CMAKE_SOURCE_DIR}/incl
)

link_directories(
        ${CMAKE_SOURCE_DIR}       # for a prebuilt libraylib.a sitting in project root
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
        src/scene/overlay_card_instructions.c
        src/scene/scene_manager.c
        src/utils/text.c
        src/utils/input.c
)

set(PLAYER_UNIT_SIM_SOURCES
        src/player_unit_sim.c
        src/component/component_card.c
)

add_executable(pokajan ${POKAJAN_SOURCES} ${CJSON_SOURCES})
add_executable(player_unit_sim ${PLAYER_UNIT_SIM_SOURCES} ${CJSON_SOURCES})
set_target_properties(pokajan PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set_target_properties(player_unit_sim PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

foreach(tgt pokajan player_unit_sim)
    target_link_libraries(${tgt} PRIVATE raylib opengl32 gdi32 winmm shcore mosquitto)
    target_link_options(${tgt} PRIVATE -static -s)
endforeach()