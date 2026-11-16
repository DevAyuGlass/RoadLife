
#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
    Camera();

    void processKeyboard(const bool* keys, float deltaTime);
    void processMouse(float xOffset, float yOffset);

    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const;
    float getMovementSpeed() const;
    void setMovementSpeed(float speed);
    float getFieldOfView() const;
    void setFieldOfView(float degrees);
    void setPosition(const glm::vec3& value);
    void reset();

private:
    glm::vec3 position;
    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;
    float fieldOfView;

    glm::vec3 getFront() const;
};

