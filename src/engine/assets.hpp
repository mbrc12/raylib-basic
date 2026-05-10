#pragma once

#include <string>

#include "raylib.h"
#include "shader.hpp"

namespace engine::assets {

void init();
void manual_unload();

Shader* shader(const char* name);
Texture2D* texture(const char* name);
Font* font(const char* name);
Model* model(const char* name);
std::string text(const char* name);

Image image(const char* name);

} // namespace engine::assets
