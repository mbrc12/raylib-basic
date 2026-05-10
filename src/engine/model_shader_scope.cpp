#include "model_shader_scope.hpp"

#include <utility>

namespace engine {

ScopedModelShader::ScopedModelShader(Model& model, const ::Shader& shader) : m_model(&model) {
    m_oldShaders.resize(model.materialCount);
    for (int i = 0; i < model.materialCount; i++) {
        m_oldShaders[i] = model.materials[i].shader;
        model.materials[i].shader = shader;
    }
}

ScopedModelShader::~ScopedModelShader() {
    if (m_model == nullptr) {
        return;
    }

    for (int i = 0; i < m_model->materialCount; i++) {
        m_model->materials[i].shader = m_oldShaders[i];
    }
}

ScopedModelShader::ScopedModelShader(ScopedModelShader&& other) noexcept
    : m_model(other.m_model), m_oldShaders(std::move(other.m_oldShaders)) {
    other.m_model = nullptr;
}

ScopedModelShader& ScopedModelShader::operator=(ScopedModelShader&& other) noexcept {
    if (this != &other) {
        if (m_model != nullptr) {
            for (int i = 0; i < m_model->materialCount; i++) {
                m_model->materials[i].shader = m_oldShaders[i];
            }
        }
        m_model = other.m_model;
        m_oldShaders = std::move(other.m_oldShaders);
        other.m_model = nullptr;
    }
    return *this;
}

} // namespace engine
