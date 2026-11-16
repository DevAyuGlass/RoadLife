#include "Application.h"

#if defined(__ANDROID__)
#include <SDL3/SDL_main.h>
#endif

#include <iostream>

int main()
{
    Application application;

    if (!application.initialize())
    {
        std::cerr << "RoadLife initialization failed\n";
        return 1;
    }

    application.run();
    application.shutdown();

    return 0;
}
