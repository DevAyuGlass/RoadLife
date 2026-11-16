#include "Application.h"

#include "../Renderer/IRenderer.h"
#include "../Renderer/OpenGL/OpenGLRenderer.h"
#include "../World/Scene.h"
#include "../Console/DeveloperConsole.h"

#include <epoxy/gl.h>

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

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL initialization error: " << SDL_GetError() << '\n';
        return false;
    }

    sdlInitialized = true;

    if (!window.create("RoadLife Engine 0.4", 1280, 720))
    {
        shutdown();
        return false;
    }

    std::cout << "RoadLife Engine 0.4\n";
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << '\n';
    std::cout << "GPU: " << glGetString(GL_RENDERER) << '\n';

    renderer = std::make_unique<OpenGLRenderer>();

    if (!renderer->initialize())
    {
        std::cerr << "Renderer initialization failed\n";
        shutdown();
        return false;
    }

    scene = std::make_unique<Scene>();

    if (!scene->isValid())
    {
        std::cerr << "Scene initialization failed\n";
        shutdown();
        return false;
    }

    console = std::make_unique<DeveloperConsole>();

    if (!console->isValid())
    {
        std::cerr
            << "Developer console initialization failed. "
            << "The application will continue without the console.\n";
    }

    mouseCaptured = true;
    window.setMouseCaptured(mouseCaptured);

    time.initialize();

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
    }
}

void Application::update()
{
    const bool consoleOpen =
        console && console->isValid() && console->isOpen();

    if (!consoleOpen)
    {
        const bool* keys = SDL_GetKeyboardState(nullptr);
        camera.processKeyboard(keys, time.getDeltaTime());
    }

    const bool timePaused =
        console && console->isValid() && console->isTimePaused();

    const float timeScale =
        console && console->isValid() ? console->getTimeScale() : 1.0f;

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

    if (console && console->isValid())
    {
        console->render(
            width,
            height,
            *scene,
            static_cast<float>(time.getFPS())
        );
    }

    window.swapBuffers();

    const int currentFPS = time.getFPS();

    if (currentFPS != lastWindowFPS)
    {
        const std::string title =
            "RoadLife Engine 0.4 | FPS: " +
            std::to_string(currentFPS);

        SDL_SetWindowTitle(window.getHandle(), title.c_str());
        lastWindowFPS = currentFPS;
    }
}

void Application::shutdown()
{
    running = false;

    // GL-backed resources must be destroyed before the GL context.
    console.reset();
    scene.reset();

    if (renderer)
        renderer->shutdown();
    renderer.reset();

    window.destroy();

    if (sdlInitialized)
    {
        SDL_Quit();
        sdlInitialized = false;
    }

    initialized = false;
}
