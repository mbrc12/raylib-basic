#pragma once
#include "engine/vec3.hpp"

namespace engine::d3 {

struct Box3 {
    Vec3f center;
    float halfX, halfY, halfZ;
};

struct SweepResult3 {
    float t;
    Vec3f pos;
    Vec3f normal;
    bool intersect;
};

SweepResult3 sweep(const Box3& body, Vec3f target, const Box3& other);

} // namespace engine::d3
