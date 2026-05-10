function(stage_raylib_artifact kind)
    set(STAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/raylib")

    file(REMOVE_RECURSE "${STAGE_DIR}")
    file(COPY "${RAYLIB_ARTIFACTS_DIR}/${kind}/include/" DESTINATION "${STAGE_DIR}/include")
    file(COPY "${RAYLIB_ARTIFACTS_DIR}/${kind}/lib/" DESTINATION "${STAGE_DIR}/lib")

    add_library(raylib STATIC IMPORTED GLOBAL)
    set_target_properties(raylib PROPERTIES
        IMPORTED_LOCATION "${STAGE_DIR}/lib/libraylib.a"
        INTERFACE_INCLUDE_DIRECTORIES "${STAGE_DIR}/include"
    )
endfunction()

function(setup_raylib_target)
    if(EMSCRIPTEN)
        _setup_raylib_web()
    elseif(GRAPHICS STREQUAL "opengl")
        _setup_raylib_opengl()
    elseif(GRAPHICS STREQUAL "angle")
        _setup_raylib_angle()
    else()
        message(FATAL_ERROR "GRAPHICS must be 'opengl' or 'angle'")
    endif()
endfunction()

function(_setup_raylib_web)
    stage_raylib_artifact("web-es3")

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

function(_setup_raylib_opengl)
    stage_raylib_artifact("macos-opengl")

    target_link_libraries(${GAME_NAME} PRIVATE raylib
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework CoreFoundation"
    )

    _setup_raylib_native()
endfunction()

function(_setup_raylib_angle)
    stage_raylib_artifact("macos-angle")

    target_compile_definitions(${GAME_NAME} PRIVATE PROJECT_USE_GLSL_ES)

    set(STAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/raylib")

    add_library(raylib_egl SHARED IMPORTED GLOBAL)
    set_target_properties(raylib_egl PROPERTIES
        IMPORTED_LOCATION "${STAGE_DIR}/lib/libEGL.dylib"
    )

    add_library(raylib_glesv2 SHARED IMPORTED GLOBAL)
    set_target_properties(raylib_glesv2 PROPERTIES
        IMPORTED_LOCATION "${STAGE_DIR}/lib/libGLESv2.dylib"
    )

    target_link_libraries(${GAME_NAME} PRIVATE raylib raylib_glesv2 raylib_egl
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework CoreFoundation"
        "-framework Metal"
        "-framework Foundation"
        "-framework CoreGraphics"
        "-framework IOSurface"
        "-framework QuartzCore"
        "-framework CoreServices"
    )

    _setup_raylib_native(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${STAGE_DIR}/lib/libEGL.dylib"
                "$<TARGET_FILE_DIR:${GAME_NAME}>/libEGL.dylib"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${STAGE_DIR}/lib/libGLESv2.dylib"
                "$<TARGET_FILE_DIR:${GAME_NAME}>/libGLESv2.dylib"
    )
endfunction()

function(_setup_raylib_native)
    set_target_properties(${GAME_NAME} PROPERTIES
        BUILD_RPATH "@executable_path"
        INSTALL_RPATH "@executable_path"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_DIR}/macos"
    )

    raylib_assets_target(${ARGN})
endfunction()

function(raylib_assets_target)
    add_custom_command(TARGET ${GAME_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E remove_directory
                "$<TARGET_FILE_DIR:${GAME_NAME}>/assets"
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${PROJECT_ASSETS_DIR}"
                "$<TARGET_FILE_DIR:${GAME_NAME}>/assets"
        ${ARGN}
    )
endfunction()
