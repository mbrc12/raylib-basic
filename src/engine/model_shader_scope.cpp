#include "model_shader_scope.hpp"

#include <utility>

namespace engine {

ModelShaderScope::ModelShaderScope(Model& model) : m_model(&model), m_enabled(false) {}

ModelShaderScope::~ModelShaderScope() {
    disable();
}

void ModelShaderScope::enable(const ::Shader& shader) {
    if (m_model == nullptr || m_enabled) {
        return;
    }

    m_oldShaders.resize(m_model->materialCount);
    for (int i = 0; i < m_model->materialCount; i++) {
        m_oldShaders[i] = m_model->materials[i].shader;
        m_model->materials[i].shader = shader;
    }
    m_enabled = true;
}

void ModelShaderScope::disable() {
    if (m_model == nullptr || !m_enabled) {
        return;
    }

    for (int i = 0; i < m_model->materialCount; i++) {
        m_model->materials[i].shader = m_oldShaders[i];
    }
    m_enabled = false;
}

ModelShaderScope::ModelShaderScope(ModelShaderScope&& other) noexcept
    : m_model(other.m_model), m_oldShaders(std::move(other.m_oldShaders)), m_enabled(other.m_enabled) {
    other.m_model = nullptr;
    other.m_enabled = false;
}

ModelShaderScope& ModelShaderScope::operator=(ModelShaderScope&& other) noexcept {
    if (this != &other) {
        disable();
        m_model = other.m_model;
        m_oldShaders = std::move(other.m_oldShaders);
        m_enabled = other.m_enabled;
        other.m_model = nullptr;
        other.m_enabled = false;
    }
    return *this;
}

} // namespace engine
