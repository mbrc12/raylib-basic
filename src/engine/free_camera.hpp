#pragma once

#include "raylib.h"

namespace engine {

class FreeCameraController {
  public:
    void reset(const Camera3D& camera);

    void setMoveSpeed(float speed);
    void setMouseSensitivity(float sensitivity);
    void setSpeedStep(float step);
    void setSprintMultiplier(float multiplier);

    void update(Camera3D* camera, float dt);

  private:
    Vector3 m_up{0.0f, 1.0f, 0.0f};
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;
    float m_moveSpeed = 45.0f;
    float m_mouseSensitivity = 0.0035f;
    float m_speedStep = 8.0f;
    float m_sprintMultiplier = 3.0f;
};

} // namespace engine
