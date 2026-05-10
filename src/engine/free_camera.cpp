#include "free_camera.hpp"

#include <algorithm>
#include <cmath>

#include "raymath.h"

namespace engine {
namespace {

float clampf(float v, float lo, float hi) {
    return std::max(lo, std::min(v, hi));
}

Vector3 forwardFromAngles(float yaw, float pitch) {
    return Vector3Normalize(Vector3{
        std::sinf(yaw) * std::cosf(pitch),
        std::sinf(pitch),
        std::cosf(yaw) * std::cosf(pitch),
    });
}

} // namespace

void FreeCameraController::reset(const Camera3D& camera) {
    m_up = camera.up;

    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    if (Vector3LengthSqr(forward) <= 0.0f) {
        forward = Vector3{0.0f, 0.0f, -1.0f};
    }

    m_pitch = std::asinf(clampf(forward.y, -1.0f, 1.0f));
    m_yaw = std::atan2f(forward.x, forward.z);
}

void FreeCameraController::setMoveSpeed(float speed) { m_moveSpeed = std::max(0.0f, speed); }

void FreeCameraController::setMouseSensitivity(float sensitivity) {
    m_mouseSensitivity = std::max(0.0f, sensitivity);
}

void FreeCameraController::setSpeedStep(float step) { m_speedStep = std::max(0.0f, step); }

void FreeCameraController::setSprintMultiplier(float multiplier) {
    m_sprintMultiplier = std::max(1.0f, multiplier);
}

void FreeCameraController::update(Camera3D* camera, float dt) {
    if (camera == nullptr) {
        return;
    }

    Vector2 mouse = GetMouseDelta();
    m_yaw -= mouse.x * m_mouseSensitivity;
    m_pitch -= mouse.y * m_mouseSensitivity;
    m_pitch = clampf(m_pitch, -1.54f, 1.54f);

    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        m_moveSpeed = std::max(1.0f, m_moveSpeed + wheel * m_speedStep);
    }

    Vector3 forward = forwardFromAngles(m_yaw, m_pitch);
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, m_up));

    Vector3 move{};
    if (IsKeyDown(KEY_W)) move = Vector3Add(move, forward);
    if (IsKeyDown(KEY_S)) move = Vector3Subtract(move, forward);
    if (IsKeyDown(KEY_D)) move = Vector3Add(move, right);
    if (IsKeyDown(KEY_A)) move = Vector3Subtract(move, right);
    if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_E)) move = Vector3Add(move, m_up);
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_Q)) move = Vector3Subtract(move, m_up);

    if (Vector3LengthSqr(move) > 0.0f) {
        move = Vector3Normalize(move);
        float speed = m_moveSpeed;
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
            speed *= m_sprintMultiplier;
        }
        camera->position = Vector3Add(camera->position, Vector3Scale(move, speed * dt));
    }

    camera->target = Vector3Add(camera->position, forward);
    camera->up = m_up;
}

} // namespace engine
