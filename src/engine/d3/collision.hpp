#pragma once
#include "engine/vec3.hpp"

namespace engine::d3 {

struct Box {
    Vec3 center;
    float halfX, halfY, halfZ;
};

struct SweepResult {
    float t;
    Vec3 pos;
    Vec3 normal;
    bool intersect;
};

SweepResult sweep(const Box& body, Vec3 target, const Box& other);

} // namespace engine::d3
