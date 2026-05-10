#include "input.hpp"

#include "assets.hpp"
#include "raylib.h"

#include <array>
#include <cstdio>
#include <initializer_list>
#include <string>

namespace engine::input {
namespace {

constexpr int kGamepadCount = 4;
constexpr float kStickToMouseScale = 12.0f;
constexpr float kStickDeadzone = 0.15f;

constexpr size_t index(Action action) {
    return static_cast<size_t>(action);
}

constexpr size_t index(AnalogAction action) {
    return static_cast<size_t>(action);
}

std::array<bool, index(Action::Count)> gDown{};
std::array<bool, index(Action::Count)> gPrevDown{};
std::array<float, index(AnalogAction::Count)> gAnalog{};
float gMouseMultiplier = defaultMouseMultiplier;
float gGamepadMultiplier = defaultGamepadMultiplier;
int gActiveGamepad = -2;

int activeGamepad() {
    for (int pad = 0; pad < kGamepadCount; pad++) {
        if (IsGamepadAvailable(pad)) {
            return pad;
        }
    }
    return -1;
}

bool anyKeyDown(std::initializer_list<KeyboardKey> keys) {
    for (KeyboardKey key : keys) {
        if (IsKeyDown(key)) {
            return true;
        }
    }
    return false;
}

bool anyButtonDown(int gamepad, std::initializer_list<GamepadButton> buttons) {
    if (gamepad < 0) {
        return false;
    }
    for (GamepadButton button : buttons) {
        if (IsGamepadButtonDown(gamepad, button)) {
            return true;
        }
    }
    return false;
}

float applyDeadzone(float value) {
    if (value > -kStickDeadzone && value < kStickDeadzone) {
        return 0.0f;
    }
    return value;
}

const char* gamepadName(int gamepad) {
    return (gamepad >= 0) ? GetGamepadName(gamepad) : "<none>";
}

} // namespace

void init() {
    gDown.fill(false);
    gPrevDown.fill(false);
    gAnalog.fill(0.0f);
    gMouseMultiplier = defaultMouseMultiplier;
    gGamepadMultiplier = defaultGamepadMultiplier;

    std::string mappings = assets::text("gamepad_mappings");
    if (!mappings.empty()) {
        int loaded = SetGamepadMappings(mappings.c_str());
        TraceLog(LOG_INFO, "INPUT: loaded %d gamepad mappings", loaded);
    } else {
        TraceLog(LOG_WARNING, "INPUT: could not load gamepad mappings");
    }

    gActiveGamepad = activeGamepad();
}

void setMouseMultiplier(float multiplier) {
    gMouseMultiplier = std::max(0.0f, multiplier);
}

void setGamepadMultiplier(float multiplier) {
    gGamepadMultiplier = std::max(0.0f, multiplier);
}

void update() {
    gPrevDown = gDown;

    int gamepad = activeGamepad();
    if (gamepad != gActiveGamepad) {
        TraceLog(LOG_INFO, "INPUT: active gamepad changed from %d (%s) to %d (%s)",
                 gActiveGamepad, gamepadName(gActiveGamepad), gamepad, gamepadName(gamepad));
        gActiveGamepad = gamepad;
    }

    gDown[index(Action::Left)] =
        anyKeyDown({KEY_A, KEY_LEFT}) ||
        anyButtonDown(gamepad, {GAMEPAD_BUTTON_LEFT_FACE_LEFT}) ||
        (gamepad >= 0 && GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X) < -kStickDeadzone);
    gDown[index(Action::Right)] =
        anyKeyDown({KEY_D, KEY_RIGHT}) ||
        anyButtonDown(gamepad, {GAMEPAD_BUTTON_LEFT_FACE_RIGHT}) ||
        (gamepad >= 0 && GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X) > kStickDeadzone);
    gDown[index(Action::Up)] =
        anyKeyDown({KEY_W, KEY_UP}) ||
        anyButtonDown(gamepad, {GAMEPAD_BUTTON_LEFT_FACE_UP}) ||
        (gamepad >= 0 && GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y) < -kStickDeadzone);
    gDown[index(Action::Down)] =
        anyKeyDown({KEY_S, KEY_DOWN}) ||
        anyButtonDown(gamepad, {GAMEPAD_BUTTON_LEFT_FACE_DOWN}) ||
        (gamepad >= 0 && GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y) > kStickDeadzone);
    gDown[index(Action::Interact)] =
        anyKeyDown({KEY_E, KEY_ENTER, KEY_KP_ENTER}) || anyButtonDown(gamepad, {GAMEPAD_BUTTON_RIGHT_FACE_DOWN});
    gDown[index(Action::Back)] =
        anyKeyDown({KEY_ESCAPE, KEY_BACKSPACE}) || anyButtonDown(gamepad, {GAMEPAD_BUTTON_RIGHT_FACE_RIGHT});

    Vector2 mouseLook = GetMouseDelta();
    float stickX = 0.0f;
    float stickY = 0.0f;
    if (gamepad >= 0) {
        stickX = applyDeadzone(GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_X));
        stickY = applyDeadzone(GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_Y));
    }

    gAnalog[index(AnalogAction::X2)] = mouseLook.x * gMouseMultiplier + stickX * gGamepadMultiplier;
    gAnalog[index(AnalogAction::Y2)] = -mouseLook.y * gMouseMultiplier - stickY * gGamepadMultiplier;
}

bool down(Action action) { return gDown[index(action)]; }

bool just_pressed(Action action) {
    size_t idx = index(action);
    return gDown[idx] && !gPrevDown[idx];
}

bool just_released(Action action) {
    size_t idx = index(action);
    return !gDown[idx] && gPrevDown[idx];
}

float value(AnalogAction action) { return gAnalog[index(action)]; }

std::string compressed_state() {
    char buffer[128];
    std::snprintf(buffer, sizeof(buffer), "%c%c%c%c%c%c,X2:%.2f,Y2:%.2f",
                  down(Action::Left) ? 'L' : '_',
                  down(Action::Right) ? 'R' : '_',
                  down(Action::Up) ? 'U' : '_',
                  down(Action::Down) ? 'D' : '_',
                  down(Action::Interact) ? 'I' : '_',
                  down(Action::Back) ? 'B' : '_',
                  value(AnalogAction::X2),
                  value(AnalogAction::Y2));
    return buffer;
}

} // namespace engine::input
