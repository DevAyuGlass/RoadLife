#include "Application.h"

#if defined(__ANDROID__)
#include <SDL3/SDL_main.h>
#endif

#include <iostream>
#if defined(__ANDROID__)
#include <SDL3/SDL.h>
#endif

int main(int argc, char* argv[])
{
    static_cast<void>(argc);
    static_cast<void>(argv);

#if defined(__ANDROID__)
    SDL_Log("RoadLife startup: entering main()");
#endif
    Application application;

    if (!application.initialize())
    {
#if defined(__ANDROID__)
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "RoadLife startup: Application::initialize() FAILED");
#endif
        std::cerr << "RoadLife initialization failed\n";
        return 1;
    }

#if defined(__ANDROID__)
    SDL_Log("RoadLife startup: initialization succeeded; entering game loop");
#endif
    application.run();
#if defined(__ANDROID__)
    SDL_Log("RoadLife startup: application.run() returned; shutting down");
#endif
    application.shutdown();

    return 0;
}
