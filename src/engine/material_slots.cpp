#include "material_slots.hpp"

#include "shader.hpp"

namespace engine {
namespace {

constexpr int slotToRaylibMap(MaterialTextureSlot slot) {
    switch (slot) {
        case MaterialTextureSlot::ShadowMap:
            return MATERIAL_MAP_METALNESS;
    }
    return MATERIAL_MAP_METALNESS;
}

constexpr int slotToRaylibShader(MaterialTextureSlot slot) {
    switch (slot) {
        case MaterialTextureSlot::ShadowMap:
            return SHADER_LOC_MAP_METALNESS;
    }
    return SHADER_LOC_MAP_METALNESS;
}

} // namespace

void bindTextureSlot(Shader& shader, MaterialTextureSlot slot, const char* samplerName) {
    shader.bindLocation(slotToRaylibShader(slot), samplerName);
}

void setTextureSlot(Model& model, MaterialTextureSlot slot, Texture2D texture) {
    int mapIndex = slotToRaylibMap(slot);
    for (int i = 0; i < model.materialCount; ++i) {
        model.materials[i].maps[mapIndex].texture = texture;
    }
}

void clearTextureSlot(Model& model, MaterialTextureSlot slot) {
    setTextureSlot(model, slot, Texture2D{});
}

} // namespace engine
