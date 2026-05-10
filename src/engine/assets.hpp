#pragma once

#include "raylib.h"

#include <string>

namespace engine {

struct ShaderSources {
    std::string vertex;
    std::string fragment;
};

std::string readFile(const char *path);

Texture2D loadTexture(const char *name);
Image loadImage(const char *name);
ShaderSources loadShader(const char *name);
Font loadFont(const char *name);

} // namespace engine
