#include "raylib.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

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
engine::Shader* effect = nullptr;
engine::Shader* lambert = nullptr;
Model* fish = nullptr;
Camera3D camera{};
float total_time = 0.0f;

void InitializeScene() {
    canvas = LoadRenderTexture(screenWidth, screenHeight);
    effect = engine::assets::shader("effect");
    lambert = engine::assets::shader("lambert");
    fish = engine::assets::model("fish");

    for (int i = 0; i < fish->materialCount; i++) {
        fish->materials[i].shader = lambert->raw();
    }

    camera.position = Vector3{0.0f, 10.0f, 10.0f};
    camera.target = Vector3{0.0f, 0.0f, 0.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    dbg("APP: Render texture valid: %s", IsRenderTextureValid(canvas) ? "yes" : "no");
    dbg("APP: Effect shader valid: %s", effect->valid() ? "yes" : "no");
    dbg("APP: Lambert shader valid: %s", lambert->valid() ? "yes" : "no");
    dbg("APP: Model valid: %s", fish ? "yes" : "no");

    lambert->send("ambient", Color{30, 30, 40, 255});
    lambert->send("diffuse", Color{220, 200, 180, 255});
}

void Update(float dt) {}

void Draw() {
    BeginTextureMode(canvas);
    ClearBackground(colors::SteamLords_MidnightBlack);

    BeginMode3D(camera);
    lambert->send("lightDir", Vector3{-0.5f, 1.0f, 0.8f});
    DrawModelEx(*fish, ORIGIN, UP, total_time * 360, {0.5, 0.5, 0.5}, colors::PureWhite);
    DrawGrid(10, 1.0f);
    EndMode3D();

    EndTextureMode();

    BeginDrawing();
    ClearBackground(Color{18, 24, 31, 255});
    DrawTextureRec(
        canvas.texture,
        Rectangle{0.0f, 0.0f, static_cast<float>(canvas.texture.width), static_cast<float>(-canvas.texture.height)},
        Vector2{0.0f, 0.0f}, colors::PureWhite);

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
    engine::assets::manual_unload();
    CloseWindow();

    return 0;
}
