#include "Application.h"

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
