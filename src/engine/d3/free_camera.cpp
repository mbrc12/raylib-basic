#include "free_camera.hpp"

#include <algorithm>
#include <cmath>

#include "engine/input.hpp"

namespace engine::d3 {
namespace {

float clampf(float v, float lo, float hi) {
    return std::max(lo, std::min(v, hi));
}

Vec3f forwardFromAngles(float yaw, float pitch) {
    return Vec3f{
        std::sinf(yaw) * std::cosf(pitch),
        std::sinf(pitch),
        std::cosf(yaw) * std::cosf(pitch),
    }.norm();
}

} // namespace

void FreeCamera::reset(const Camera3D& camera) {
    m_up = camera.up;

    Vec3f forward = Vec3f(camera.target) - Vec3f(camera.position);
    forward = forward.norm();

    if (forward.lenSqr() <= 0.0f) {
        forward = Vec3f{0.0f, 0.0f, -1.0f};
    }

    m_pitch = std::asinf(clampf(forward.y, -1.0f, 1.0f));
    m_yaw = std::atan2f(forward.x, forward.z);
}

void FreeCamera::setMoveSpeed(float speed) { m_moveSpeed = std::max(0.0f, speed); }

void FreeCamera::setSpeedStep(float step) { m_speedStep = std::max(0.0f, step); }

void FreeCamera::update(Camera3D* camera, float dt) {
    if (camera == nullptr) {
        return;
    }

    Vector2 look = Vector2{input::value(input::AnalogAction::X2), input::value(input::AnalogAction::Y2)};
    m_yaw -= look.x * m_lookSensitivity;
    m_pitch += look.y * m_lookSensitivity;
    m_pitch = clampf(m_pitch, -1.54f, 1.54f);

    Vec3f forward = forwardFromAngles(m_yaw, m_pitch);
    Vec3f right = forward.cross(m_up).norm();

    Vec3f move{};
    if (input::down(input::Action::Up)) move = move + forward;
    if (input::down(input::Action::Down)) move = move - forward;
    if (input::down(input::Action::Right)) move = move + right;
    if (input::down(input::Action::Left)) move = move - right;
    if (input::down(input::Action::Interact)) move = move + m_up;
    if (input::down(input::Action::Back)) move = move - m_up;

    if (move.lenSqr() > 0.0f) {
        move = move.norm();
        camera->position = Vec3f(camera->position) + move * (m_moveSpeed * dt);
    }

    camera->target = Vec3f(camera->position) + forward;
    camera->up = m_up;
}

} // namespace engine::d3
