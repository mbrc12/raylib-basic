#pragma once

#include <string>
#include "colors.hpp"
#include "engine/vec2.hpp"
#include <raylib.h>
namespace engine::d2 {

struct SpriteDrawConfig {
    Vec2f pos;
    float scale = 1.0f;
    float rot = 0.0f;
    bool flipX = false;
    bool flipY = false;
    Color tint = colors::PureWhite;
};

struct NinepatchDrawConfig {
    Vec2f pos;
    Vec2f size;
    float rot = 0.0f;
    bool flipX = false;
    bool flipY = false;
    Color tint = colors::PureWhite;
};

class Sprite {
  public:
    Sprite(std::string texture, int x, int y, int width, int height, bool center);
    ~Sprite() = default;

    void draw(SpriteDrawConfig cfg) const;
    void drawNinepatch(NinepatchDrawConfig cfg) const;

  private:
    std::string m_textureName;
    int m_x;
    int m_y;
    int m_width;
    int m_height;
    bool m_center;
};
}
