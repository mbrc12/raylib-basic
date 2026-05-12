#include "engine/registry.hpp"
#include "raylib.h"

#include <format>

#include "colors.hpp"
#include "engine/input.hpp"
#include "engine/mainloop.hpp"
#include "game_scene.hpp"

#ifndef PROJECT_WINDOW_TITLE
#define PROJECT_WINDOW_TITLE "game"
#endif

constexpr int gameRenderWidth = 480;
constexpr int gameRenderHeight = 270;
constexpr int gameScale = 3;
constexpr int defaultScreenWidth = gameRenderWidth * gameScale;
constexpr int defaultScreenHeight = gameRenderHeight * gameScale;

RenderTexture2D canvas{};
GameScene scene{};
int screenshotIndex = 0;
bool captureScreenshot = false;
bool clearStartupTopmost = false;

void presentCanvas() {
    BeginDrawing();
    ClearBackground(colors::SteamLords_MidnightBlack);
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    DrawTexturePro(
        canvas.texture,
        Rectangle{0.0f, 0.0f, static_cast<float>(canvas.texture.width), static_cast<float>(-canvas.texture.height)},
        Rectangle{0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight)},
        Vector2{0.0f, 0.0f},
        0.0f,
        colors::PureWhite
    );

    if (IsKeyPressed(KEY_F6)) {
        captureScreenshot = true;
    }

    EndDrawing();

    if (captureScreenshot) {
        auto path = std::format("screenshot-{:03d}.png", screenshotIndex++);
        TakeScreenshot(path.c_str());
        captureScreenshot = false;
    }

    if (clearStartupTopmost) {
        ClearWindowState(FLAG_WINDOW_TOPMOST);
        clearStartupTopmost = false;
    }
}

void frame() {
    float dt = GetFrameTime();
    engine::input::update();

    scene.update(dt);

    scene.draw();

    presentCanvas();
}

int main() {
    SetConfigFlags(FLAG_WINDOW_TOPMOST);
    InitWindow(defaultScreenWidth, defaultScreenHeight, PROJECT_WINDOW_TITLE);
    SetTargetFPS(60);

    canvas = LoadRenderTexture(gameRenderWidth, gameRenderHeight);
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);

    engine::registry::store(&canvas);

    engine::input::init();
    scene.load();
    scene.enter();

    SetWindowFocused();
    clearStartupTopmost = true;

    runMainLoop(frame);

    scene.exit();
    UnloadRenderTexture(canvas);
    CloseWindow();

    return 0;
}
