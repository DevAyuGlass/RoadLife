#include "Camera.h"

#include "../World/Scene.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

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
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    return glm::normalize(direction);
}

void Camera::setMode(Mode value)
{
    if (mode == value)
        return;

    mode = value;
    verticalVelocity = 0.0f;
    horizontalVelocity = glm::vec3(0.0f);
    grounded = false;
    jumpHeld = false;
    currentEyeHeight = 0.63f;

    if (mode == Mode::Walk)
    {
        // Ground plane is at Y=-1.02; position is the camera/eye position.
        position.y = -1.02f + 0.63f;
        grounded = true;
    }
}

Camera::Mode Camera::getMode() const { return mode; }
const char* Camera::getModeName() const { return mode == Mode::Walk ? "WALK" : "FLIGHT"; }

void Camera::processMovement(
    float forward, float strafe, float vertical,
    bool sprint, Scene& scene, float deltaTime
)
{
    const float dt = glm::clamp(deltaTime, 0.0f, 0.05f);
    const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    const glm::vec3 front = getFront();

    if (mode == Mode::Flight)
    {
        const glm::vec3 right = glm::normalize(glm::cross(front, worldUp));
        glm::vec3 movement = front * forward + right * strafe + worldUp * vertical;
        const float movementLength = glm::length(movement);
        if (movementLength > 0.0f)
        {
            const float sprintMultiplier = sprint ? 3.0f : 1.0f;
            position += (movement / movementLength) * movementSpeed * sprintMultiplier * dt;
        }
        horizontalVelocity = glm::vec3(0.0f);
        return;
    }

    const glm::vec3 flatFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
    const glm::vec3 right = glm::normalize(glm::cross(flatFront, worldUp));
    glm::vec3 wishDirection = flatFront * forward + right * strafe;
    const float wishLength = glm::length(wishDirection);
    if (wishLength > 1.0f)
        wishDirection /= wishLength;

    constexpr float standingEyeHeight = 0.63f;
    constexpr float crouchingEyeHeight = 0.12f;
    constexpr float standingBodyHeight = 1.72f;
    constexpr float crouchingBodyHeight = 0.92f;
    constexpr float gravity = 18.0f;
    constexpr float jumpVelocity = 6.8f;
    constexpr float acceleration = 26.0f;
    constexpr float braking = 19.0f;

    const bool crouching = vertical < -0.5f;
    const float eyeHeight = crouching ? crouchingEyeHeight : standingEyeHeight;
    const float bodyHeight = crouching ? crouchingBodyHeight : standingBodyHeight;
    // Keep the player's feet in place when switching between standing and crouching.
    position.y += eyeHeight - currentEyeHeight;
    currentEyeHeight = eyeHeight;
    const glm::vec3 previousPosition = position;
    const float maxSpeed = movementSpeed * (sprint ? 1.85f : 1.0f) *
                           (crouching ? 0.48f : 1.0f);

    const glm::vec3 targetVelocity = wishDirection * maxSpeed;
    const glm::vec3 velocityDelta = targetVelocity - horizontalVelocity;
    const float maxChange = (wishLength > 0.001f ? acceleration : braking) * dt;
    const float deltaLength = glm::length(velocityDelta);
    if (deltaLength <= maxChange)
        horizontalVelocity = targetVelocity;
    else if (deltaLength > 0.0f)
        horizontalVelocity += velocityDelta * (maxChange / deltaLength);

    position.x += horizontalVelocity.x * dt;
    position.z += horizontalVelocity.z * dt;

    // Jump only on the press edge. Holding Space no longer auto-bunny-hops on landing.
    const bool jumpPressed = vertical > 0.5f && !jumpHeld;
    jumpHeld = vertical > 0.5f;

    if (jumpPressed && grounded)
    {
        verticalVelocity = jumpVelocity;
        grounded = false;
    }

    if (!grounded)
    {
        verticalVelocity -= gravity * dt;
        position.y += verticalVelocity * dt;
    }

    scene.resolvePlayerCollision(
        previousPosition, position, 0.32f, eyeHeight, bodyHeight,
        verticalVelocity, grounded
    );
}

void Camera::processMouse(float xOffset, float yOffset)
{
    yaw += xOffset * mouseSensitivity;
    pitch -= yOffset * mouseSensitivity;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(position, position + getFront(), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 Camera::getPosition() const { return position; }
float Camera::getMovementSpeed() const { return movementSpeed; }
void Camera::setMovementSpeed(float speed) { movementSpeed = glm::clamp(speed, 0.1f, 100.0f); }
float Camera::getFieldOfView() const { return fieldOfView; }
void Camera::setFieldOfView(float degrees) { fieldOfView = glm::clamp(degrees, 30.0f, 110.0f); }

void Camera::setPosition(const glm::vec3& value)
{
    position = value;
    verticalVelocity = 0.0f;
    horizontalVelocity = glm::vec3(0.0f);
    grounded = false;
    jumpHeld = false;
    currentEyeHeight = 0.63f;
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
    horizontalVelocity = glm::vec3(0.0f);
    grounded = false;
    jumpHeld = false;
    currentEyeHeight = 0.63f;
}
