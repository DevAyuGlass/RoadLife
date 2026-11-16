
#include <SDL3/SDL.h>
#include <epoxy/gl.h>

#include "../Camera/Camera.h"
#include "../World/Scene.h"
#include "../Console/DeveloperConsole.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <string>
#include <algorithm>

int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL error: " << SDL_GetError() << '\n';
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE
    );

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window* window = SDL_CreateWindow(
        "RoadLife Engine 0.3",
        1280,
        720,
        SDL_WINDOW_OPENGL |
        SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_HIGH_PIXEL_DENSITY
    );

    if (!window)
    {
        std::cerr << "Window error: " << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);

    if (!context)
    {
        std::cerr << "OpenGL error: " << SDL_GetError() << '\n';

        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;
    }

    SDL_GL_SetSwapInterval(1);

    std::cout << "RoadLife Engine 0.3\n";
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << '\n';
    std::cout << "GPU: " << glGetString(GL_RENDERER) << '\n';

    {
        Scene scene;

        if (!scene.isValid())
        {
            std::cerr << "Scene initialization failed\n";

            SDL_GL_DestroyContext(context);
            SDL_DestroyWindow(window);
            SDL_Quit();

            return 1;
        }

        Camera camera;
        DeveloperConsole console;

        if (!console.isValid())
        {
            std::cerr
                << "Developer console initialization failed. "
                << "Game will continue without console.\n";
        }

        bool running = true;
        bool mouseCaptured = true;

        SDL_SetWindowRelativeMouseMode(window, true);

        Uint64 previousTime = SDL_GetPerformanceCounter();
        Uint64 fpsTime = previousTime;

        int frameCount = 0;
        int displayedFPS = 0;

        while (running)
        {
            Uint64 currentTime = SDL_GetPerformanceCounter();

            float deltaTime =
                static_cast<float>(
                    static_cast<double>(currentTime - previousTime) /
                    static_cast<double>(SDL_GetPerformanceFrequency())
                );

            previousTime = currentTime;
            deltaTime = std::min(deltaTime, 0.05f);

            SDL_Event event;

            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_EVENT_QUIT)
                {
                    running = false;
                }

                if (event.type == SDL_EVENT_KEY_DOWN &&
                    !event.key.repeat &&
                    (event.key.key == SDLK_F1 ||
                     event.key.key == SDLK_GRAVE) &&
                    console.isValid())
                {
                    console.toggle(window);

                    mouseCaptured = !console.isOpen();

                    SDL_SetWindowRelativeMouseMode(
                        window,
                        mouseCaptured
                    );

                    continue;
                }

                if (console.isOpen())
                {
                    if (event.type == SDL_EVENT_KEY_DOWN &&
                        !event.key.repeat &&
                        event.key.key == SDLK_ESCAPE)
                    {
                        console.toggle(window);

                        mouseCaptured = true;

                        SDL_SetWindowRelativeMouseMode(
                            window,
                            true
                        );
                    }
                    else
                    {
                        console.handleEvent(
                            event,
                            scene,
                            static_cast<float>(displayedFPS)
                        );
                    }

                    continue;
                }

                if (event.type == SDL_EVENT_KEY_DOWN &&
                    !event.key.repeat &&
                    event.key.key == SDLK_ESCAPE)
                {
                    mouseCaptured = !mouseCaptured;

                    SDL_SetWindowRelativeMouseMode(
                        window,
                        mouseCaptured
                    );
                }

                if (event.type == SDL_EVENT_MOUSE_MOTION &&
                    mouseCaptured)
                {
                    camera.processMouse(
                        static_cast<float>(event.motion.xrel),
                        static_cast<float>(event.motion.yrel)
                    );
                }
            }

            if (!console.isOpen())
            {
                const bool* keys = SDL_GetKeyboardState(nullptr);

                camera.processKeyboard(keys, deltaTime);
            }

            if (!console.isTimePaused())
            {
                scene.update(
                    deltaTime * console.getTimeScale()
                );
            }

            int width = 0;
            int height = 0;

            SDL_GetWindowSizeInPixels(
                window,
                &width,
                &height
            );

            if (width <= 0 || height <= 0)
                continue;

            glViewport(0, 0, width, height);

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            glClearColor(
                0.055f,
                0.065f,
                0.085f,
                1.0f
            );

            glClear(
                GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT
            );

            glm::mat4 projection = glm::perspective(
                glm::radians(70.0f),
                static_cast<float>(width) /
                    static_cast<float>(height),
                0.1f,
                1000.0f
            );

            scene.render(
                camera.getViewMatrix(),
                projection
            );

            console.render(
                width,
                height,
                scene,
                static_cast<float>(displayedFPS)
            );

            SDL_GL_SwapWindow(window);

            ++frameCount;

            if (currentTime - fpsTime >=
                SDL_GetPerformanceFrequency())
            {
                displayedFPS = frameCount;

                frameCount = 0;
                fpsTime = currentTime;

                std::string title =
                    "RoadLife Engine 0.3 | FPS: " +
                    std::to_string(displayedFPS);

                SDL_SetWindowTitle(
                    window,
                    title.c_str()
                );
            }
        }
    }

    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

