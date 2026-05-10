#pragma once

#include "raylib.h"

namespace engine {

struct DirectionalShadow {
    RenderTexture2D map{};
    Camera3D camera{};
    Matrix lightVP{};

    void init(int size, const Vector3& sunDir, const Vector3& target, float distance, float orthoSize);
    void beginDepthPass();
    void endDepthPass();
    void unload();
};

} // namespace engine
