#include "game_scene.hpp"

#include <string>

#include "colors.hpp"
#include "engine/input.hpp"
#include "engine/text.hpp"
#include "engine/util.hpp"

struct GameScene::Impl {
    float totalTime = 0.0f;
    RenderTexture2D canvas{};

    void load() { dbg("Loading scene assets"); }

    void enter() {}

    void update(float dt) { totalTime += dt; }

    void draw() {
        BeginTextureMode(canvas);
        ClearBackground(colors::SteamLords_MidnightBlack);

        auto tc = colors::SteamLords_ForestGreen;

        std::string overlay = "FPS: " + std::to_string(GetFPS()) + "  " + engine::input::compressed_state();
        engine::drawText(overlay, 0, 0, tc);
        auto v = engine::measureText(overlay).as<int>();
        DrawRectangleLines(0, 0, v.x, v.y, tc);

        EndTextureMode();
    }

    void exit() {}
};

GameScene::GameScene() : m_impl(std::make_unique<Impl>()) {}

GameScene::~GameScene() = default;

void GameScene::setCanvas(RenderTexture2D canvas) { m_impl->canvas = canvas; }

void GameScene::load() { m_impl->load(); }

void GameScene::enter() { m_impl->enter(); }

void GameScene::update(float dt) { m_impl->update(dt); }

void GameScene::draw() { m_impl->draw(); }

void GameScene::exit() { m_impl->exit(); }
