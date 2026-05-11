#pragma once
#include "engine/vec3.hpp"

namespace engine::d3 {

struct Box {
    Vec3f center;
    float halfX, halfY, halfZ;
};

struct SweepResult {
    float t;
    Vec3f pos;
    Vec3f normal;
    bool intersect;
};

SweepResult sweep(const Box& body, Vec3f target, const Box& other);

} // namespace engine::d3
