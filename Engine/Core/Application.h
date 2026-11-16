#pragma once

#include "Time.h"
#include "../Platform/Window/Window.h"
#include "../Camera/Camera.h"
#include "../Input/InputManager.h"

#include <memory>

class IRenderer;
class Scene;
class DeveloperConsole;

class Application
{
public:
    Application();
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
    InputManager input;

    std::unique_ptr<IRenderer> renderer;
    std::unique_ptr<Scene> scene;
#if !defined(__ANDROID__)
    std::unique_ptr<DeveloperConsole> console;
#endif

    bool running = false;
    bool mouseCaptured = true;
    bool sdlInitialized = false;
    bool initialized = false;

    int lastWindowFPS = -1;

    void processEvents();
    void update();
    void render();
};
