#pragma once

#include <string>

#include "raylib.h"
#include "resource.hpp"
#include "shader.hpp"

namespace engine::assets {

void init();

Resource<Shader> shader(const char* name);
Resource<Texture2D> texture(const char* name);
Resource<Font> font(const char* name);

Image image(const char* name);
std::string readFile(const char* path);

} // namespace engine::assets
