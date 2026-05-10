#include "raylib.h"

#include "engine/assets.hpp"
#include "engine/mainloop.hpp"
#ifndef PROJECT_WINDOW_TITLE
#define PROJECT_WINDOW_TITLE "game"
#endif

namespace {
constexpr int kScreenWidth = 960;
constexpr int kScreenHeight = 540;

RenderTexture2D gTarget{};
engine::Resource<engine::Shader> gShader;
float gTime = 0.0f;

void InitializeScene() {
    gTarget = LoadRenderTexture(kScreenWidth, kScreenHeight);
    gShader = engine::assets::shader("effect");

    TraceLog(LOG_INFO, "APP: Render texture valid: %s", IsRenderTextureValid(gTarget) ? "yes" : "no");
    TraceLog(LOG_INFO, "APP: Shader valid: %s", gShader->valid() ? "yes" : "no");
}

void DrawFrame() {

    gTime += GetFrameTime();

    BeginTextureMode(gTarget);
    ClearBackground(Color{16, 22, 30, 255});
    DrawCircleGradient(Vector2{kScreenWidth / 2.0f, kScreenHeight / 2.0f}, 220, Color{255, 116, 82, 255},
                       Color{25, 78, 132, 255});
    DrawRectangle(90, 250, 780, 12, Color{240, 238, 220, 255});
    DrawText("ES 3.0 shader path", 96, 180, 42, RAYWHITE);
    DrawText("uses #version 300 es + texelFetch()", 100, 235, 24, Color{180, 232, 255, 255});
    EndTextureMode();

    gShader->send("resolution", Vector2{static_cast<float>(kScreenWidth), static_cast<float>(kScreenHeight)});
    gShader->send("time", gTime);

    BeginDrawing();
    ClearBackground(Color{18, 24, 31, 255});
    gShader->enable();
    DrawTextureRec(
        gTarget.texture,
        Rectangle{0.0f, 0.0f, static_cast<float>(gTarget.texture.width), static_cast<float>(-gTarget.texture.height)},
        Vector2{0.0f, 0.0f}, WHITE);
    gShader->disable();
    DrawFPS(12, 12);
    EndDrawing();
}
} // namespace

int main() {
    InitWindow(kScreenWidth, kScreenHeight, PROJECT_WINDOW_TITLE);
    SetTargetFPS(60);
    InitializeScene();

    runMainLoop(DrawFrame);

    gTarget = {};
    CloseWindow();

    return 0;
}
