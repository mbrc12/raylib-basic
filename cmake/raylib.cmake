# Raylib is staged from the sibling ext/raylib repo.
# This file only selects the staged artifact and wires the executable to it.

function(raylib_stage_artifact kind)
    # Copy the selected dist tree into the local build tree so CMake can import it.
    set(RAYLIB_STAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/raylib")
    file(REMOVE_RECURSE "${RAYLIB_STAGE_DIR}")
    file(COPY "${RAYLIB_ARTIFACTS_DIR}/${kind}/include/" DESTINATION "${RAYLIB_STAGE_DIR}/include")
    file(COPY "${RAYLIB_ARTIFACTS_DIR}/${kind}/lib/" DESTINATION "${RAYLIB_STAGE_DIR}/lib")

    add_library(raylib STATIC IMPORTED GLOBAL)
    set_target_properties(raylib PROPERTIES
        IMPORTED_LOCATION "${RAYLIB_STAGE_DIR}/lib/libraylib.a"
        INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_STAGE_DIR}/include"
    )
endfunction()

function(raylib_import_library name kind path)
    add_library(${name} ${kind} IMPORTED GLOBAL)
    set_target_properties(${name} PROPERTIES
        IMPORTED_LOCATION "${path}"
    )
endfunction()

function(raylib_finish_native)
    # Native builds share the same output layout and asset copy step.
    set_target_properties(${GAME_NAME} PROPERTIES
        BUILD_RPATH "@executable_path"
        INSTALL_RPATH "@executable_path"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_DIR}/macos"
    )

    raylib_assets_target(${ARGN})
endfunction()

function(raylib_link_frameworks target)
    foreach(framework IN LISTS ARGN)
        target_link_libraries(${target} PRIVATE "-framework ${framework}")
    endforeach()
endfunction()

function(setup_raylib_target)
    if(EMSCRIPTEN)
        raylib_setup_web()
    elseif(GRAPHICS STREQUAL "opengl")
        raylib_setup_macos_opengl()
    elseif(GRAPHICS STREQUAL "angle")
        raylib_setup_macos_angle()
    elseif(GRAPHICS STREQUAL "sdl3-angle")
        raylib_setup_macos_sdl3_angle()
    else()
        message(FATAL_ERROR "GRAPHICS must be 'opengl', 'angle', or 'sdl3-angle'")
    endif()
endfunction()

function(raylib_setup_web)
    # Web uses the dedicated ES3 artifact and a small shell wrapper.
    raylib_stage_artifact("web-es3")

    set(PROJECT_WEB_SHELL_TEMPLATE "${PROJECT_ASSETS_DIR}/templates/web-shell.html.in")
    set(PROJECT_WEB_SHELL "${CMAKE_CURRENT_BINARY_DIR}/web-shell.html")
    configure_file("${PROJECT_WEB_SHELL_TEMPLATE}" "${PROJECT_WEB_SHELL}" @ONLY)

    set_target_properties(${GAME_NAME} PROPERTIES
        SUFFIX ".html"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_DIR}/web"
        LINK_DEPENDS "${PROJECT_WEB_SHELL};${PROJECT_WEB_SHELL_TEMPLATE}"
    )

    target_link_libraries(${GAME_NAME} PRIVATE raylib)
    target_link_options(${GAME_NAME} PRIVATE
        "SHELL:-sWASM=1"
        "SHELL:-sUSE_GLFW=3"
        "SHELL:-sMIN_WEBGL_VERSION=2"
        "SHELL:-sMAX_WEBGL_VERSION=2"
        "SHELL:-sFULL_ES3=1"
        "SHELL:-sALLOW_MEMORY_GROWTH=1"
        "SHELL:-sASSERTIONS=1"
        "SHELL:--shell-file ${PROJECT_WEB_SHELL}"
        "SHELL:--preload-file ${PROJECT_ASSETS_DIR}@assets"
    )
endfunction()

function(raylib_setup_macos_opengl)
    # Native OpenGL is the plain GLFW-backed macOS path.
    raylib_stage_artifact("macos-opengl")

    target_link_libraries(${GAME_NAME} PRIVATE raylib)
    raylib_link_frameworks(${GAME_NAME}
        OpenGL
        Cocoa
        IOKit
        CoreVideo
        CoreFoundation
    )

    raylib_finish_native()
endfunction()

function(raylib_setup_macos_angle)
    # ANGLE keeps the renderer on GLES 3 while still using the native windowing stack.
    raylib_stage_artifact("macos-angle")
    target_compile_definitions(${GAME_NAME} PRIVATE PROJECT_USE_GLSL_ES)

    set(RAYLIB_STAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/raylib")
    raylib_import_library(raylib_egl SHARED "${RAYLIB_STAGE_DIR}/lib/libEGL.dylib")
    raylib_import_library(raylib_glesv2 SHARED "${RAYLIB_STAGE_DIR}/lib/libGLESv2.dylib")

    target_link_libraries(${GAME_NAME} PRIVATE raylib raylib_glesv2 raylib_egl)
    raylib_link_frameworks(${GAME_NAME}
        Cocoa
        IOKit
        CoreVideo
        CoreFoundation
        Metal
        Foundation
        CoreGraphics
        IOSurface
        QuartzCore
        CoreServices
    )

    raylib_finish_native(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${RAYLIB_STAGE_DIR}/lib/libEGL.dylib"
                "$<TARGET_FILE_DIR:${GAME_NAME}>/libEGL.dylib"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${RAYLIB_STAGE_DIR}/lib/libGLESv2.dylib"
                "$<TARGET_FILE_DIR:${GAME_NAME}>/libGLESv2.dylib"
    )
endfunction()

function(raylib_setup_macos_sdl3_angle)
    # SDL3 supplies input/windowing, ANGLE supplies the GLES 3 renderer.
    raylib_stage_artifact("macos-sdl3-angle")
    target_compile_definitions(${GAME_NAME} PRIVATE PROJECT_USE_GLSL_ES)

    set(RAYLIB_STAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/raylib")
    raylib_import_library(raylib_egl SHARED "${RAYLIB_STAGE_DIR}/lib/libEGL.dylib")
    raylib_import_library(raylib_glesv2 SHARED "${RAYLIB_STAGE_DIR}/lib/libGLESv2.dylib")
    raylib_import_library(raylib_sdl3 STATIC "${RAYLIB_STAGE_DIR}/lib/libSDL3.a")

    target_link_libraries(${GAME_NAME} PRIVATE raylib raylib_sdl3 raylib_glesv2 raylib_egl)
    raylib_link_frameworks(${GAME_NAME}
        Cocoa
        IOKit
        CoreVideo
        CoreFoundation
        AVFoundation
        AudioToolbox
        CoreAudio
        CoreMedia
        CoreHaptics
        GameController
        Carbon
        ForceFeedback
        UniformTypeIdentifiers
        Metal
        Foundation
        CoreGraphics
        IOSurface
        QuartzCore
        CoreServices
    )

    raylib_finish_native(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${RAYLIB_STAGE_DIR}/lib/libEGL.dylib"
                "$<TARGET_FILE_DIR:${GAME_NAME}>/libEGL.dylib"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${RAYLIB_STAGE_DIR}/lib/libGLESv2.dylib"
                "$<TARGET_FILE_DIR:${GAME_NAME}>/libGLESv2.dylib"
    )
endfunction()

function(raylib_assets_target)
    # Copy the full assets tree next to the executable and allow extra commands per backend.
    file(GLOB_RECURSE PROJECT_ASSET_FILES "${PROJECT_ASSETS_DIR}/*")
    add_custom_target(copy_assets ALL
        COMMAND "${CMAKE_COMMAND}" -E remove_directory
                "$<TARGET_FILE_DIR:${GAME_NAME}>/assets"
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${PROJECT_ASSETS_DIR}"
                "$<TARGET_FILE_DIR:${GAME_NAME}>/assets"
        ${ARGN}
        DEPENDS ${PROJECT_ASSET_FILES}
    )
    add_dependencies(${GAME_NAME} copy_assets)
endfunction()
