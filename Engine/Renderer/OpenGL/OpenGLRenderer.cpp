#include "OpenGLRenderer.h"

#include "../GL.h"

#include <iostream>

bool OpenGLRenderer::initialize()
{
    if (initialized)
        return true;

    const auto* version = glGetString(GL_VERSION);

    if (!version)
    {
        std::cerr << "OpenGL renderer initialization failed: no current context\n";
        return false;
    }

    std::cout << "Renderer backend: OpenGL\n";
    initialized = true;
    return true;
}

void OpenGLRenderer::beginFrame(int width, int height)
{
    if (!initialized || width <= 0 || height <= 0)
        return;

    glViewport(0, 0, width, height);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClearColor(0.055f, 0.065f, 0.085f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::shutdown()
{
    initialized = false;
}
