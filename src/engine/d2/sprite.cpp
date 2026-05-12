#include "engine/d2/sprite.hpp"
#include "engine/assets.hpp"

namespace engine::d2 {

Sprite::Sprite(std::string texture, int x, int y, int width, int height, bool center)
    : m_textureName(texture), m_x(x), m_y(y), m_width(width), m_height(height), m_center(center) {}

void Sprite::draw(SpriteDrawConfig cfg) const {
    auto texture = engine::assets::texture(m_textureName.c_str());

    Rectangle source{static_cast<float>(m_x), static_cast<float>(m_y), static_cast<float>(m_width),
                     static_cast<float>(m_height)};
    Rectangle dest{cfg.pos.x, cfg.pos.y, cfg.scale * m_width, cfg.scale * m_height};
     
    Vector2 origin = m_center ? Vector2{dest.width / 2.0f, dest.height / 2.0f} : Vector2{0.0f, 0.0f};

    if (cfg.flipX) {
        source.width *= -1.0f;
    }
    if (cfg.flipY) {
        source.height *= -1.0f;
    }

    DrawTexturePro(*texture, source, dest, origin, cfg.rot, cfg.tint);
}

void Sprite::configureNinepatch(int l, int r, int t, int b) {
    m_ninepatch = NinepatchData{l, r, t, b};
}

void Sprite::drawNinepatch(NinepatchDrawConfig cfg) const {
    if (!m_ninepatch) {
        throw std::runtime_error("Ninepatch data not configured for this sprite");
    }

    auto texture = engine::assets::texture(m_textureName.c_str());
    NPatchInfo info = { 
        .source = Rectangle{static_cast<float>(m_x), static_cast<float>(m_y), static_cast<float>(m_width), static_cast<float>(m_height)},
        .left = m_ninepatch->l,
        .top = m_ninepatch->t,
        .right = m_ninepatch->r,
        .bottom = m_ninepatch->b,
        .layout = NPATCH_NINE_PATCH
    };

    Rectangle dest{cfg.pos.x, cfg.pos.y, cfg.size.x, cfg.size.y};
    Vector2 origin = m_center ? Vector2{dest.width / 2.0f, dest.height / 2.0f} : Vector2{0.0f, 0.0f};

    if (cfg.flipX) {
        info.source.width *= -1.0f;
    }
    if (cfg.flipY) {
        info.source.height *= -1.0f;
    }

    DrawTextureNPatch(*texture, info, dest, origin, cfg.rot, cfg.tint);
}

}; // namespace engine::d2
