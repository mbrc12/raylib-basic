#!/usr/bin/env fish

argparse 'c/compile=' 'r/run=' -- $argv; or exit 1

set -l project_root (dirname (status --current-filename))
cd "$project_root"; or exit 1

function __dev_watch_files
    find assets src -type f
    echo CMakeLists.txt
    echo build.fish
    echo dev.fish
end

if set -q _flag_compile
    set -l target $_flag_compile
    if test "$target" != "opengl" -a "$target" != "angle"
        echo "usage: fish dev.fish --compile [angle|opengl]"
        exit 1
    end

    fish build.fish --no-run "$target"; or exit 1
    __dev_watch_files | sort -u | entr -c fish build.fish --no-run "$target"
    exit 0
end

if set -q _flag_run
    set -l target $_flag_run
    switch "$target"
        case angle opengl
            exec build/bin/macos/game
        case '*'
            echo "usage: fish dev.fish --run [angle|opengl]"
            exit 1
    end
end

echo "usage: fish dev.fish --compile [angle|opengl] | --run [angle|opengl]"
exit 1
