
#include "Camera.h"

#include <SDL3/SDL_scancode.h>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
    : position(0.0f, 0.0f, 5.0f),
      yaw(-90.0f),
      pitch(0.0f),
      movementSpeed(5.0f),
      mouseSensitivity(0.1f)
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

void Camera::processKeyboard(const bool* keys, float deltaTime)
{
    glm::vec3 front = getFront();

    glm::vec3 right = glm::normalize(
        glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f))
    );

    float velocity = movementSpeed * deltaTime;

    if (keys[SDL_SCANCODE_W])
        position += front * velocity;

    if (keys[SDL_SCANCODE_S])
        position -= front * velocity;

    if (keys[SDL_SCANCODE_A])
        position -= right * velocity;

    if (keys[SDL_SCANCODE_D])
        position += right * velocity;

    if (keys[SDL_SCANCODE_SPACE])
        position.y += velocity;

    if (keys[SDL_SCANCODE_LCTRL])
        position.y -= velocity;
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

void Camera::setPosition(const glm::vec3& value)
{
    position = value;
}

void Camera::reset()
{
    position = glm::vec3(0.0f, 0.0f, 5.0f);
    yaw = -90.0f;
    pitch = 0.0f;
    movementSpeed = 5.0f;
}

