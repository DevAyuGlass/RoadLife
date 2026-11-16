
#include "Camera.h"

#include <SDL3/SDL_scancode.h>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
    : position(0.0f, 0.0f, 5.0f),
      yaw(-90.0f),
      pitch(0.0f),
      movementSpeed(5.0f),
      mouseSensitivity(0.1f),
      fieldOfView(70.0f)
{
}

glm::vec3 Camera::getFront() const
{
    glm::vec3 direction;

    direction.x = cos(glm::radians(yaw)) *
                  cos(glm::radians(pitch));

    direction.y = sin(glm::radians(pitch));

    direction.z = sin(glm::radians(yaw)) *
                  cos(glm::radians(pitch));

    return glm::normalize(direction);
}

void Camera::setMode(Mode value)
{
    if (mode == value)
        return;

    mode = value;
    verticalVelocity = 0.0f;
    grounded = false;

    if (mode == Mode::Walk)
        position.y = 0.63f;
}

Camera::Mode Camera::getMode() const
{
    return mode;
}

const char* Camera::getModeName() const
{
    return mode == Mode::Walk ? "WALK" : "FLIGHT";
}

void Camera::processKeyboard(const bool* keys, float deltaTime)
{
    const float forward =
        (keys[SDL_SCANCODE_W] ? 1.0f : 0.0f) -
        (keys[SDL_SCANCODE_S] ? 1.0f : 0.0f);
    const float strafe =
        (keys[SDL_SCANCODE_D] ? 1.0f : 0.0f) -
        (keys[SDL_SCANCODE_A] ? 1.0f : 0.0f);
    const float vertical =
        (keys[SDL_SCANCODE_SPACE] ? 1.0f : 0.0f) -
        (keys[SDL_SCANCODE_LCTRL] ? 1.0f : 0.0f);
    const bool sprint =
        keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];

    processMovement(forward, strafe, vertical, sprint, deltaTime);
}

void Camera::processMovement(
    float forward,
    float strafe,
    float vertical,
    bool sprint,
    float deltaTime
)
{
    const float dt = glm::clamp(deltaTime, 0.0f, 0.05f);
    const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    const glm::vec3 front = getFront();

    if (mode == Mode::Flight)
    {
        const glm::vec3 right = glm::normalize(glm::cross(front, worldUp));
        glm::vec3 movement =
            front * forward +
            right * strafe +
            worldUp * vertical;

        const float movementLength = glm::length(movement);
        if (movementLength > 0.0f)
        {
            const float sprintMultiplier = sprint ? 3.0f : 1.0f;
            position += (movement / movementLength) *
                        movementSpeed * sprintMultiplier * dt;
        }
        return;
    }

    // Walk mode: horizontal movement, gravity, jumping and crouching.
    const glm::vec3 flatFront = glm::normalize(
        glm::vec3(front.x, 0.0f, front.z)
    );
    const glm::vec3 right = glm::normalize(glm::cross(flatFront, worldUp));
    glm::vec3 movement = flatFront * forward + right * strafe;
    const float movementLength = glm::length(movement);

    if (movementLength > 0.0f)
    {
        const float sprintMultiplier = sprint ? 1.8f : 1.0f;
        position += (movement / movementLength) *
                    movementSpeed * sprintMultiplier * dt;
    }

    constexpr float standingHeight = 0.63f;
    constexpr float crouchingHeight = 0.12f;
    constexpr float gravity = 18.0f;
    constexpr float jumpVelocity = 6.2f;

    const float targetHeight =
        vertical < -0.5f ? crouchingHeight : standingHeight;

    if (vertical > 0.5f && grounded)
    {
        verticalVelocity = jumpVelocity;
        grounded = false;
    }

    if (grounded)
    {
        // Keep grounded movement stable and make crouching independent of gravity.
        position.y = targetHeight;
        verticalVelocity = 0.0f;
    }
    else
    {
        verticalVelocity -= gravity * dt;
        position.y += verticalVelocity * dt;

        if (position.y <= targetHeight)
        {
            position.y = targetHeight;
            verticalVelocity = 0.0f;
            grounded = true;
        }
    }
}

void Camera::processMouse(float xOffset, float yOffset)
{
    yaw += xOffset * mouseSensitivity;
    pitch -= yOffset * mouseSensitivity;

    pitch = glm::clamp(pitch, -89.0f, 89.0f);
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(
        position,
        position + getFront(),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
}

glm::vec3 Camera::getPosition() const
{
    return position;
}

float Camera::getMovementSpeed() const
{
    return movementSpeed;
}

void Camera::setMovementSpeed(float speed)
{
    movementSpeed = glm::clamp(speed, 0.1f, 100.0f);
}

float Camera::getFieldOfView() const
{
    return fieldOfView;
}

void Camera::setFieldOfView(float degrees)
{
    fieldOfView = glm::clamp(degrees, 30.0f, 110.0f);
}

void Camera::setPosition(const glm::vec3& value)
{
    position = value;
    verticalVelocity = 0.0f;
    grounded = false;
}

void Camera::reset()
{
    position = glm::vec3(0.0f, 0.0f, 5.0f);
    yaw = -90.0f;
    pitch = 0.0f;
    movementSpeed = 5.0f;
    fieldOfView = 70.0f;
    mode = Mode::Flight;
    verticalVelocity = 0.0f;
    grounded = false;
}

