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
constexpr int terrainTexSize = 256;
constexpr float terrainWorldSize = 200.0f;
constexpr float terrainMaxHeight = 12.0f;

RenderTexture2D canvas{};
engine::Shader* effect = nullptr;
engine::Shader* lambert = nullptr;
Model* fish = nullptr;
Model terrainModel{};
Texture2D terrainTex{};
Camera3D camera{};
float total_time = 0.0f;

Model GenerateTerrain() {
    Image img = GenImageColor(terrainTexSize, terrainTexSize, BLACK);

    SetRandomSeed(42);

    for (int y = 0; y < terrainTexSize; y++) {
        for (int x = 0; x < terrainTexSize; x++) {
            unsigned char g = (unsigned char)(GetRandomValue(80, 180));
            unsigned char r = (unsigned char)(g / 4 + GetRandomValue(0, 30));
            unsigned char b = (unsigned char)(g / 5 + GetRandomValue(0, 20));
            ImageDrawPixel(&img, x, y, Color{r, g, b, 255});
        }
    }

    Mesh mesh = GenMeshHeightmap(img, Vector3{terrainWorldSize, terrainMaxHeight, terrainWorldSize});
    terrainTex = LoadTextureFromImage(img);
    UnloadImage(img);

    Model model = LoadModelFromMesh(mesh);
    model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = terrainTex;
    model.materials[0].shader = lambert->raw();

    return model;
}

void InitializeScene() {
    canvas = LoadRenderTexture(screenWidth, screenHeight);
    effect = engine::assets::shader("effect");
    lambert = engine::assets::shader("lambert");
    fish = engine::assets::model("fish");

    terrainModel = GenerateTerrain();

    for (int i = 0; i < fish->materialCount; i++) {
        fish->materials[i].shader = lambert->raw();
    }

    float halfWorld = terrainWorldSize / 2.0f;
    camera.position = Vector3{halfWorld, terrainMaxHeight * 0.4f + 2.0f, halfWorld};
    camera.target = Vector3{halfWorld, terrainMaxHeight * 0.4f, halfWorld - 1.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 65.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    DisableCursor();

    dbg("APP: Render texture valid: %s", IsRenderTextureValid(canvas) ? "yes" : "no");
    dbg("APP: Effect shader valid: %s", effect->valid() ? "yes" : "no");
    dbg("APP: Lambert shader valid: %s", lambert->valid() ? "yes" : "no");
    dbg("APP: Model valid: %s", fish ? "yes" : "no");

    lambert->send("ambient", Color{60, 70, 90, 255});
    lambert->send("diffuse", Color{255, 245, 230, 255});
}

void Update(float dt) {
    UpdateCamera(&camera, CAMERA_FREE);
}

void Draw() {
    BeginTextureMode(canvas);
    ClearBackground(colors::SteamLords_SteelBlue);

    BeginMode3D(camera);
    lambert->send("lightDir", Vector3{-0.4f, 1.0f, 0.6f});
    lambert->send("viewPos", camera.position);
    lambert->send("fogColor", colors::SteamLords_SteelBlue);
    lambert->send("fogDensity", 0.1f);

    DrawModel(terrainModel, ORIGIN, 1.0f, WHITE);

    Vector3 fishPos{
        terrainWorldSize / 2.0f,
        terrainMaxHeight + 4.0f,
        terrainWorldSize / 2.0f + 3.0f};
    DrawModelEx(*fish, fishPos, UP, total_time * 80, {0.4, 0.4, 0.4}, colors::PureWhite);

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
    total_time += dt;
    Update(dt);
    Draw();
}
} // namespace

int main() {
    InitWindow(screenWidth, screenHeight, PROJECT_WINDOW_TITLE);
    SetTargetFPS(60);
    InitializeScene();

    runMainLoop(Frame);

    EnableCursor();
    UnloadTexture(terrainTex);
    UnloadModel(terrainModel);
    UnloadRenderTexture(canvas);
    engine::assets::manual_unload();
    CloseWindow();

    return 0;
}
