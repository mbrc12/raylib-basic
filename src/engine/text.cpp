#include "engine/text.hpp"
#include "engine/assets.hpp"
#include "engine/vec2.hpp"

namespace engine {

Font defaultFont() {
    return *assets::font("main");
}

void drawText(const std::string& text, int posX, int posY, Color color, int fontsize) {
    auto font = defaultFont();
    if (fontsize == -1) fontsize = font.baseSize;
    DrawTextEx(font, text.c_str(), {static_cast<float>(posX), static_cast<float>(posY)}, static_cast<float>(fontsize), 0.0f, color);
}

Vec2f measureText(const std::string& text, int fontSize) {
    Font font = defaultFont();
    if (fontSize == -1) fontSize = font.baseSize;
    Vector2 size = MeasureTextEx(font, text.c_str(), static_cast<float>(fontSize), 0.0f);
    return size;
}

} // namespace engine
