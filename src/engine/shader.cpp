#include "shader.hpp"

#include "assets.hpp"

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

Shader::Shader(const char *name) {
    auto sources = loadShader(name);
    std::string vertexSource = shaderHeader() + sources.vertex;
    std::string fragmentSource = shaderHeader() + sources.fragment;
    m_shader = LoadShaderFromMemory(vertexSource.c_str(), fragmentSource.c_str());
}

Shader::~Shader() {
    if (IsShaderValid(m_shader)) {
        UnloadShader(m_shader);
    }
}

Shader::Shader(Shader &&other) noexcept : m_shader(other.m_shader) {
    other.m_shader = {};
}

Shader &Shader::operator=(Shader &&other) noexcept {
    if (this != &other) {
        if (IsShaderValid(m_shader)) {
            UnloadShader(m_shader);
        }
        m_shader = other.m_shader;
        other.m_shader = {};
    }
    return *this;
}

void Shader::enable() {
    BeginShaderMode(m_shader);
}

void Shader::disable() {
    EndShaderMode();
}

void Shader::send(const char *name, const float *values, int uniformType) {
    SetShaderValue(m_shader, getLocation(name), values, uniformType);
}

void Shader::send(const char *name, const Vector2 &value) {
    SetShaderValue(m_shader, getLocation(name), &value.x, SHADER_UNIFORM_VEC2);
}

void Shader::send(const char *name, float value) {
    SetShaderValue(m_shader, getLocation(name), &value, SHADER_UNIFORM_FLOAT);
}

bool Shader::valid() const {
    return IsShaderValid(m_shader);
}

int Shader::getLocation(const char *name) {
    return GetShaderLocation(m_shader, name);
}

} // namespace engine
