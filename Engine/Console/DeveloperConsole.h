#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <epoxy/gl.h>

#include <string>
#include <vector>

class Scene;
class Camera;

class DeveloperConsole
{
public:
    DeveloperConsole();
    ~DeveloperConsole();

    DeveloperConsole(const DeveloperConsole&) = delete;
    DeveloperConsole& operator=(const DeveloperConsole&) = delete;

    bool isValid() const;
    bool isOpen() const;

    void toggle(SDL_Window* window);
    void handleEvent(const SDL_Event& event, Scene& scene, Camera& camera, float fps);
    void render(int width, int height, const Scene& scene, float fps);

    float getTimeScale() const;
    bool isTimePaused() const;

private:
    enum class Tab
    {
        Console,
        Logs,
        Performance,
        World
    };

    TTF_Font* font = nullptr;

    GLuint shaderProgram = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint texture = 0;

    bool valid = false;
    bool opened = false;
    bool timePaused = false;

    float timeScale = 1.0f;

    Tab activeTab = Tab::Console;

    std::string input;
    std::vector<std::string> output;
    std::vector<std::string> history;

    int historyPosition = 0;

    void addLine(const std::string& text);
    void executeCommand(Scene& scene, Camera& camera, float fps);
    void autocomplete();

    bool createGraphics();
    GLuint compileShader(GLenum type, const char* source);

    void drawText(
        SDL_Surface* target,
        const std::string& text,
        int x,
        int y,
        SDL_Color color
    );

    void fillRect(
        SDL_Surface* target,
        int x,
        int y,
        int width,
        int height,
        SDL_Color color
    );

    void renderConsoleTab(SDL_Surface* surface, int x, int y, int height);
    void renderLogsTab(SDL_Surface* surface, int x, int y, int width, int height);
    void renderPerformanceTab(SDL_Surface* surface, int x, int y, float fps);
    void renderWorldTab(SDL_Surface* surface, int x, int y, const Scene& scene);

    static std::string formatTime(float hours);
};
