
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

private:
    glm::vec3 position;
    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;

    glm::vec3 getFront() const;
};

