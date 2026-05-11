#pragma once

#include "raylib.h"

namespace engine {

enum class MaterialTextureSlot {
    ShadowMap,
};

void bindTextureSlot(Shader& shader, MaterialTextureSlot slot, const char* samplerName);
void setTextureSlot(Model& model, MaterialTextureSlot slot, Texture2D texture);
void clearTextureSlot(Model& model, MaterialTextureSlot slot);

} // namespace engine
