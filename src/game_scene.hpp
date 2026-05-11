#pragma once

#include <memory>

#include "engine/scene.hpp"

class GameScene : public engine::Scene {
  public:
    GameScene();
    ~GameScene() override;

    void load() override;
    void enter() override;
    void update(float dt) override;
    void draw() override;
    void exit() override;

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
