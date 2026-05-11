#pragma once

namespace engine {

class Scene {
  public:
    virtual ~Scene() = default;

    virtual void load() = 0;
    virtual void enter() = 0;
    virtual void update(float dt) = 0;
    virtual void draw() = 0;
    virtual void exit() = 0;
};

} // namespace engine
