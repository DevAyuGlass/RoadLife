#pragma once

#include <SDL3/SDL.h>

class Camera;

class InputManager
{
public:
    InputManager() = default;
    ~InputManager();

    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    void handleEvent(const SDL_Event& event, int viewportWidth, int viewportHeight);
    void shutdown();
    void update(Camera& camera, float deltaTime, bool enabled);
    void reset();

    bool hasGamepad() const;

private:
    SDL_Gamepad* gamepad = nullptr;
    SDL_GamepadID gamepadId = 0;

    SDL_FingerID moveFinger = 0;
    SDL_FingerID lookFinger = 0;
    bool hasMoveFinger = false;
    bool hasLookFinger = false;

    float moveX = 0.0f;
    float moveY = 0.0f;
    float lookDeltaX = 0.0f;
    float lookDeltaY = 0.0f;

    int viewportWidth = 1;
    int viewportHeight = 1;

    void openGamepad(SDL_JoystickID deviceIndex);
    void closeGamepad();
    static float applyDeadzone(float value, float deadzone);
};
