#pragma once

#include <raylib.h>
#include <string>
#include "engine/vec2.hpp"

namespace engine {
    
   void drawText(const std::string& text, int posX, int posY, Color color, int fontsize = -1);
   Vec2f measureText(const std::string& text, int fontSize = -1);
    
};
