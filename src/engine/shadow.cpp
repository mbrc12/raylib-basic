#include "shadow.hpp"

#include "raymath.h"
#include "rlgl.h"

namespace engine {

void DirectionalShadow::init(int size, const Vector3& sunDir, const Vector3& target, float distance, float orthoSize) {
    map = LoadRenderTexture(size, size);
    SetTextureFilter(map.texture, TEXTURE_FILTER_POINT);
    SetTextureWrap(map.texture, TEXTURE_WRAP_CLAMP);

    Vector3 lightDir = Vector3Normalize(sunDir);
    Vector3 lightPos = Vector3Add(target, Vector3Scale(lightDir, distance));

    Matrix lightProj = MatrixOrtho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 200.0f);
    Matrix lightView = MatrixLookAt(lightPos, target, Vector3{0.0f, 1.0f, 0.0f});
    lightVP = MatrixMultiply(lightView, lightProj);

    camera.position = lightPos;
    camera.target = target;
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = orthoSize * 2.0f;
    camera.projection = CAMERA_ORTHOGRAPHIC;
}

void DirectionalShadow::beginDepthPass() {
    BeginTextureMode(map);
    ClearBackground(WHITE);
    rlDisableColorBlend();
    rlEnableBackfaceCulling();
    rlSetCullFace(RL_CULL_FACE_FRONT);
    BeginMode3D(camera);
}

void DirectionalShadow::endDepthPass() {
    EndMode3D();
    rlSetCullFace(RL_CULL_FACE_BACK);
    rlDisableBackfaceCulling();
    rlEnableColorBlend();
    EndTextureMode();
}

void DirectionalShadow::unload() {
    if (IsRenderTextureValid(map)) {
        UnloadRenderTexture(map);
        map = {};
    }
}

} // namespace engine
