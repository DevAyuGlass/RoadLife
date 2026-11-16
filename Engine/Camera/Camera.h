
#pragma once

#include <glm/glm.hpp>

class Scene;

class Camera
{
public:
    enum class Mode
    {
        Flight,
        Walk
    };

    Camera();

    void setMode(Mode value);
    Mode getMode() const;
    const char* getModeName() const;

    void processMovement(float forward, float strafe, float vertical,
                         bool sprint, Scene& scene, float deltaTime);
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
    Mode mode = Mode::Flight;
    float verticalVelocity = 0.0f;
    bool grounded = false;
    bool jumpHeld = false;
    float currentEyeHeight = 0.63f;
    glm::vec3 horizontalVelocity{0.0f};

    glm::vec3 getFront() const;
};

