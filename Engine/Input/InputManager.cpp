#include "InputManager.h"

#include "../Camera/Camera.h"
#include "../World/Scene.h"

#include <algorithm>
#include <cmath>

InputManager::~InputManager()
{
    shutdown();
}

void InputManager::shutdown()
{
    closeGamepad();
    reset();
}

void InputManager::openGamepad(SDL_JoystickID deviceIndex)
{
    if (gamepad)
        return;

    gamepad = SDL_OpenGamepad(deviceIndex);
    if (gamepad)
        gamepadId = SDL_GetGamepadID(gamepad);
}

void InputManager::closeGamepad()
{
    if (gamepad)
    {
        SDL_CloseGamepad(gamepad);
        gamepad = nullptr;
    }

    gamepadId = 0;
}

float InputManager::applyDeadzone(float value, float deadzone)
{
    const float magnitude = std::abs(value);
    if (magnitude <= deadzone)
        return 0.0f;

    const float scaled = (magnitude - deadzone) / (1.0f - deadzone);
    return std::copysign(std::min(scaled, 1.0f), value);
}

void InputManager::handleEvent(
    const SDL_Event& event,
    int width,
    int height
)
{
    viewportWidth = std::max(1, width);
    viewportHeight = std::max(1, height);

    if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST)
    {
        reset();
        return;
    }

    if (event.type == SDL_EVENT_GAMEPAD_ADDED)
    {
        openGamepad(event.gdevice.which);
        return;
    }

    if (event.type == SDL_EVENT_GAMEPAD_REMOVED)
    {
        if (gamepad && event.gdevice.which == gamepadId)
            closeGamepad();
        return;
    }

    if (event.type == SDL_EVENT_FINGER_DOWN)
    {
        const float x = event.tfinger.x;
        const float pixelX = x * static_cast<float>(viewportWidth);

        if (pixelX < static_cast<float>(viewportWidth) * 0.46f &&
            !hasMoveFinger)
        {
            moveFinger = event.tfinger.fingerID;
            hasMoveFinger = true;
            moveX = 0.0f;
            moveY = 0.0f;
        }
        else if (!hasLookFinger)
        {
            lookFinger = event.tfinger.fingerID;
            hasLookFinger = true;
        }

        return;
    }

    if (event.type == SDL_EVENT_FINGER_MOTION)
    {
        if (hasMoveFinger && event.tfinger.fingerID == moveFinger)
        {
            const float dx =
                event.tfinger.dx * static_cast<float>(viewportWidth);
            const float dy =
                event.tfinger.dy * static_cast<float>(viewportHeight);

            const float radius =
                static_cast<float>(std::min(viewportWidth, viewportHeight)) * 0.16f;

            moveX = std::clamp(moveX + dx / std::max(radius, 1.0f), -1.0f, 1.0f);
            moveY = std::clamp(moveY + dy / std::max(radius, 1.0f), -1.0f, 1.0f);
        }
        else if (hasLookFinger && event.tfinger.fingerID == lookFinger)
        {
            lookDeltaX +=
                event.tfinger.dx * static_cast<float>(viewportWidth);
            lookDeltaY +=
                event.tfinger.dy * static_cast<float>(viewportHeight);
        }

        return;
    }

    if (event.type == SDL_EVENT_FINGER_UP)
    {
        if (hasMoveFinger && event.tfinger.fingerID == moveFinger)
        {
            hasMoveFinger = false;
            moveFinger = 0;
            moveX = 0.0f;
            moveY = 0.0f;
        }

        if (hasLookFinger && event.tfinger.fingerID == lookFinger)
        {
            hasLookFinger = false;
            lookFinger = 0;
        }
    }
}

void InputManager::update(
    Camera& camera,
    Scene& scene,
    float deltaTime,
    bool enabled
)
{
    if (!enabled)
    {
        reset();
        return;
    }

    const bool* keys = SDL_GetKeyboardState(nullptr);

    float forward = 0.0f;
    float strafe = 0.0f;
    float vertical = 0.0f;

    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])
        forward += 1.0f;
    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])
        forward -= 1.0f;
    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT])
        strafe += 1.0f;
    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])
        strafe -= 1.0f;
    if (keys[SDL_SCANCODE_SPACE])
        vertical += 1.0f;
    if (keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL])
        vertical -= 1.0f;

    bool sprint =
        keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];

    if (gamepad)
    {
        const float leftX = applyDeadzone(
            static_cast<float>(SDL_GetGamepadAxis(
                gamepad, SDL_GAMEPAD_AXIS_LEFTX)) / 32767.0f,
            0.18f
        );
        const float leftY = applyDeadzone(
            static_cast<float>(SDL_GetGamepadAxis(
                gamepad, SDL_GAMEPAD_AXIS_LEFTY)) / 32767.0f,
            0.18f
        );
        const float rightX = applyDeadzone(
            static_cast<float>(SDL_GetGamepadAxis(
                gamepad, SDL_GAMEPAD_AXIS_RIGHTX)) / 32767.0f,
            0.16f
        );
        const float rightY = applyDeadzone(
            static_cast<float>(SDL_GetGamepadAxis(
                gamepad, SDL_GAMEPAD_AXIS_RIGHTY)) / 32767.0f,
            0.16f
        );

        strafe += leftX;
        forward -= leftY;

        camera.processMouse(rightX * 7.5f, rightY * 7.5f);

        sprint = sprint ||
            SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_LEFT_STICK) ||
            SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 16000;
    }

    // Touch controls: left-side virtual stick, right-side camera drag.
    strafe += applyDeadzone(moveX, 0.10f);
    forward -= applyDeadzone(moveY, 0.10f);

    camera.processMouse(lookDeltaX, lookDeltaY);
    lookDeltaX = 0.0f;
    lookDeltaY = 0.0f;

    strafe = std::clamp(strafe, -1.0f, 1.0f);
    forward = std::clamp(forward, -1.0f, 1.0f);
    vertical = std::clamp(vertical, -1.0f, 1.0f);

    camera.processMovement(forward, strafe, vertical, sprint, scene, deltaTime);
}

void InputManager::reset()
{
    moveX = 0.0f;
    moveY = 0.0f;
    lookDeltaX = 0.0f;
    lookDeltaY = 0.0f;
    hasMoveFinger = false;
    hasLookFinger = false;
    moveFinger = 0;
    lookFinger = 0;
}

bool InputManager::hasGamepad() const
{
    return gamepad != nullptr;
}
