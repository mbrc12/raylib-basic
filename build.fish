#!/usr/bin/env fish

argparse 'd/debug' 'n/no-run' -- $argv; or exit 1
set -l target $argv[1]

if test -z "$target"
    echo "usage: fish build.fish [-d|--debug] [angle|sdl3-angle|opengl|web]"
    exit 1
end

if set -q _flag_debug
    set -l debug_flag -DCMAKE_BUILD_TYPE=Debug
end

set -l project_root (dirname (status --current-filename))
cd "$project_root"; or exit 1

function finish_line
    set -l mode $argv[1]
    set -l stamp (date '+%H:%M:%S')
    set_color green
    echo "Finished at $stamp [$mode]"
    set_color normal
end

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
        finish_line "ANGLE"
    case sdl3-angle
        if set -q debug_flag
            cmake -B build/macos -DGRAPHICS=sdl3-angle $debug_flag
        else
            cmake -B build/macos -DGRAPHICS=sdl3-angle
        end
        cmake --build build/macos --target game; or exit 1
        if not set -q _flag_no_run
            echo "Running: build/bin/macos/game (SDL3 + ANGLE)"
            exec build/bin/macos/game
        end
        finish_line "SDL3 + ANGLE"
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
        finish_line "OpenGL"
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
        finish_line "Web"
    case '*'
        echo "unknown target: $target"
        echo "usage: fish build.fish [-d|--debug] [-n|--no-run] [angle|sdl3-angle|opengl|web]"
        exit 1
end
