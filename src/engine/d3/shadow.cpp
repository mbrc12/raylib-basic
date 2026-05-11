#include "shadow.hpp"

#include "raymath.h"
#include "rlgl.h"

namespace engine::d3 {

void Shadow::init(int size, const Vec3& sunDir, const Vec3& target, float distance, float orthoSize) {
    map = LoadRenderTexture(size, size);
    SetTextureFilter(map.texture, TEXTURE_FILTER_POINT);
    SetTextureWrap(map.texture, TEXTURE_WRAP_CLAMP);

    Vec3 lightDir = sunDir.norm();
    Vec3 lightPos = target + lightDir * distance;

    Matrix lightProj = MatrixOrtho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 200.0f);
    Matrix lightView = MatrixLookAt(lightPos, target, Vec3::Up);
    lightVP = MatrixMultiply(lightView, lightProj);

    camera.position = lightPos;
    camera.target = target;
    camera.up = Vec3::Up;
    camera.fovy = orthoSize * 2.0f;
    camera.projection = CAMERA_ORTHOGRAPHIC;
}

void Shadow::beginDepthPass() {
    BeginTextureMode(map);
    ClearBackground(WHITE);
    rlDisableColorBlend();
    rlEnableBackfaceCulling();
    rlSetCullFace(RL_CULL_FACE_FRONT);
    BeginMode3D(camera);
}

void Shadow::endDepthPass() {
    EndMode3D();
    rlSetCullFace(RL_CULL_FACE_BACK);
    rlDisableBackfaceCulling();
    rlEnableColorBlend();
    EndTextureMode();
}

void Shadow::unload() {
    if (IsRenderTextureValid(map)) {
        UnloadRenderTexture(map);
        map = {};
    }
}

} // namespace engine::d3
