cmake_minimum_required(VERSION 3.20)
project(pokajan C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)


if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(BUILD_POKAJAN "Build main executable" ON)
option(BUILD_PLAYER_UNIT_SIM "Build player unit simulator executable" ON)
option(BUILD_DRM "Build the DRM variant" OFF)

# On Windows the raylib/mosquitto installers already give us binaries.
# Everywhere else, build raylib from source.
if(WIN32)
    set(_raylib_fetch_default OFF)
else()
    set(_raylib_fetch_default ON)
endif()
option(FETCH_RAYLIB "Download and build raylib from source" ${_raylib_fetch_default})

set(RAYLIB_VERSION "6.0" CACHE STRING "raylib git tag to fetch")
set(RAYLIB_PATH "C:/raylib/raylib" CACHE PATH "Prebuilt raylib location")
set(MOSQUITTO_PATH "C:/Program Files/Mosquitto/devel" CACHE PATH "Mosquitto devel location")

if(BUILD_DRM)
    set(BUILD_POKAJAN OFF)
    set(BUILD_PLAYER_UNIT_SIM OFF)
endif()

# ---------------------------------------------------------------------------
# raylib
# ---------------------------------------------------------------------------
if(FETCH_RAYLIB)
    include(FetchContent)

    set(FETCHCONTENT_QUIET OFF)
    
    # These have to be set before MakeAvailable - raylib reads them as its own
    # cache options. FORCE is needed because raylib declares them itself.
    set(BUILD_EXAMPLES    OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES       OFF CACHE BOOL "" FORCE)
    set(CUSTOMIZE_BUILD   OFF CACHE BOOL "" FORCE)
    set(USE_EXTERNAL_GLFW OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

    if(BUILD_DRM)
        set(PLATFORM        "DRM"     CACHE STRING "" FORCE)
        set(OPENGL_VERSION  "ES 2.0"  CACHE STRING "" FORCE)
    else()
        set(PLATFORM        "Desktop" CACHE STRING "" FORCE)
    endif()

    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG        ${RAYLIB_VERSION}
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
    )
    FetchContent_MakeAvailable(raylib)
else()
    # Prebuilt static lib - wrapped in an imported target so the rest of this
    # file can treat it exactly like the fetched one.
    add_library(raylib STATIC IMPORTED GLOBAL)
    set_target_properties(raylib PROPERTIES
        IMPORTED_LOCATION             "${RAYLIB_PATH}/src/libraylib.a"
        INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_PATH}/src")
    if(WIN32)
        target_link_libraries(raylib INTERFACE opengl32 gdi32 winmm shcore)
    elseif(BUILD_DRM)
        target_link_libraries(raylib INTERFACE GLESv2 EGL gbm drm)
    endif()
endif()

# ---------------------------------------------------------------------------
# mosquitto
# ---------------------------------------------------------------------------
add_library(pokajan_mosquitto INTERFACE)

if(WIN32)
    target_include_directories(pokajan_mosquitto INTERFACE "${MOSQUITTO_PATH}")
    target_link_directories(pokajan_mosquitto INTERFACE "${MOSQUITTO_PATH}")
    target_link_libraries(pokajan_mosquitto INTERFACE mosquitto)
else()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(MOSQUITTO QUIET IMPORTED_TARGET libmosquitto)
    endif()

    if(TARGET PkgConfig::MOSQUITTO)
        target_link_libraries(pokajan_mosquitto INTERFACE PkgConfig::MOSQUITTO)
    else()
        find_path(MOSQUITTO_INCLUDE_DIR mosquitto.h)
        find_library(MOSQUITTO_LIBRARY mosquitto)
        if(NOT MOSQUITTO_INCLUDE_DIR OR NOT MOSQUITTO_LIBRARY)
            message(FATAL_ERROR
                "libmosquitto not found. Install libmosquitto-dev (Debian/Ubuntu/Raspberry Pi OS) "
                "or point CMAKE_PREFIX_PATH at your install.")
        endif()
        target_include_directories(pokajan_mosquitto INTERFACE "${MOSQUITTO_INCLUDE_DIR}")
        target_link_libraries(pokajan_mosquitto INTERFACE "${MOSQUITTO_LIBRARY}")
    endif()
endif()

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------
set(CJSON_SOURCES incl/cJSON/cJSON.c)

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

# Shared per-target setup. Note this is per-target, not global: raylib is part
# of our build tree now and must not inherit our warning flags or our
# PLATFORM_* defines.
function(pokajan_configure_target tgt)
    target_include_directories(${tgt} PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/incl
    )
    target_compile_options(${tgt} PRIVATE
            $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wshadow;-Wunused-parameter>
    )
    target_link_libraries(${tgt} PRIVATE raylib pokajan_mosquitto)
    if(UNIX)
        target_link_libraries(${tgt} PRIVATE m pthread dl)
    endif()
endfunction()

# ---------------------------------------------------------------------------
# Desktop targets
# ---------------------------------------------------------------------------
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
    pokajan_configure_target(${tgt})
    target_compile_definitions(${tgt} PRIVATE PLATFORM_DESKTOP)
    if(WIN32)
        # -static only makes sense against the MinGW runtime; on Linux it would
        # try to statically link X11/GL and fail.
        target_link_options(${tgt} PRIVATE -static -s)
    endif()
endforeach()

# ---------------------------------------------------------------------------
# DRM target
# ---------------------------------------------------------------------------
if(BUILD_DRM)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
        message(FATAL_ERROR "BUILD_DRM requires a Linux target (native Pi build or an ARM cross toolchain file)")
    endif()

    add_executable(pokajan_drm ${POKAJAN_SOURCES} ${CJSON_SOURCES})
    set_target_properties(pokajan_drm PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
    pokajan_configure_target(pokajan_drm)

    target_compile_definitions(pokajan_drm PRIVATE
            PLATFORM_DRM
            $<$<CONFIG:Debug>:DEBUG>
            $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    )
endif()