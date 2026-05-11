#pragma once

#include <string>

namespace engine::input {

inline constexpr float defaultMouseMultiplier = 0.4f;
inline constexpr float defaultGamepadMultiplier = 12.0f;

enum class Action {
    Left,
    Right,
    Up,
    Down,
    Interact,
    Back,
    Count,
};

enum class AnalogAction {
    X2,
    Y2,
    Count,
};

void init();
void update();
void setMouseMultiplier(float multiplier);
void setGamepadMultiplier(float multiplier);

bool down(Action action);
bool just_pressed(Action action);
bool just_released(Action action);

float value(AnalogAction action);

std::string compressed_state();

} // namespace engine::input
