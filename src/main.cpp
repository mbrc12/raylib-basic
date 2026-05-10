#include "raylib.h"

#include "colors.hpp"

#include "engine/util.hpp"
#include "engine/assets.hpp"
#include "engine/input.hpp"
#include "engine/free_camera.hpp"
#include "engine/model_shader_scope.hpp"
#include "engine/shadow.hpp"
#include "engine/mainloop.hpp"
#ifndef PROJECT_WINDOW_TITLE
#define PROJECT_WINDOW_TITLE "game"
#endif

constexpr int gameRenderWidth = 480;
constexpr int gameRenderHeight = 270;
constexpr int gameScale = 3;
constexpr int screenWidth = gameRenderWidth * gameScale;
constexpr int screenHeight = gameRenderHeight * gameScale;
constexpr int terrainTexSize = 256;
constexpr float terrainWorldSize = 200.0f;
constexpr float terrainMaxHeight = 12.0f;
constexpr int shadowMapSize = 512;
constexpr Vector3 fishScale{5.0f, 5.0f, 5.0f};
constexpr Vector3 sunDirection{-0.4f, 1.0f, 0.6f};

RenderTexture2D canvas{};
engine::Shader* lambert = nullptr;
engine::Shader* depth = nullptr;
Font* uiFont = nullptr;
Model* fish = nullptr;
Model terrainModel{};
Texture2D terrainTex{};
Camera3D camera{};
engine::DirectionalShadow shadow{};
engine::FreeCameraController cameraController{};
float total_time = 0.0f;
int screenshotIndex = 0;
bool captureScreenshot = false;
bool clearStartupTopmost = false;

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
    model.materials[0].maps[MATERIAL_MAP_METALNESS].texture = shadow.map.texture;
    model.materials[0].shader = lambert->raw();

    return model;
}

void InitializeScene() {
    canvas = LoadRenderTexture(gameRenderWidth, gameRenderHeight);
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);
    lambert = engine::assets::shader("lambert");
    depth = engine::assets::shader("depth");
    uiFont = engine::assets::font("main");
    if (uiFont != nullptr) {
        SetTextureFilter(uiFont->texture, TEXTURE_FILTER_POINT);
    }
    fish = engine::assets::model("fish");
    lambert->bindLocation(SHADER_LOC_MAP_METALNESS, "shadowMap");

    float halfWorld = terrainWorldSize / 2.0f;
    camera.position = Vector3{halfWorld, terrainMaxHeight * 7.0f, halfWorld + terrainWorldSize * 0.75f};
    camera.target = Vector3{halfWorld, terrainMaxHeight * 0.35f, halfWorld};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    cameraController.reset(camera);
    cameraController.setMoveSpeed(70.0f);
    cameraController.setSpeedStep(10.0f);

    DisableCursor();
    engine::input::init();

    Vector3 lightTarget{halfWorld, terrainMaxHeight * 0.3f, halfWorld};
    shadow.init(shadowMapSize, sunDirection, lightTarget, 70.0f, terrainWorldSize * 0.7f);

    terrainModel = GenerateTerrain();

    for (int i = 0; i < fish->materialCount; i++) {
        fish->materials[i].shader = lambert->raw();
        fish->materials[i].maps[MATERIAL_MAP_METALNESS].texture = shadow.map.texture;
    }

    lambert->send("ambient", Color{60, 70, 90, 255});
    lambert->send("diffuse", Color{255, 245, 230, 255});
}

void Update([[maybe_unused]] float dt) {
    cameraController.update(&camera, dt);
}

void drawDepthPass() {
    shadow.beginDepthPass();

    [[maybe_unused]] engine::ScopedModelShader terrainDepth(terrainModel, depth->raw());
    [[maybe_unused]] engine::ScopedModelShader fishDepth(*fish, depth->raw());
    depth->send("lightVP", shadow.lightVP);

    DrawModel(terrainModel, ORIGIN.v(), 1.0f, WHITE);

    Vector3 fishPos{
        terrainWorldSize / 2.0f,
        terrainMaxHeight + 4.0f,
        terrainWorldSize / 2.0f + 3.0f};
    DrawModelEx(*fish, fishPos, UP.v(), total_time * 80, fishScale, WHITE);

    shadow.endDepthPass();
}

void drawMainPass() {
    BeginTextureMode(canvas);
    ClearBackground(colors::SteamLords_SteelBlue);

    BeginMode3D(camera);

    lambert->send("lightDir", sunDirection);
    lambert->send("viewPos", camera.position);
    lambert->send("fogColor", colors::SteamLords_SteelBlue);
    lambert->send("fogDensity", 0.0f);
    lambert->send("lightVP", shadow.lightVP);

    DrawModel(terrainModel, ORIGIN.v(), 1.0f, WHITE);

    Vector3 fishPos{
        terrainWorldSize / 2.0f,
        terrainMaxHeight + 4.0f,
        terrainWorldSize / 2.0f + 3.0f};
    DrawModelEx(*fish, fishPos, UP.v(), total_time * 80, fishScale, colors::PureWhite);

    EndMode3D();

    using engine::input::Action, engine::input::down;

    std::string overlay = "FPS: " + std::to_string(GetFPS()) + "  " + engine::input::compressed_state();
    if (uiFont != nullptr) {
        DrawRectangleLines(0, 0, MeasureTextEx(*uiFont, overlay.c_str(), 12.0f, 0.0f).x, 12, Color{0, 0, 0, 128});
        DrawTextEx(*uiFont, overlay.c_str(), Vector2{0,0}, 12.0, 0.0f, colors::PureWhite);
         // DrawTextEx(*uiFont, overlay.c_str(), Vector2{12.0f, 12.0f}, 12.0, 0.0f, colors::PureWhite);
    }

    EndTextureMode();
}

void Draw() {
    drawDepthPass();
    drawMainPass();

    BeginDrawing();
    ClearBackground(Color{18, 24, 31, 255});
    DrawTexturePro(
        canvas.texture,
        Rectangle{0.0f, 0.0f, static_cast<float>(canvas.texture.width), static_cast<float>(-canvas.texture.height)},
        Rectangle{0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight)},
        Vector2{0.0f, 0.0f},
        0.0f,
        colors::PureWhite);

    if (IsKeyPressed(KEY_F6)) {
        captureScreenshot = true;
    }

    EndDrawing();

    if (captureScreenshot) {
        char path[64];
        snprintf(path, sizeof(path), "screenshot-%03d.png", screenshotIndex++);
        TakeScreenshot(path);
        captureScreenshot = false;
    }

    if (clearStartupTopmost) {
        ClearWindowState(FLAG_WINDOW_TOPMOST);
        clearStartupTopmost = false;
    }
}

void DetachSceneTextures() {
    terrainModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = Texture2D{};
    terrainModel.materials[0].maps[MATERIAL_MAP_METALNESS].texture = Texture2D{};
    for (int i = 0; i < fish->materialCount; i++) {
        fish->materials[i].maps[MATERIAL_MAP_METALNESS].texture = Texture2D{};
    }
}

void Frame() {
    float dt = GetFrameTime();
    total_time += dt;
    engine::input::update();
    Update(dt);
    Draw();
}

int main() {
    SetConfigFlags(FLAG_WINDOW_TOPMOST);
    InitWindow(screenWidth, screenHeight, PROJECT_WINDOW_TITLE);
    SetTargetFPS(60);
    InitializeScene();
    SetWindowFocused();
    clearStartupTopmost = true;

    runMainLoop(Frame);

    EnableCursor();
    DetachSceneTextures();
    shadow.unload();
    UnloadTexture(terrainTex);
    UnloadModel(terrainModel);
    UnloadRenderTexture(canvas);
    engine::assets::manual_unload();
    CloseWindow();

    return 0;
}
