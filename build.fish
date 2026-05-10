#!/usr/bin/env fish

set -l target $argv[1]

if test -z "$target"
    echo "usage: fish build.fish [angle|opengl|web]"
    exit 1
end

set -l project_root (dirname (status --current-filename))
cd "$project_root"; or exit 1

switch "$target"
    case angle
        cmake -B build/macos -DGRAPHICS=angle
        cmake --build build/macos --target game; or exit 1
        echo "Running: build/bin/macos/game (ANGLE)"
        exec build/bin/macos/game
    case opengl
        cmake -B build/macos -DGRAPHICS=opengl
        cmake --build build/macos --target game; or exit 1
        echo "Running: build/bin/macos/game (OpenGL)"
        exec build/bin/macos/game
    case web
        emcmake cmake -B build/web
        cmake --build build/web --target game
        echo "Built: build/bin/web/game.html (Web)"
    case '*'
        echo "unknown target: $target"
        echo "usage: fish build.fish [angle|opengl|web]"
        exit 1
end
