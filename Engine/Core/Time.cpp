#include "Time.h"

#include <algorithm>

void Time::initialize()
{
    frequency = SDL_GetPerformanceFrequency();
    previousCounter = SDL_GetPerformanceCounter();
    fpsCounter = previousCounter;

    deltaTime = 0.0f;
    elapsedTime = 0.0f;
    frameCount = 0;
    fps = 0;
}

void Time::update()
{
    if (frequency == 0)
    {
        deltaTime = 0.0f;
        return;
    }

    const Uint64 currentCounter = SDL_GetPerformanceCounter();
    const Uint64 elapsedCounter = currentCounter - previousCounter;

    previousCounter = currentCounter;

    deltaTime = static_cast<float>(
        static_cast<double>(elapsedCounter) /
        static_cast<double>(frequency)
    );

    deltaTime = std::min(deltaTime, 0.05f);
    elapsedTime += deltaTime;

    ++frameCount;

    if (currentCounter - fpsCounter >= frequency)
    {
        fps = frameCount;
        frameCount = 0;
        fpsCounter = currentCounter;
    }
}

float Time::getDeltaTime() const
{
    return deltaTime;
}

float Time::getElapsedTime() const
{
    return elapsedTime;
}

int Time::getFPS() const
{
    return fps;
}
