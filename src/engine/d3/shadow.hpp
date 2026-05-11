#pragma once

#include "raylib.h"
#include "engine/vec3.hpp"

namespace engine::d3 {

struct Shadow {
    RenderTexture2D map{};
    Camera3D camera{};
    Matrix lightVP{};

    void init(int size, const Vec3f& sunDir, const Vec3f& target, float distance, float orthoSize);
    void beginDepthPass();
    void endDepthPass();
    void unload();
};

} // namespace engine::d3
