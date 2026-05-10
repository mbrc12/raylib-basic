#include "mainloop.hpp"

#include "raylib.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif

namespace {

std::function<void()> gFrame;

void emscriptenFrameWrapper(void* userData) {
    (void)userData;
    gFrame();
}

} // namespace

void runMainLoop(std::function<void()> frame) {
    gFrame = std::move(frame);

#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop_arg(emscriptenFrameWrapper, nullptr, 0, 1);
#else
    while (!WindowShouldClose()) {
        gFrame();
    }
#endif
}
