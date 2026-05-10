#!/usr/bin/env fish

argparse 'd/debug' -- $argv; or exit 1
set -l target $argv[1]

if test -z "$target"
    echo "usage: fish build.fish [-d|--debug] [angle|opengl|web]"
    exit 1
end

set -l debug_flag ""
if set -q _flag_debug
    set debug_flag "-DCMAKE_BUILD_TYPE=Debug"
end

set -l project_root (dirname (status --current-filename))
cd "$project_root"; or exit 1

switch "$target"
    case angle
        cmake -B build/macos -DGRAPHICS=angle $debug_flag
        cmake --build build/macos --target game; or exit 1
        echo "Running: build/bin/macos/game (ANGLE)"
        exec build/bin/macos/game
    case opengl
        cmake -B build/macos -DGRAPHICS=opengl $debug_flag
        cmake --build build/macos --target game; or exit 1
        echo "Running: build/bin/macos/game (OpenGL)"
        exec build/bin/macos/game
    case web
        emcmake cmake -B build/web $debug_flag
        cmake --build build/web --target game
        echo "Built: build/bin/web/game.html (Web)"
    case '*'
        echo "unknown target: $target"
        echo "usage: fish build.fish [-d|--debug] [angle|opengl|web]"
        exit 1
end
