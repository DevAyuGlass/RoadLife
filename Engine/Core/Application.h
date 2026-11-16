#pragma once

#include "Time.h"
#include "../Platform/Window/Window.h"
#include "../Camera/Camera.h"

#include <memory>

class Scene;
class DeveloperConsole;

class Application
{
public:
    Application() = default;
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool initialize();
    void run();
    void shutdown();

private:
    Window window;
    Time time;
    Camera camera;

    std::unique_ptr<Scene> scene;
    std::unique_ptr<DeveloperConsole> console;

    bool running = false;
    bool mouseCaptured = true;
    bool sdlInitialized = false;
    bool initialized = false;

    int lastWindowFPS = -1;

    void processEvents();
    void update();
    void render();
};
