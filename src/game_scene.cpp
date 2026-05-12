#include "game_scene.hpp"

#include "colors.hpp"
#include "engine/assets.hpp"
#include "engine/registry.hpp"

struct GameScene::Impl {
    float totalTime = 0.0f;
    RenderTexture2D canvas{};

    void load() {}

    void enter() {}

    void update(float dt) { totalTime += dt; }

    void draw() {
        auto canvas = engine::registry::get<RenderTexture2D>();
        BeginTextureMode(*canvas);

        ClearBackground(colors::SteamLords_MidnightBlack);

        auto shader = engine::assets::shader("effect");
        shader->send("colorA", colors::SteamLords_IndigoBerry);
        shader->send("colorB", colors::SteamLords_MidnightBlack);
        shader->send("time", totalTime);
        shader->send("size", Vec2f{100.0f, 100.0f});
        shader->enable();

        DrawRectangle(20, 20, 100, 100, colors::PureWhite);

        shader->disable();

        auto sprite = engine::assets::sprite("player");
        sprite->draw({.pos = Vec2f{200.0f, 100.0f}});

        DrawRectangle(199, 99, 2, 2, colors::SteamLords_Mahogany);

        EndTextureMode();
    }

    void exit() {}
};

GameScene::GameScene() : m_impl(std::make_unique<Impl>()) {}

GameScene::~GameScene() = default;

void GameScene::load() { m_impl->load(); }

void GameScene::enter() { m_impl->enter(); }

void GameScene::update(float dt) { m_impl->update(dt); }

void GameScene::draw() { m_impl->draw(); }

void GameScene::exit() { m_impl->exit(); }
