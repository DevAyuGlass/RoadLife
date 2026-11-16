#pragma once

#include <SDL3/SDL.h>

class Time
{
public:
    void initialize();
    void update();

    float getDeltaTime() const;
    float getElapsedTime() const;
    int getFPS() const;

private:
    Uint64 previousCounter = 0;
    Uint64 fpsCounter = 0;
    Uint64 frequency = 0;

    float deltaTime = 0.0f;
    float elapsedTime = 0.0f;

    int frameCount = 0;
    int fps = 0;
};
