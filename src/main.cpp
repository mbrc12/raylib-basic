#include "raylib.h"
#include "colors.hpp"

#include "engine/util.hpp"
#include "engine/assets.hpp"
#include "engine/mainloop.hpp"
#ifndef PROJECT_WINDOW_TITLE
#define PROJECT_WINDOW_TITLE "game"
#endif

namespace {
constexpr int screenWidth = 960;
constexpr int screenHeight = 540;

RenderTexture2D canvas{};
engine::Resource<engine::Shader> effect;
float total_time = 0.0f;

void InitializeScene() {
    canvas = LoadRenderTexture(screenWidth, screenHeight);
    effect = engine::assets::shader("effect");

    dbg("APP: Render texture valid: %s", IsRenderTextureValid(canvas) ? "yes" : "no");
    dbg("APP: Shader valid: %s", effect->valid() ? "yes" : "no");
}

void Update(float dt) {}

void Draw() {
    BeginTextureMode(canvas);
    ClearBackground(colors::SteamLords_MidnightBlack);
    DrawRectangle(90, 250, 780, 12, colors::SteamLords_IndigoBerry);
    EndTextureMode();

    effect->send("resolution", Vector2{static_cast<float>(screenWidth), static_cast<float>(screenHeight)});
    effect->send("time", total_time);
    effect->send("colorA", colors::SteamLords_DeepFern);
    effect->send("colorB", colors::SteamLords_EggplantPurple);

    BeginDrawing();
    ClearBackground(Color{18, 24, 31, 255});
    effect->enable();
    DrawTextureRec(
        canvas.texture,
        Rectangle{0.0f, 0.0f, static_cast<float>(canvas.texture.width), static_cast<float>(-canvas.texture.height)},
        Vector2{0.0f, 0.0f}, colors::PureWhite);
    effect->disable();
    DrawFPS(12, 12);
    EndDrawing();
}

void Frame() {
    float dt = GetFrameTime();
    total_time += GetFrameTime();
    Update(dt);
    Draw();
}
} // namespace

int main() {
    InitWindow(screenWidth, screenHeight, PROJECT_WINDOW_TITLE);
    SetTargetFPS(60);
    InitializeScene();

    runMainLoop(Frame);

    UnloadRenderTexture(canvas);
    CloseWindow();

    return 0;
}
