#pragma once

#include "raylib.h"

#include <vector>

namespace engine::d3 {

class ModelShaderScope {
  public:
    explicit ModelShaderScope(Model& model);
    ~ModelShaderScope();

    ModelShaderScope(const ModelShaderScope&) = delete;
    ModelShaderScope& operator=(const ModelShaderScope&) = delete;

    ModelShaderScope(ModelShaderScope&& other) noexcept;
    ModelShaderScope& operator=(ModelShaderScope&& other) noexcept;

    void enable(const ::Shader& shader);
    void disable();

  private:
    Model* m_model = nullptr;
    std::vector<::Shader> m_oldShaders;
    bool m_enabled = false;
};

} // namespace engine::d3
