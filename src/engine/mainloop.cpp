#include "mainloop.hpp"

#include "raylib.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

namespace {

std::function<void()> gFrame;
bool gFullscreenRequested = false;

#if defined(__EMSCRIPTEN__)
void requestWebFullscreen() {
    if (gFullscreenRequested) {
        return;
    }

    const int requested = EM_ASM_INT({
        const canvas = Module.canvas || document.getElementById("canvas");
        if (!canvas) {
            return 0;
        }

        const fullscreenRequestFn =
            canvas.requestFullscreen ||
            canvas.webkitRequestFullscreen ||
            canvas.msRequestFullscreen;

        if (!fullscreenRequestFn) {
            return 0;
        }

        try {
            const result = fullscreenRequestFn.call(canvas);
            if (result && typeof result.catch === "function") {
                result.catch((error) => {
                    console.warn("Fullscreen request failed:", error);
                });
            }
            return 1;
        } catch (error) {
            console.warn("Fullscreen request failed:", error);
            return 0;
        }
    });

    if (requested) {
        gFullscreenRequested = true;
    }
}

EM_BOOL requestFullscreenOnMouse(int, const EmscriptenMouseEvent *, void *) {
    requestWebFullscreen();
    return EM_FALSE;
}

EM_BOOL requestFullscreenOnTouch(int, const EmscriptenTouchEvent *, void *) {
    requestWebFullscreen();
    return EM_FALSE;
}

void installWebFullscreenHandlers() {
    emscripten_set_click_callback("#canvas", nullptr, EM_FALSE, requestFullscreenOnMouse);
    emscripten_set_touchstart_callback("#canvas", nullptr, EM_FALSE, requestFullscreenOnTouch);
}
#endif

void emscriptenFrameCallback() {
    gFrame();
}

} // namespace

void runMainLoop(std::function<void()> frame) {
    gFrame = std::move(frame);

#if defined(__EMSCRIPTEN__)
    installWebFullscreenHandlers();
    emscripten_set_main_loop(emscriptenFrameCallback, 0, 1);
#else
    while (!WindowShouldClose()) {
        gFrame();
    }
#endif
}
