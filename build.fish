#!/usr/bin/env fish

argparse 'd/debug' 'n/no-run' -- $argv; or exit 1
set -l target $argv[1]

if test -z "$target"
    echo "usage: fish build.fish [-d|--debug] [angle|opengl|web]"
    exit 1
end

if set -q _flag_debug
    set -l debug_flag -DCMAKE_BUILD_TYPE=Debug
end

set -l project_root (dirname (status --current-filename))
cd "$project_root"; or exit 1

switch "$target"
    case angle
        if set -q debug_flag
            cmake -B build/macos -DGRAPHICS=angle $debug_flag
        else
            cmake -B build/macos -DGRAPHICS=angle
        end
        cmake --build build/macos --target game; or exit 1
        if not set -q _flag_no_run
            echo "Running: build/bin/macos/game (ANGLE)"
            exec build/bin/macos/game
        end
    case opengl
        if set -q debug_flag
            cmake -B build/macos -DGRAPHICS=opengl $debug_flag
        else
            cmake -B build/macos -DGRAPHICS=opengl
        end
        cmake --build build/macos --target game; or exit 1
        if not set -q _flag_no_run
            echo "Running: build/bin/macos/game (OpenGL)"
            exec build/bin/macos/game
        end
    case web
        if set -q debug_flag
            emcmake cmake -B build/web $debug_flag
        else
            emcmake cmake -B build/web
        end
        cmake --build build/web --target game
        if not set -q _flag_no_run
            echo "Built: build/bin/web/game.html (Web)"
        end
    case '*'
        echo "unknown target: $target"
        echo "usage: fish build.fish [-d|--debug] [-n|--no-run] [angle|opengl|web]"
        exit 1
end
