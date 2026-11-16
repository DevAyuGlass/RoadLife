#pragma once

#include <SDL3/SDL.h>

class Window
{
public:
    Window() = default;
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool create(const char* title, int width, int height);
    void destroy();

    void swapBuffers() const;
    void setMouseCaptured(bool captured) const;
    void getPixelSize(int& width, int& height) const;

    SDL_Window* getHandle() const;
    SDL_GLContext getContext() const;

    int getWidth() const;
    int getHeight() const;
    bool isValid() const;

private:
    SDL_Window* handle = nullptr;
    SDL_GLContext context = nullptr;

    int width = 0;
    int height = 0;
};
