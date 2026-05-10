#pragma once

#include "raylib.h"

#include <vector>

namespace engine {

class ScopedModelShader {
  public:
    ScopedModelShader(Model& model, const ::Shader& shader);
    ~ScopedModelShader();

    ScopedModelShader(const ScopedModelShader&) = delete;
    ScopedModelShader& operator=(const ScopedModelShader&) = delete;

    ScopedModelShader(ScopedModelShader&& other) noexcept;
    ScopedModelShader& operator=(ScopedModelShader&& other) noexcept;

  private:
    Model* m_model = nullptr;
    std::vector<::Shader> m_oldShaders;
};

} // namespace engine
