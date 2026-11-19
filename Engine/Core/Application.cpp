#include "Application.h"

#include "../Renderer/IRenderer.h"
#include "../Renderer/OpenGL/OpenGLRenderer.h"
#include "../World/Scene.h"
#if !defined(__ANDROID__)
#include "../Console/DeveloperConsole.h"
#endif
#include "../Renderer/GL.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <string>

Application::Application() = default;

Application::~Application()
{
    shutdown();
}

bool Application::initialize()
{
    if (initialized)
        return true;

#if defined(__ANDROID__)
    SDL_Log("RoadLife startup [1/6]: SDL_Init");
#endif
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
#if defined(__ANDROID__)
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed: %s", SDL_GetError());
#endif
        std::cerr << "SDL initialization error: " << SDL_GetError() << '\n';
        return false;
    }

    sdlInitialized = true;
#if defined(__ANDROID__)
    SDL_Log("RoadLife startup [2/6]: SDL initialized; creating window and GLES context");
#endif

    if (!window.create("RoadLife Engine 0.4", 1280, 720))
    {
        shutdown();
        return false;
    }

#if defined(__ANDROID__)
    const auto* glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const auto* glRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    SDL_Log("RoadLife startup [3/6]: GL_VERSION=%s; GL_RENDERER=%s",
            glVersion ? glVersion : "(null)",
            glRenderer ? glRenderer : "(null)");
#endif
    std::cout << "RoadLife Engine 0.4\n";
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << '\n';
    std::cout << "GPU: " << glGetString(GL_RENDERER) << '\n';

#if defined(__ANDROID__)
    SDL_Log("RoadLife startup [4/6]: initializing renderer");
#endif
    renderer = std::make_unique<OpenGLRenderer>();

    if (!renderer->initialize())
    {
#if defined(__ANDROID__)
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "RoadLife startup: renderer initialization FAILED");
#endif
        std::cerr << "Renderer initialization failed\n";
        shutdown();
        return false;
    }

#if defined(__ANDROID__)
    SDL_Log("RoadLife startup [5/6]: constructing scene and GPU resources");
#endif
    scene = std::make_unique<Scene>();

    if (!scene->isValid())
    {
#if defined(__ANDROID__)
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "RoadLife startup: Scene::isValid() FAILED");
#endif
        std::cerr << "Scene initialization failed\n";
        shutdown();
        return false;
    }

#if !defined(__ANDROID__)
    console = std::make_unique<DeveloperConsole>();

    if (!console->isValid())
    {
        std::cerr
            << "Developer console initialization failed. "
            << "The application will continue without the console.\n";
    }
    mouseCaptured = true;
#else
    mouseCaptured = false;
#endif
    window.setMouseCaptured(mouseCaptured);

    time.initialize();
#if defined(__ANDROID__)
    SDL_Log("RoadLife startup [6/6]: all systems initialized");
#endif

    running = true;
    initialized = true;

    return true;
}

void Application::run()
{
    if (!initialized)
        return;

    while (running)
    {
        time.update();

        processEvents();

        if (!running)
            break;

        update();
        render();
    }
}

void Application::processEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            running = false;
            continue;
        }

        input.handleEvent(event, window.getWidth(), window.getHeight());

#if !defined(__ANDROID__)
        const bool consoleAvailable =
            console && console->isValid();

        if (event.type == SDL_EVENT_KEY_DOWN &&
            !event.key.repeat &&
            (event.key.key == SDLK_F1 ||
             event.key.key == SDLK_GRAVE) &&
            consoleAvailable)
        {
            console->toggle(window.getHandle());

            mouseCaptured = !console->isOpen();
            window.setMouseCaptured(mouseCaptured);
            continue;
        }

        if (consoleAvailable && console->isOpen())
        {
            if (event.type == SDL_EVENT_KEY_DOWN &&
                !event.key.repeat &&
                event.key.key == SDLK_ESCAPE)
            {
                console->toggle(window.getHandle());

                mouseCaptured = true;
                window.setMouseCaptured(true);
            }
            else
            {
                console->handleEvent(
                    event,
                    *scene,
                    camera,
                    static_cast<float>(time.getFPS())
                );
            }

            continue;
        }

#endif

        if (event.type == SDL_EVENT_KEY_DOWN &&
            !event.key.repeat &&
            event.key.key == SDLK_V)
        {
            const Camera::Mode nextMode =
                camera.getMode() == Camera::Mode::Flight
                    ? Camera::Mode::Walk
                    : Camera::Mode::Flight;
            camera.setMode(nextMode, *scene);
            std::cout << "Movement mode: " << camera.getModeName()
                      << " (V to switch)\\n";
        }

        if (event.type == SDL_EVENT_KEY_DOWN &&
            !event.key.repeat &&
            event.key.key == SDLK_ESCAPE)
        {
            mouseCaptured = !mouseCaptured;
            window.setMouseCaptured(mouseCaptured);
        }

        if (event.type == SDL_EVENT_MOUSE_MOTION && mouseCaptured)
        {
            camera.processMouse(
                static_cast<float>(event.motion.xrel),
                static_cast<float>(event.motion.yrel)
            );
        }

        if (event.type == SDL_EVENT_MOUSE_WHEEL && mouseCaptured)
        {
            camera.setFieldOfView(
                camera.getFieldOfView() -
                static_cast<float>(event.wheel.y) * 2.0f
            );
        }
    }
}

void Application::update()
{
#if !defined(__ANDROID__)
    const bool consoleOpen = console && console->isValid() && console->isOpen();
    const bool timePaused = console && console->isValid() && console->isTimePaused();
    const float timeScale = console && console->isValid() ? console->getTimeScale() : 1.0f;
#else
    constexpr bool timePaused = false;
    constexpr float timeScale = 1.0f;
#endif
    input.update(camera, *scene, time.getDeltaTime(),
#if !defined(__ANDROID__)
        !consoleOpen
#else
        true
#endif
    );

    if (!timePaused)
        scene->update(time.getDeltaTime() * timeScale);
}

void Application::render()
{
    if (!scene || !renderer || !window.isValid())
        return;

    int width = 0;
    int height = 0;
    window.getPixelSize(width, height);

    if (width <= 0 || height <= 0)
        return;

    renderer->beginFrame(width, height);

    const glm::mat4 projection = glm::perspective(
        glm::radians(camera.getFieldOfView()),
        static_cast<float>(width) / static_cast<float>(height),
        0.1f,
        1000.0f
    );

    scene->render(camera.getViewMatrix(), projection);

#if !defined(__ANDROID__)
    if (console && console->isValid())
    {
        console->render(width, height, *scene, static_cast<float>(time.getFPS()));
    }
#endif

    window.swapBuffers();

    const int currentFPS = time.getFPS();

    if (currentFPS != lastWindowFPS)
    {
        const std::string title =
            "RoadLife Engine 0.4 | FPS: " +
            std::to_string(currentFPS) +
            " | MODE: " + camera.getModeName() + " (V)";

        SDL_SetWindowTitle(window.getHandle(), title.c_str());
        lastWindowFPS = currentFPS;
    }
}

void Application::shutdown()
{
    running = false;

    // GL-backed resources must be destroyed before the GL context.
#if !defined(__ANDROID__)
    console.reset();
#endif
    scene.reset();

    if (renderer)
        renderer->shutdown();
    renderer.reset();

    window.destroy();

    input.shutdown();

    if (sdlInitialized)
    {
        SDL_Quit();
        sdlInitialized = false;
    }

    initialized = false;
}
