#include "free_camera.hpp"

#include <algorithm>
#include <cmath>

#include "raymath.h"
#include "input.hpp"

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

void FreeCameraController::setSpeedStep(float step) { m_speedStep = std::max(0.0f, step); }

void FreeCameraController::update(Camera3D* camera, float dt) {
    if (camera == nullptr) {
        return;
    }

    Vector2 look = Vector2{input::value(input::AnalogAction::X2), input::value(input::AnalogAction::Y2)};
    m_yaw -= look.x * m_lookSensitivity;
    m_pitch += look.y * m_lookSensitivity;
    m_pitch = clampf(m_pitch, -1.54f, 1.54f);

    Vector3 forward = forwardFromAngles(m_yaw, m_pitch);
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, m_up));

    Vector3 move{};
    if (input::down(input::Action::Up)) move = Vector3Add(move, forward);
    if (input::down(input::Action::Down)) move = Vector3Subtract(move, forward);
    if (input::down(input::Action::Right)) move = Vector3Add(move, right);
    if (input::down(input::Action::Left)) move = Vector3Subtract(move, right);
    if (input::down(input::Action::Interact)) move = Vector3Add(move, m_up);
    if (input::down(input::Action::Back)) move = Vector3Subtract(move, m_up);

    if (Vector3LengthSqr(move) > 0.0f) {
        move = Vector3Normalize(move);
        camera->position = Vector3Add(camera->position, Vector3Scale(move, m_moveSpeed * dt));
    }

    camera->target = Vector3Add(camera->position, forward);
    camera->up = m_up;
}

} // namespace engine
