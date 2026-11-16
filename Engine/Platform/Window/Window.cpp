#include "Window.h"

#include <iostream>

Window::~Window()
{
    destroy();
}

bool Window::create(const char* title, int requestedWidth, int requestedHeight)
{
    if (isValid())
        return true;

#if defined(__ANDROID__)
    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES) ||
#else
    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) ||
#endif
        !SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24) ||
        !SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1))
    {
        std::cerr << "OpenGL attribute error: " << SDL_GetError() << '\n';
        return false;
    }

    handle = SDL_CreateWindow(
        title,
        requestedWidth,
        requestedHeight,
        SDL_WINDOW_OPENGL |
#if defined(__ANDROID__)
        SDL_WINDOW_FULLSCREEN |
#else
        SDL_WINDOW_RESIZABLE |
#endif
        SDL_WINDOW_HIGH_PIXEL_DENSITY
    );

    if (!handle)
    {
        std::cerr << "Window creation error: " << SDL_GetError() << '\n';
        return false;
    }

    context = SDL_GL_CreateContext(handle);

    if (!context)
    {
        std::cerr << "OpenGL context error: " << SDL_GetError() << '\n';
        destroy();
        return false;
    }

    width = requestedWidth;
    height = requestedHeight;

    static_cast<void>(SDL_GL_SetSwapInterval(1));

    return true;
}

void Window::destroy()
{
    if (context)
    {
        SDL_GL_DestroyContext(context);
        context = nullptr;
    }

    if (handle)
    {
        SDL_DestroyWindow(handle);
        handle = nullptr;
    }

    width = 0;
    height = 0;
}

void Window::swapBuffers() const
{
    if (handle)
        SDL_GL_SwapWindow(handle);
}

void Window::setMouseCaptured(bool captured) const
{
    if (handle)
        static_cast<void>(SDL_SetWindowRelativeMouseMode(handle, captured));
}

void Window::getPixelSize(int& pixelWidth, int& pixelHeight) const
{
    pixelWidth = 0;
    pixelHeight = 0;

    if (handle)
        static_cast<void>(
            SDL_GetWindowSizeInPixels(handle, &pixelWidth, &pixelHeight)
        );
}

SDL_Window* Window::getHandle() const
{
    return handle;
}

SDL_GLContext Window::getContext() const
{
    return context;
}

int Window::getWidth() const
{
    return width;
}

int Window::getHeight() const
{
    return height;
}

bool Window::isValid() const
{
    return handle != nullptr && context != nullptr;
}
