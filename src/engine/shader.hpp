#pragma once

#include "raylib.h"

#include <string>

namespace engine {

class Shader {
public:
    Shader();

    Shader(const char *name);

    ~Shader();

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void enable();
    void disable();

    void send(const char* name, const float* values, int uniformType);
    void send(const char* name, const Vector2& value);
    void send(const char* name, float value);

    bool valid() const;

  private:
    ::Shader m_shader{};
    int getLocation(const char* name);
};

} // namespace engine
