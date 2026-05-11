#pragma once

#include "engine/vec2.hpp"

namespace engine::d2 {

struct Box2 {
    Vec2f center;
    float halfX, halfY;
};

struct SweepResult2 {
    float t;
    Vec2f pos;
    Vec2f normal;
    bool intersect;
};

SweepResult2 sweep(const Box2& body, Vec2f target, const Box2& other);

} // namespace engine::d2
