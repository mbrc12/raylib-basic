#pragma once

#include "raylib.h"

namespace engine {

class Shader {
  public:
    Shader();
    Shader(const char* vertSource, const char* fragSource);
    explicit Shader(::Shader raw);

    ~Shader();

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void enable();
    void disable();

    void send(const char* name, const float* values, int uniformType);
    void send(const char* name, float value);
    void send(const char* name, const Vector2& value);
    void send(const char* name, const Vector3& value);
    void send(const char* name, const float* values, int uniformType, int count);
    void send(const char* name, const Vector2* values, int count);
    void send(const char* name, const Vector3* values, int count);
    void send(const char* name, const Texture2D& texture);
    void send(const char* name, const Matrix& mat);
    void send(const char* name, const Color& color);

    bool valid();
    void unload();
    void bindLocation(int locIndex, const char* name);
    const ::Shader& raw() const { return m_shader; }

  private:
    ::Shader m_shader{};
    int getLocation(const char* name);
};

} // namespace engine
