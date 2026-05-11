cmake_minimum_required(VERSION 3.25)

project(game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GAME_NAME "game" CACHE STRING "Executable basename, web output basename, and window title.")
set(RAYLIB_ARTIFACTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../ext/raylib/dist" CACHE PATH "Path to the external raylib artifact dist directory.")
set(GRAPHICS "sdl3-angle" CACHE STRING "Graphics backend on macOS: angle, sdl3-angle, or opengl")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(raylib)

set(PROJECT_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
set(PROJECT_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/bin")

add_subdirectory(deps/tomlplusplus)
if(NOT EMSCRIPTEN)
    add_subdirectory(deps/backward-cpp)
endif()

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(${GAME_NAME} ${GAME_SOURCES})
target_compile_definitions(${GAME_NAME} PRIVATE PROJECT_WINDOW_TITLE="${GAME_NAME}")
target_include_directories(${GAME_NAME} PRIVATE src)
target_link_libraries(${GAME_NAME} PRIVATE
    tomlplusplus::tomlplusplus
)
if(NOT EMSCRIPTEN)
    target_link_libraries(${GAME_NAME} PRIVATE Backward::Object)
endif()

setup_raylib_target()
