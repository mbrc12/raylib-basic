#pragma once

#include "raylib.h"
#include "vec3.hpp"

namespace engine {

class FreeCameraController {
  public:
    void reset(const Camera3D& camera);

    void setMoveSpeed(float speed);
    void setSpeedStep(float step);

    void update(Camera3D* camera, float dt);

  private:
    Vec3 m_up{Vec3::Up};
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;
    float m_moveSpeed = 45.0f;
    float m_lookSensitivity = 0.0015f;
    float m_speedStep = 8.0f;
};

} // namespace engine
