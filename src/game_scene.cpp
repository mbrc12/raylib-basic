#include "game_scene.hpp"

#include <string>

#include "colors.hpp"
#include "engine/assets.hpp"
#include "engine/free_camera.hpp"
#include "engine/input.hpp"
#include "engine/material_slots.hpp"
#include "engine/model_shader_scope.hpp"
#include "engine/shadow.hpp"
#include "engine/util.hpp"

namespace {

constexpr int terrainTexSize = 256;
constexpr float terrainWorldSize = 200.0f;
constexpr float terrainMaxHeight = 12.0f;
constexpr int shadowMapSize = 512;
constexpr Vector3 fishScale{5.0f, 5.0f, 5.0f};
constexpr Vector3 sunDirection{-0.4f, 1.0f, 0.6f};

Model generateTerrain(Texture2D* terrainTex, engine::Shader* lambert, const engine::DirectionalShadow& shadow) {
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
    *terrainTex = LoadTextureFromImage(img);
    UnloadImage(img);

    Model model = LoadModelFromMesh(mesh);
    model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = *terrainTex;
    engine::setTextureSlot(model, engine::MaterialTextureSlot::ShadowMap, shadow.map.texture);
    model.materials[0].shader = lambert->raw();

    return model;
}

Vector3 fishPosition() {
    return Vector3{terrainWorldSize / 2.0f, terrainMaxHeight + 4.0f, terrainWorldSize / 2.0f + 3.0f};
}

} // namespace

struct GameScene::Impl {
    engine::Shader* lambert = nullptr;
    engine::Shader* depth = nullptr;
    Font* uiFont = nullptr;
    Model* fish = nullptr;
    RenderTexture2D canvas{};
    Model terrainModel{};
    Texture2D terrainTex{};
    Camera3D camera{};
    engine::DirectionalShadow shadow{};
    engine::FreeCameraController cameraController{};
    float totalTime = 0.0f;

    void load() {
        dbg("Loading scene assets");
        lambert = engine::assets::shader("lambert");
        engine::bindTextureSlot(*lambert, engine::MaterialTextureSlot::ShadowMap, "shadowMap");

        depth = engine::assets::shader("depth");
        uiFont = engine::assets::font("main");
        if (uiFont != nullptr) {
            SetTextureFilter(uiFont->texture, TEXTURE_FILTER_POINT);
        }
        fish = engine::assets::model("fish");

        float halfWorld = terrainWorldSize / 2.0f;
        camera.position = Vector3{halfWorld, terrainMaxHeight * 7.0f, halfWorld + terrainWorldSize * 0.75f};
        camera.target = Vector3{halfWorld, terrainMaxHeight * 0.35f, halfWorld};
        camera.up = Vector3{0.0f, 1.0f, 0.0f};
        camera.fovy = 45.0f;
        camera.projection = CAMERA_PERSPECTIVE;

        Vector3 lightTarget{halfWorld, terrainMaxHeight * 0.3f, halfWorld};
        shadow.init(shadowMapSize, sunDirection, lightTarget, 70.0f, terrainWorldSize * 0.7f);

        terrainModel = generateTerrain(&terrainTex, lambert, shadow);
        for (int i = 0; i < fish->materialCount; i++) {
            fish->materials[i].shader = lambert->raw();
        }
        engine::setTextureSlot(*fish, engine::MaterialTextureSlot::ShadowMap, shadow.map.texture);

        lambert->send("ambient", Color{60, 70, 90, 255});
        lambert->send("diffuse", Color{255, 245, 230, 255});
    }

    void detachSceneTextures() {
        dbg("Detaching scene textures");
        terrainModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = Texture2D{};
        engine::clearTextureSlot(terrainModel, engine::MaterialTextureSlot::ShadowMap);
        engine::clearTextureSlot(*fish, engine::MaterialTextureSlot::ShadowMap);
    }

    void drawDepthPass() {
        shadow.beginDepthPass();

        [[maybe_unused]] engine::ScopedModelShader terrainDepth(terrainModel, depth->raw());
        [[maybe_unused]] engine::ScopedModelShader fishDepth(*fish, depth->raw());
        depth->send("lightVP", shadow.lightVP);

        DrawModel(terrainModel, ORIGIN.v(), 1.0f, WHITE);
        DrawModelEx(*fish, fishPosition(), UP.v(), totalTime * 80, fishScale, WHITE);

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
        DrawModelEx(*fish, fishPosition(), UP.v(), totalTime * 80, fishScale, colors::PureWhite);

        EndMode3D();

        std::string overlay = "FPS: " + std::to_string(GetFPS()) + "  " + engine::input::compressed_state();
        if (uiFont != nullptr) {
            DrawRectangleLines(0, 0, MeasureTextEx(*uiFont, overlay.c_str(), 12.0f, 0.0f).x, 12, Color{0, 0, 0, 128});
            DrawTextEx(*uiFont, overlay.c_str(), Vector2{0, 0}, 12.0, 0.0f, colors::PureWhite);
        }

        EndTextureMode();
    }

    void enter() {
        cameraController.reset(camera);
        cameraController.setMoveSpeed(70.0f);
        cameraController.setSpeedStep(10.0f);
        DisableCursor();
    }

    void update(float dt) {
        totalTime += dt;
        cameraController.update(&camera, dt);
    }

    void draw() {
        drawDepthPass();
        drawMainPass();
    }

    void exit() {
        detachSceneTextures();
        shadow.unload();
        UnloadTexture(terrainTex);
        UnloadModel(terrainModel);
        EnableCursor();
    }
};

GameScene::GameScene() : m_impl(std::make_unique<Impl>()) {}

GameScene::~GameScene() = default;

void GameScene::setCanvas(RenderTexture2D canvas) { m_impl->canvas = canvas; }

void GameScene::load() { m_impl->load(); }

void GameScene::enter() { m_impl->enter(); }

void GameScene::update(float dt) { m_impl->update(dt); }

void GameScene::draw() { m_impl->draw(); }

void GameScene::exit() { m_impl->exit(); }
