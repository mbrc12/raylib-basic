#include "shader.hpp"

#include <string>

namespace engine {
namespace {

std::string shaderHeader() {
#if defined(__EMSCRIPTEN__) || defined(PROJECT_USE_GLSL_ES)
    return R"(#version 300 es
precision highp float;
)";
#else
    return R"(#version 330
)";
#endif
}

} // namespace

Shader::Shader() {}

Shader::Shader(::Shader raw) : m_shader(raw) {}

Shader::Shader(const char* vertSource, const char* fragSource) {
    std::string vert = shaderHeader() + vertSource;
    std::string frag = shaderHeader() + fragSource;
    m_shader = LoadShaderFromMemory(vert.c_str(), frag.c_str());
}

Shader::~Shader() {
    if (IsShaderValid(m_shader)) {
        UnloadShader(m_shader);
    }
}

Shader::Shader(Shader&& other) noexcept : m_shader(other.m_shader) { other.m_shader = {}; }

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (IsShaderValid(m_shader)) {
            UnloadShader(m_shader);
        }
        m_shader = other.m_shader;
        other.m_shader = {};
    }
    return *this;
}

void Shader::enable() { BeginShaderMode(m_shader); }

void Shader::disable() { EndShaderMode(); }

void Shader::send(const char* name, const float* values, int uniformType) {
    SetShaderValue(m_shader, getLocation(name), values, uniformType);
}

void Shader::send(const char* name, float value) {
    SetShaderValue(m_shader, getLocation(name), &value, SHADER_UNIFORM_FLOAT);
}

void Shader::send(const char* name, const Vector2& value) {
    SetShaderValue(m_shader, getLocation(name), &value.x, SHADER_UNIFORM_VEC2);
}

void Shader::send(const char* name, const Vector3& value)  {
    SetShaderValue(m_shader, getLocation(name), &value.x, SHADER_UNIFORM_VEC3);
}

void Shader::send(const char* name, const float* values, int uniformType, int count)  {
    SetShaderValueV(m_shader, getLocation(name), values, uniformType, count);
}

void Shader::send(const char* name, const Vector2* values, int count)  {
    SetShaderValueV(m_shader, getLocation(name), values, SHADER_UNIFORM_VEC2, count);
}

void Shader::send(const char* name, const Vector3* values, int count)  {
    SetShaderValueV(m_shader, getLocation(name), values, SHADER_UNIFORM_VEC3, count);
}

void Shader::send(const char* name, const Texture2D& texture)  {
    SetShaderValueTexture(m_shader, getLocation(name), texture);
}

void Shader::send(const char* name, const Matrix& mat)  { SetShaderValueMatrix(m_shader, getLocation(name), mat); }

void Shader::send(const char* name, const Color& color) {
    float v[4] = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f};
    SetShaderValue(m_shader, getLocation(name), v, SHADER_UNIFORM_VEC4);
}

bool Shader::valid() { return IsShaderValid(m_shader); }

int Shader::getLocation(const char* name)  { return GetShaderLocation(m_shader, name); }

} // namespace engine
