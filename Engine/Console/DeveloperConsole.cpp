
#include "DeveloperConsole.h"

#include "../World/Scene.h"
#include "../Camera/Camera.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <array>

static const char* consoleVertexShader = R"(
#version 420 core

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aUV;

out vec2 vUV;

void main()
{
    vUV = aUV;
    gl_Position = vec4(aPosition, 0.0, 1.0);
}
)";

static const char* consoleFragmentShader = R"(
#version 420 core

in vec2 vUV;

uniform sampler2D uTexture;

out vec4 FragColor;

void main()
{
    FragColor = texture(uTexture, vUV);
}
)";

// ------------------------------------------------------------
// COLORS
// ------------------------------------------------------------

static constexpr SDL_Color COLOR_TEXT       = {220, 229, 240, 255};
static constexpr SDL_Color COLOR_MUTED      = {119, 143, 169, 255};
static constexpr SDL_Color COLOR_BLUE       = {91, 174, 255, 255};
static constexpr SDL_Color COLOR_GREEN      = {105, 218, 151, 255};
static constexpr SDL_Color COLOR_YELLOW     = {245, 194, 91, 255};
static constexpr SDL_Color COLOR_RED        = {255, 105, 105, 255};
static constexpr SDL_Color COLOR_BACKGROUND = {7, 12, 20, 245};
static constexpr SDL_Color COLOR_PANEL      = {15, 24, 37, 255};
static constexpr SDL_Color COLOR_HEADER     = {19, 31, 47, 255};
static constexpr SDL_Color COLOR_BORDER     = {43, 61, 82, 255};
static constexpr SDL_Color COLOR_INPUT      = {12, 22, 35, 255};

// ------------------------------------------------------------
// CONSTRUCTOR
// ------------------------------------------------------------

DeveloperConsole::DeveloperConsole()
{
    if (!TTF_Init())
    {
        std::cerr << "TTF initialization failed: "
                  << SDL_GetError() << '\n';
        return;
    }

    const std::array<const char*, 3> fontPaths = {
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "/usr/share/fonts/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
    };

    for (const char* path : fontPaths)
    {
        font = TTF_OpenFont(path, 15.0f);

        if (font)
            break;
    }

    if (!font)
    {
        std::cerr << "Console font not found: "
                  << SDL_GetError() << '\n';
        return;
    }

    if (!createGraphics())
        return;

    addLine("[SUCCESS] RoadLife Developer Console initialized.");
    addLine("[INFO] Type 'help' to list available commands.");
    addLine("");

    valid = true;
}

// ------------------------------------------------------------
// DESTRUCTOR
// ------------------------------------------------------------

DeveloperConsole::~DeveloperConsole()
{
    if (texture)
        glDeleteTextures(1, &texture);

    if (vbo)
        glDeleteBuffers(1, &vbo);

    if (vao)
        glDeleteVertexArrays(1, &vao);

    if (shaderProgram)
        glDeleteProgram(shaderProgram);

    if (font)
        TTF_CloseFont(font);

    TTF_Quit();
}

// ------------------------------------------------------------
// BASIC
// ------------------------------------------------------------

bool DeveloperConsole::isValid() const
{
    return valid;
}

bool DeveloperConsole::isOpen() const
{
    return opened;
}

float DeveloperConsole::getTimeScale() const
{
    return timeScale;
}

bool DeveloperConsole::isTimePaused() const
{
    return timePaused;
}

void DeveloperConsole::toggle(SDL_Window* window)
{
    opened = !opened;

    if (opened)
        SDL_StartTextInput(window);
    else
        SDL_StopTextInput(window);
}

// ------------------------------------------------------------
// LOGGING
// ------------------------------------------------------------

void DeveloperConsole::addLine(const std::string& text)
{
    output.push_back(text);

    constexpr std::size_t maxLines = 250;

    if (outputScrollOffset > 0)
        ++outputScrollOffset;

    if (output.size() > maxLines)
    {
        output.erase(output.begin());
        outputScrollOffset = std::min(
            outputScrollOffset,
            static_cast<int>(output.size()) - 1
        );
    }
}

// ------------------------------------------------------------
// TIME FORMAT
// ------------------------------------------------------------

std::string DeveloperConsole::formatTime(float hours)
{
    hours = std::fmod(hours, 24.0f);

    if (hours < 0.0f)
        hours += 24.0f;

    int totalMinutes = static_cast<int>(hours * 60.0f);

    int h = totalMinutes / 60;
    int m = totalMinutes % 60;

    std::ostringstream stream;

    stream << std::setfill('0')
           << std::setw(2) << h
           << ':'
           << std::setw(2) << m;

    return stream.str();
}

// ------------------------------------------------------------
// COMMAND EXECUTION
// ------------------------------------------------------------

void DeveloperConsole::executeCommand(Scene& scene, Camera& camera, float fps)
{
    if (input.empty())
        return;

    const std::string command = input;

    addLine("> " + command);

    history.push_back(command);
    historyPosition = static_cast<int>(history.size());

    std::istringstream stream(command);

    std::string first;
    std::string second;
    std::string third;

    stream >> first >> second >> third;

    if (first == "help")
    {
        addLine("[INFO] Available commands:");
        addLine("  help                 - show commands");
        addLine("  clear                - clear console");
        addLine("  fps                  - show FPS");
        addLine("  time                 - show game time");
        addLine("  time set HH:MM       - set game time");
        addLine("  time set 18.5        - set time in hours");
        addLine("  time scale 10        - time speed multiplier");
        addLine("  time pause           - pause game clock");
        addLine("  time resume          - resume game clock");
        addLine("  wireframe [on|off]   - toggle or set wireframe rendering");
        addLine("  fog                  - show fog state");
        addLine("  fog on|off           - enable or disable distance fog");
        addLine("  fog density 0.01     - set fog density (0.0001..0.1)");
        addLine("  shadows [on|off]     - toggle shadow mapping");
        addLine("  grid [on|off]        - toggle ground grid");
        addLine("  camera               - show camera position and speed");
        addLine("  camera speed 10      - set movement speed (0.1..100)");
        addLine("  camera fov 90        - set field of view (30..110 degrees)");
        addLine("  SHIFT                - sprint while flying around");
        addLine("  camera reset         - reset camera position and rotation");
        addLine("  camera teleport x y z - move camera to coordinates");
        addLine("  objects              - show scene object count");
        addLine("  spawn cube x y z [s] - spawn a cube (default size 1)");
        addLine("  objects clear        - remove all scene cubes");
        addLine("  status               - show current engine/world status");
        addLine("  time dawn|noon|sunset|midnight - quick time presets");
        addLine("  history              - show recent commands");
        addLine("  echo <text>          - print a message to the console");
        addLine("  version              - show engine version and graphics API");
        addLine("  scroll top|bottom    - jump through console output");
    }
    else if (first == "clear")
    {
        output.clear();
        outputScrollOffset = 0;
        addLine("[SUCCESS] Console output cleared.");
    }
    else if (first == "history")
    {
        addLine("[INFO] Recent commands:");
        const std::size_t end = history.size() > 1 ? history.size() - 1 : 0;
        const std::size_t begin = end > 12 ? end - 12 : 0;
        if (begin == end)
            addLine("  (no previous commands)");
        for (std::size_t i = begin; i < end; ++i)
            addLine("  " + history[i]);
    }
    else if (first == "echo")
    {
        const std::size_t separator = command.find(' ');
        addLine(separator == std::string::npos
            ? "[INFO] "
            : command.substr(separator + 1));
    }
    else if (first == "version")
    {
        addLine("[INFO] RoadLife Engine 0.4 | OpenGL 4.2 Core");
        addLine("[INFO] SDL3 | C++20 | Developer build");
    }
    else if (first == "scroll")
    {
        if (second == "top")
            outputScrollOffset = std::max(0, static_cast<int>(output.size()) - 1);
        else if (second == "bottom")
            outputScrollOffset = 0;
        else
            addLine("[WARNING] Usage: scroll top|bottom");
    }
    else if (first == "fps")
    {
        std::ostringstream result;

        result << "[INFO] FPS: "
               << std::fixed
               << std::setprecision(1)
               << fps;

        addLine(result.str());
    }
    else if (first == "wireframe")
    {
        if (second == "on" || second == "off")
            scene.setWireframe(second == "on");
        else if (second.empty())
            scene.setWireframe(!scene.isWireframe());
        else
            addLine("[WARNING] Usage: wireframe [on|off]");

        if (second.empty() || second == "on" || second == "off")
            addLine(scene.isWireframe()
                ? "[SUCCESS] Wireframe rendering enabled."
                : "[SUCCESS] Solid rendering enabled.");
    }
    else if (first == "fog")
    {
        if (second == "density")
        {
            try
            {
                std::size_t parsed = 0;
                const float value = std::stof(third, &parsed);
                if (parsed != third.size() || !std::isfinite(value) ||
                    value < 0.0001f || value > 0.1f)
                    throw std::out_of_range("Invalid fog density");
                scene.setFogDensity(value);
                addLine("[SUCCESS] Fog density: " + std::to_string(scene.getFogDensity()));
            }
            catch (...)
            {
                addLine("[ERROR] Usage: fog density 0.0001..0.1");
            }
        }
        else if (second.empty())
        {
            std::ostringstream result;
            result << "[INFO] Distance fog: "
                   << (scene.isFogEnabled() ? "enabled" : "disabled")
                   << " | density: " << std::fixed << std::setprecision(4)
                   << scene.getFogDensity();
            addLine(result.str());
        }
        else if (second == "on" || second == "off")
        {
            scene.setFogEnabled(second == "on");
            addLine(second == "on"
                ? "[SUCCESS] Distance fog enabled."
                : "[SUCCESS] Distance fog disabled.");
        }
        else
        {
            addLine("[WARNING] Usage: fog [on|off|density value]");
        }
    }
    else if (first == "shadows")
    {
        if (second.empty())
        {
            addLine(scene.areShadowsEnabled() ? "[INFO] Shadows: enabled." : "[INFO] Shadows: disabled.");
        }
        else if (second == "on" || second == "off")
        {
            scene.setShadowsEnabled(second == "on");
            addLine(second == "on" ? "[SUCCESS] Shadows enabled." : "[SUCCESS] Shadows disabled.");
        }
        else addLine("[WARNING] Usage: shadows [on|off]");
    }
    else if (first == "grid")
    {
        if (second.empty())
            addLine(scene.isGridEnabled() ? "[INFO] Grid: enabled." : "[INFO] Grid: disabled.");
        else if (second == "on" || second == "off")
        {
            scene.setGridEnabled(second == "on");
            addLine(second == "on" ? "[SUCCESS] Grid enabled." : "[SUCCESS] Grid disabled.");
        }
        else addLine("[WARNING] Usage: grid [on|off]");
    }
    else if (first == "camera")
    {
        if (second.empty())
        {
            const glm::vec3 p = camera.getPosition();
            std::ostringstream result;
            result << std::fixed << std::setprecision(2)
                   << "[INFO] Position: " << p.x << ", " << p.y << ", " << p.z
                   << " | Speed: " << camera.getMovementSpeed()
                   << " | FOV: " << camera.getFieldOfView();
            addLine(result.str());
        }
        else if (second == "speed")
        {
            try
            {
                std::size_t parsed = 0;
                const float value = std::stof(third, &parsed);
                if (parsed != third.size() || !std::isfinite(value) || value < 0.1f || value > 100.0f)
                    throw std::out_of_range("Invalid camera speed");
                camera.setMovementSpeed(value);
                addLine("[SUCCESS] Camera speed set to " + std::to_string(camera.getMovementSpeed()));
            }
            catch (...) { addLine("[ERROR] Usage: camera speed 0.1..100"); }
        }
        else if (second == "fov")
        {
            try
            {
                std::size_t parsed = 0;
                const float value = std::stof(third, &parsed);
                if (parsed != third.size() || !std::isfinite(value) ||
                    value < 30.0f || value > 110.0f)
                    throw std::out_of_range("Invalid field of view");
                camera.setFieldOfView(value);
                addLine("[SUCCESS] Camera FOV set to " +
                        std::to_string(camera.getFieldOfView()) + " degrees.");
            }
            catch (...)
            {
                addLine("[ERROR] Usage: camera fov 30..110");
            }
        }
        else if (second == "reset")
        {
            camera.reset();
            addLine("[SUCCESS] Camera reset.");
        }
        else if (second == "teleport")
        {
            std::string sx = third, sy, sz;
            stream >> sy >> sz;
            try
            {
                std::size_t px = 0, py = 0, pz = 0;
                const float x = std::stof(sx, &px);
                const float y = std::stof(sy, &py);
                const float z = std::stof(sz, &pz);
                if (px != sx.size() || py != sy.size() || pz != sz.size() ||
                    !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
                    throw std::out_of_range("Invalid coordinates");
                camera.setPosition(glm::vec3(x, y, z));
                addLine("[SUCCESS] Camera teleported.");
            }
            catch (...) { addLine("[ERROR] Usage: camera teleport x y z"); }
        }
        else addLine("[WARNING] Usage: camera [speed|reset|teleport]");
    }
    else if (first == "status")
    {
        const glm::vec3 p = camera.getPosition();
        std::ostringstream result;
        result << std::fixed << std::setprecision(1)
               << "[STATUS] FPS " << fps
               << " | time " << formatTime(scene.getTimeOfDay())
               << " | objects " << scene.getObjectCount()
               << " | camera " << std::setprecision(2)
               << p.x << "," << p.y << "," << p.z;
        addLine(result.str());
        addLine(std::string("[STATUS] Shadows ") + (scene.areShadowsEnabled() ? "ON" : "OFF") +
                " | fog " + (scene.isFogEnabled() ? "ON" : "OFF") +
                " | grid " + (scene.isGridEnabled() ? "ON" : "OFF") +
                " | wireframe " + (scene.isWireframe() ? "ON" : "OFF"));
    }
    else if (first == "objects")
    {
        if (second == "clear")
        {
            scene.clearObjects();
            addLine("[SUCCESS] All scene cubes removed.");
        }
        else addLine("[INFO] Scene cubes: " + std::to_string(scene.getObjectCount()));
    }
    else if (first == "spawn")
    {
        std::string sx = third, sy, sz, ss;
        stream >> sy >> sz >> ss;
        if (second != "cube")
        {
            addLine("[WARNING] Usage: spawn cube x y z [scale]");
        }
        else
        {
            try
            {
                std::size_t px = 0, py = 0, pz = 0, ps = 0;
                const float x = std::stof(sx, &px);
                const float y = std::stof(sy, &py);
                const float z = std::stof(sz, &pz);
                const float scale = ss.empty() ? 1.0f : std::stof(ss, &ps);
                if (px != sx.size() || py != sy.size() || pz != sz.size() ||
                    (!ss.empty() && ps != ss.size()) ||
                    !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) || !std::isfinite(scale) ||
                    std::abs(x) > 10000.0f || std::abs(y) > 10000.0f || std::abs(z) > 10000.0f || scale < 0.1f || scale > 20.0f)
                    throw std::out_of_range("Invalid cube parameters");
                if (scene.spawnCube(glm::vec3(x, y, z), scale))
                    addLine("[SUCCESS] Cube spawned. Total: " + std::to_string(scene.getObjectCount()));
                else
                    addLine("[WARNING] Object limit reached (2000). Use 'objects clear'.");
            }
            catch (...) { addLine("[ERROR] Usage: spawn cube x y z [scale]"); }
        }
    }
    else if (first == "time")
    {
        if (second.empty())
        {
            addLine(
                "[INFO] Game time: " +
                formatTime(scene.getTimeOfDay())
            );

            addLine(
                "[INFO] Time scale: " +
                std::to_string(timeScale) + "x"
            );

            addLine(
                timePaused
                    ? "[WARNING] Clock is paused."
                    : "[SUCCESS] Clock is running."
            );
        }
        else if (second == "set")
        {
            if (third.empty())
            {
                addLine("[WARNING] Usage: time set HH:MM");
            }
            else
            {
                try
                {
                    float hours = 0.0f;

                    const std::size_t separator =
                        third.find(':');

                    if (separator != std::string::npos)
                    {
                        int h = std::stoi(
                            third.substr(0, separator)
                        );

                        int m = std::stoi(
                            third.substr(separator + 1)
                        );

                        if (h < 0 || h > 23 || m < 0 || m > 59)
                            throw std::out_of_range("Invalid time");

                        hours = static_cast<float>(h) +
                                static_cast<float>(m) / 60.0f;
                    }
                    else
                    {
                        std::size_t parsed = 0;

                        hours = std::stof(third, &parsed);

                        if (parsed != third.size() ||
                            !std::isfinite(hours) ||
                            hours < 0.0f ||
                            hours >= 24.0f)
                        {
                            throw std::out_of_range("Invalid time");
                        }
                    }

                    scene.setTimeOfDay(hours);

                    addLine(
                        "[SUCCESS] Time changed to " +
                        formatTime(scene.getTimeOfDay())
                    );
                }
                catch (...)
                {
                    addLine("[ERROR] Invalid time. Example: time set 18:30");
                }
            }
        }
        else if (second == "scale")
        {
            if (third.empty())
            {
                addLine("[WARNING] Usage: time scale 10");
            }
            else
            {
                try
                {
                    std::size_t parsed = 0;
                    float value = std::stof(third, &parsed);

                    if (parsed != third.size() ||
                        !std::isfinite(value) ||
                        value < 0.0f ||
                        value > 1000.0f)
                    {
                        throw std::out_of_range("Invalid scale");
                    }

                    timeScale = value;

                    std::ostringstream result;
                    result << "[SUCCESS] Time scale changed to "
                           << std::fixed << std::setprecision(1)
                           << timeScale << "x";

                    addLine(result.str());
                }
                catch (...)
                {
                    addLine("[ERROR] Scale must be between 0 and 1000.");
                }
            }
        }
        else if (second == "pause")
        {
            timePaused = true;
            addLine("[WARNING] Game clock paused.");
        }
        else if (second == "resume")
        {
            timePaused = false;
            addLine("[SUCCESS] Game clock resumed.");
        }
        else if (second == "dawn" || second == "sunrise" ||
                 second == "noon" || second == "sunset" ||
                 second == "dusk" || second == "midnight")
        {
            float preset = 6.0f;
            if (second == "noon") preset = 12.0f;
            else if (second == "sunset" || second == "dusk") preset = 18.0f;
            else if (second == "midnight") preset = 0.0f;
            scene.setTimeOfDay(preset);
            addLine("[SUCCESS] Time preset: " + second + " (" + formatTime(preset) + ").");
        }
        else
        {
            addLine("[ERROR] Unknown time command. Type 'help'.");
        }
    }
    else
    {
        addLine("[ERROR] Unknown command: " + first);
        addLine("[INFO] Type 'help' for available commands.");
    }

    input.clear();
}

// ------------------------------------------------------------
// AUTOCOMPLETE
// ------------------------------------------------------------

void DeveloperConsole::autocomplete()
{
    const std::vector<std::string> commands = {
        "help", "clear", "fps", "time", "time set", "time scale",
        "time pause", "time resume", "wireframe", "wireframe on", "wireframe off",
        "fog", "fog density",
        "shadows", "grid", "camera", "camera speed", "camera fov", "camera reset",
        "camera teleport", "objects", "objects clear", "spawn cube",
        "status", "history", "echo", "version", "scroll top", "scroll bottom",
        "time dawn", "time sunrise", "time noon",
        "time sunset", "time dusk", "time midnight"
    };

    std::vector<std::string> matches;

    for (const auto& command : commands)
    {
        if (command.rfind(input, 0) == 0)
            matches.push_back(command);
    }

    std::sort(matches.begin(), matches.end());
    matches.erase(
        std::unique(matches.begin(), matches.end()),
        matches.end()
    );

    if (matches.size() == 1)
    {
        input = matches.front();
        return;
    }

    if (matches.size() > 1)
    {
        std::string result = "[INFO] Matches:";

        for (const auto& match : matches)
            result += "  " + match;

        addLine(result);
    }
}

// ------------------------------------------------------------
// INPUT EVENTS
// ------------------------------------------------------------

void DeveloperConsole::handleEvent(
    const SDL_Event& event,
    Scene& scene,
    Camera& camera,
    float fps
)
{
    if (!opened)
        return;

    if (event.type == SDL_EVENT_MOUSE_WHEEL)
    {
        int direction = event.wheel.y > 0.0f ? 1 :
                        event.wheel.y < 0.0f ? -1 : 0;
        if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
            direction = -direction;
        outputScrollOffset = std::clamp(
            outputScrollOffset + direction * 3,
            0,
            std::max(0, static_cast<int>(output.size()) - 1)
        );
        return;
    }

    if (event.type == SDL_EVENT_TEXT_INPUT)
    {
        input += event.text.text;
        return;
    }

    if (event.type != SDL_EVENT_KEY_DOWN || event.key.repeat)
        return;

    switch (event.key.key)
    {
        case SDLK_BACKSPACE:
            if (!input.empty())
            {
                std::size_t start = input.size() - 1;
                while (start > 0 &&
                       (static_cast<unsigned char>(input[start]) & 0xC0) == 0x80)
                    --start;
                input.erase(start);
            }
            break;

        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            executeCommand(scene, camera, fps);
            break;

        case SDLK_UP:
            if (!history.empty() && historyPosition > 0)
            {
                --historyPosition;
                input = history[historyPosition];
            }
            break;

        case SDLK_DOWN:
            if (historyPosition < static_cast<int>(history.size()) - 1)
            {
                ++historyPosition;
                input = history[historyPosition];
            }
            else
            {
                historyPosition = static_cast<int>(history.size());
                input.clear();
            }
            break;

        case SDLK_TAB:
            autocomplete();
            break;

        case SDLK_PAGEUP:
            outputScrollOffset = std::min(
                std::max(0, static_cast<int>(output.size()) - 1),
                outputScrollOffset + 8
            );
            break;

        case SDLK_PAGEDOWN:
            outputScrollOffset = std::max(0, outputScrollOffset - 8);
            break;

        case SDLK_HOME:
            outputScrollOffset = std::max(0, static_cast<int>(output.size()) - 1);
            break;

        case SDLK_END:
            outputScrollOffset = 0;
            break;

        case SDLK_1:
            activeTab = Tab::Console;
            break;

        case SDLK_2:
            activeTab = Tab::Logs;
            break;

        case SDLK_3:
            activeTab = Tab::Performance;
            break;

        case SDLK_4:
            activeTab = Tab::World;
            break;

        default:
            break;
    }
}

// ------------------------------------------------------------
// OPENGL SHADER
// ------------------------------------------------------------

GLuint DeveloperConsole::compileShader(
    GLenum type,
    const char* source
)
{
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char log[1024]{};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);

        std::cerr << "Console shader error: "
                  << log << '\n';

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

// ------------------------------------------------------------
// GRAPHICS
// ------------------------------------------------------------

bool DeveloperConsole::createGraphics()
{
    GLuint vertex = compileShader(
        GL_VERTEX_SHADER,
        consoleVertexShader
    );

    GLuint fragment = compileShader(
        GL_FRAGMENT_SHADER,
        consoleFragmentShader
    );

    if (!vertex || !fragment)
    {
        if (vertex)
            glDeleteShader(vertex);

        if (fragment)
            glDeleteShader(fragment);

        return false;
    }

    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint linked = GL_FALSE;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        std::cerr << "Console shader linking failed.\n";
        return false;
    }

    const float vertices[] = {
        -1.0f, -1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 0.0f,

        -1.0f, -1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 0.0f
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE,
        4 * sizeof(float),
        nullptr
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE,
        4 * sizeof(float),
        reinterpret_cast<void*>(2 * sizeof(float))
    );

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    return vao != 0 && vbo != 0 && texture != 0;
}

// ------------------------------------------------------------
// DRAW HELPERS
// ------------------------------------------------------------

void DeveloperConsole::fillRect(
    SDL_Surface* target,
    int x,
    int y,
    int width,
    int height,
    SDL_Color color
)
{
    if (width <= 0 || height <= 0)
        return;

    SDL_Rect rect{x, y, width, height};

    SDL_FillSurfaceRect(
        target,
        &rect,
        SDL_MapSurfaceRGBA(
            target,
            color.r,
            color.g,
            color.b,
            color.a
        )
    );
}

void DeveloperConsole::drawText(
    SDL_Surface* target,
    const std::string& text,
    int x,
    int y,
    SDL_Color color
)
{
    if (text.empty())
        return;

    SDL_Surface* rendered = TTF_RenderText_Blended(
        font,
        text.c_str(),
        text.size(),
        color
    );

    if (!rendered)
        return;

    SDL_Rect destination{
        x,
        y,
        rendered->w,
        rendered->h
    };

    SDL_BlitSurface(
        rendered,
        nullptr,
        target,
        &destination
    );

    SDL_DestroySurface(rendered);
}

// ------------------------------------------------------------
// CONSOLE TAB
// ------------------------------------------------------------

void DeveloperConsole::renderConsoleTab(
    SDL_Surface* surface,
    int x,
    int y,
    int width,
    int height
)
{
    drawText(surface, "ENGINE OUTPUT", x, y, COLOR_MUTED);

    y += 26;

    const int lineHeight = 21;
    const int availableLines = std::max(1, (height - 55) / lineHeight);
    const int totalLines = static_cast<int>(output.size());
    const int maxOffset = std::max(0, totalLines - availableLines);
    outputScrollOffset = std::clamp(outputScrollOffset, 0, maxOffset);

    const int endLine = std::max(0, totalLines - outputScrollOffset);
    const int firstLine = std::max(0, endLine - availableLines);

    for (int i = firstLine; i < endLine; ++i)
    {
        const std::string& line = output[static_cast<std::size_t>(i)];
        SDL_Color color = COLOR_TEXT;

        if (line.find("[SUCCESS]") != std::string::npos)
            color = COLOR_GREEN;
        else if (line.find("[WARNING]") != std::string::npos)
            color = COLOR_YELLOW;
        else if (line.find("[ERROR]") != std::string::npos)
            color = COLOR_RED;
        else if (line.find("[INFO]") != std::string::npos)
            color = COLOR_BLUE;
        else if (!line.empty() && line[0] == '>')
            color = COLOR_GREEN;

        drawText(surface, line, x, y, color);
        y += lineHeight;
    }

    if (totalLines > availableLines)
    {
        const int trackX = x + width - 5;
        const int trackY = y - (endLine - firstLine) * lineHeight;
        const int trackHeight = availableLines * lineHeight;
        const int thumbHeight = std::max(
            18, trackHeight * availableLines / totalLines
        );
        const int thumbTravel = std::max(0, trackHeight - thumbHeight);
        const int thumbY = trackY + (maxOffset > 0
            ? thumbTravel * (maxOffset - outputScrollOffset) / maxOffset
            : 0);
        fillRect(surface, trackX, trackY, 3, trackHeight, COLOR_BORDER);
        fillRect(surface, trackX, thumbY, 3, thumbHeight, COLOR_BLUE);
    }
}
// ------------------------------------------------------------
// LOGS TAB
// ------------------------------------------------------------

void DeveloperConsole::renderLogsTab(
    SDL_Surface* surface,
    int x,
    int y,
    int width,
    int height
)
{
    drawText(surface, "EVENT LOG", x, y, COLOR_MUTED);
    drawText(
        surface,
        std::to_string(output.size()) + " entries",
        x + width - 130,
        y,
        COLOR_MUTED
    );

    y += 26;

    const int lineHeight = 21;
    const int availableLines = std::max(1, (height - 55) / lineHeight);
    const int totalLines = static_cast<int>(output.size());
    const int maxOffset = std::max(0, totalLines - availableLines);
    outputScrollOffset = std::clamp(outputScrollOffset, 0, maxOffset);

    const int endLine = std::max(0, totalLines - outputScrollOffset);
    const int firstLine = std::max(0, endLine - availableLines);

    for (int i = firstLine; i < endLine; ++i)
    {
        const std::string& line = output[static_cast<std::size_t>(i)];
        SDL_Color color = COLOR_TEXT;

        if (line.find("[SUCCESS]") != std::string::npos)
            color = COLOR_GREEN;
        else if (line.find("[WARNING]") != std::string::npos)
            color = COLOR_YELLOW;
        else if (line.find("[ERROR]") != std::string::npos)
            color = COLOR_RED;
        else if (line.find("[INFO]") != std::string::npos)
            color = COLOR_BLUE;

        drawText(surface, line, x, y, color);
        y += lineHeight;
    }

    if (totalLines > availableLines)
    {
        const int trackX = x + width - 5;
        const int trackY = y - (endLine - firstLine) * lineHeight;
        const int trackHeight = availableLines * lineHeight;
        const int thumbHeight = std::max(
            18, trackHeight * availableLines / totalLines
        );
        const int thumbTravel = std::max(0, trackHeight - thumbHeight);
        const int thumbY = trackY + (maxOffset > 0
            ? thumbTravel * (maxOffset - outputScrollOffset) / maxOffset
            : 0);
        fillRect(surface, trackX, trackY, 3, trackHeight, COLOR_BORDER);
        fillRect(surface, trackX, thumbY, 3, thumbHeight, COLOR_BLUE);
    }
}

// ------------------------------------------------------------
// PERFORMANCE TAB
// ------------------------------------------------------------

void DeveloperConsole::renderPerformanceTab(
    SDL_Surface* surface,
    int x,
    int y,
    int width,
    float fps
)
{
    drawText(surface, "RENDER PERFORMANCE", x, y, COLOR_MUTED);

    y += 42;

    const float frameTime = fps > 0.0f ? 1000.0f / fps : 0.0f;

    std::ostringstream fpsText;
    fpsText << std::fixed << std::setprecision(1) << fps;

    std::ostringstream frameText;
    frameText << std::fixed << std::setprecision(2) << frameTime << " ms";

    drawText(surface, "FRAMES PER SECOND", x, y, COLOR_MUTED);
    drawText(surface, fpsText.str(), x, y + 27, COLOR_BLUE);

    drawText(surface, "FRAME TIME", x + 250, y, COLOR_MUTED);
    drawText(surface, frameText.str(), x + 250, y + 27, COLOR_GREEN);

    y += 70;

    if (!fpsSamples.empty())
    {
        const auto [minimum, maximum] = std::minmax_element(
            fpsSamples.begin(), fpsSamples.end()
        );
        float average = 0.0f;
        for (const float sample : fpsSamples)
            average += sample;
        average /= static_cast<float>(fpsSamples.size());

        std::ostringstream metrics;
        metrics << std::fixed << std::setprecision(1)
                << "FPS min/avg/max: " << *minimum << " / "
                << average << " / " << *maximum;
        drawText(surface, metrics.str(), x, y, COLOR_TEXT);

        const int graphY = y + 28;
        const int graphHeight = 42;
        const int graphWidth = std::max(1, std::min(width - 8, 720));
        fillRect(surface, x, graphY, graphWidth, graphHeight, COLOR_INPUT);
        const float scale = std::max(1.0f, *maximum);
        const int sampleCount = static_cast<int>(fpsSamples.size());
        for (int i = 0; i < sampleCount; ++i)
        {
            const int barX = x + i * graphWidth / sampleCount;
            const int nextX = x + (i + 1) * graphWidth / sampleCount;
            const int barWidth = std::max(1, nextX - barX - 1);
            const int barHeight = static_cast<int>(
                glm::clamp(fpsSamples[static_cast<std::size_t>(i)] / scale,
                           0.0f, 1.0f) * static_cast<float>(graphHeight - 4)
            );
            fillRect(surface, barX, graphY + graphHeight - barHeight - 2,
                     barWidth, std::max(1, barHeight), COLOR_BLUE);
        }
        y += 70;
    }

    drawText(surface, "SIMULATION", x, y, COLOR_MUTED);
    drawText(
        surface,
        "Time scale: " + std::to_string(timeScale) + "x",
        x,
        y + 27,
        COLOR_TEXT
    );

    drawText(
        surface,
        timePaused ? "Clock: PAUSED" : "Clock: RUNNING",
        x + 250,
        y + 27,
        timePaused ? COLOR_YELLOW : COLOR_GREEN
    );

}

// ------------------------------------------------------------
// WORLD TAB
// ------------------------------------------------------------

void DeveloperConsole::renderWorldTab(
    SDL_Surface* surface,
    int x,
    int y,
    const Scene& scene
)
{
    drawText(surface, "WORLD SIMULATION", x, y, COLOR_MUTED);

    y += 42;

    drawText(surface, "GAME TIME", x, y, COLOR_MUTED);
    drawText(
        surface,
        formatTime(scene.getTimeOfDay()),
        x,
        y + 27,
        COLOR_BLUE
    );

    drawText(surface, "TIME SCALE", x + 250, y, COLOR_MUTED);
    drawText(
        surface,
        std::to_string(timeScale) + "x",
        x + 250,
        y + 27,
        COLOR_GREEN
    );

    y += 100;

    drawText(surface, "CLOCK STATUS", x, y, COLOR_MUTED);
    drawText(
        surface,
        timePaused ? "PAUSED" : "RUNNING",
        x,
        y + 27,
        timePaused ? COLOR_YELLOW : COLOR_GREEN
    );

    y += 85;

    drawText(
        surface,
        "Use 'time set', 'time scale', 'time pause' and 'time resume'.",
        x,
        y,
        COLOR_MUTED
    );
}
// ------------------------------------------------------------
// MAIN RENDER
// ------------------------------------------------------------

void DeveloperConsole::render(
    int width,
    int height,
    const Scene& scene,
    float fps
)
{
    if (!opened || !valid || width <= 0 || height <= 0)
        return;

    if (fps > 0.0f && std::isfinite(fps))
    {
        fpsSamples.push_back(fps);
        constexpr std::size_t maxSamples = 120;
        if (fpsSamples.size() > maxSamples)
            fpsSamples.erase(fpsSamples.begin());
    }

    SDL_Surface* surface = SDL_CreateSurface(
        width,
        height,
        SDL_PIXELFORMAT_RGBA32
    );

    if (!surface)
        return;

    SDL_FillSurfaceRect(
        surface,
        nullptr,
        SDL_MapSurfaceRGBA(surface, 0, 0, 0, 0)
    );

    // Main panel.
    const int panelHeight = std::min(410, height);
    const int panelY = height - panelHeight;

    fillRect(
        surface,
        0,
        panelY,
        width,
        panelHeight,
        COLOR_BACKGROUND
    );

    // Top header.
    fillRect(
        surface,
        0,
        panelY,
        width,
        58,
        COLOR_HEADER
    );

    fillRect(
        surface,
        0,
        panelY + 57,
        width,
        1,
        COLOR_BORDER
    );

    // Logo block.
    fillRect(
        surface,
        16,
        panelY + 13,
        32,
        32,
        {34, 91, 145, 255}
    );

    drawText(
        surface,
        "RL",
        22,
        panelY + 20,
        {255, 255, 255, 255}
    );

    drawText(
        surface,
        "ROADLIFE",
        60,
        panelY + 12,
        COLOR_TEXT
    );

    drawText(
        surface,
        "DEVELOPER CONSOLE",
        60,
        panelY + 32,
        COLOR_MUTED
    );

    // Online status.
    fillRect(
        surface,
        width - 112,
        panelY + 20,
        8,
        8,
        COLOR_GREEN
    );

    drawText(
        surface,
        "ONLINE",
        width - 96,
        panelY + 15,
        COLOR_GREEN
    );

    // Tabs.
    const int tabY = panelY + 58;

    fillRect(
        surface,
        0,
        tabY,
        width,
        39,
        COLOR_PANEL
    );

    const char* tabNames[] = {
        "1  CONSOLE",
        "2  LOGS",
        "3  PERFORMANCE",
        "4  WORLD"
    };

    const Tab tabs[] = {
        Tab::Console,
        Tab::Logs,
        Tab::Performance,
        Tab::World
    };

    int tabX = 18;

    for (int i = 0; i < 4; ++i)
    {
        const bool selected = activeTab == tabs[i];

        const int tabWidth = (i == 2) ? 150 : 112;

        if (selected)
        {
            fillRect(
                surface,
                tabX,
                tabY + 3,
                tabWidth - 8,
                34,
                {28, 48, 70, 255}
            );

            fillRect(
                surface,
                tabX,
                tabY + 35,
                tabWidth - 8,
                2,
                COLOR_BLUE
            );
        }

        drawText(
            surface,
            tabNames[i],
            tabX + 9,
            tabY + 12,
            selected ? COLOR_BLUE : COLOR_MUTED
        );

        tabX += tabWidth;
    }

    // Content area.
    const int contentX = 20;
    const int contentY = tabY + 54;
    const int contentWidth = std::max(1, width - 40);
    const int contentHeight = std::max(1, panelHeight - 160);

    switch (activeTab)
{
    case Tab::Console:
        renderConsoleTab(
            surface,
            contentX,
            contentY,
            contentWidth,
            contentHeight
        );
        break;

    case Tab::Logs:
        renderLogsTab(
            surface,
            contentX,
            contentY,
            contentWidth,
            contentHeight
        );
        break;

    case Tab::Performance:
        renderPerformanceTab(
            surface,
            contentX,
            contentY,
            contentWidth,
            fps
        );
        break;

    case Tab::World:
        renderWorldTab(
            surface,
            contentX,
            contentY,
            scene
        );
        break;
}

    // Input field.
    const int inputHeight = 42;
    const int inputY = height - inputHeight - 31;

    fillRect(
        surface,
        14,
        inputY,
        width - 28,
        inputHeight,
        COLOR_INPUT
    );

    fillRect(
        surface,
        14,
        inputY,
        3,
        inputHeight,
        COLOR_BLUE
    );

    drawText(
        surface,
        "RL >",
        27,
        inputY + 12,
        COLOR_BLUE
    );

    drawText(
        surface,
        input + "_",
        83,
        inputY + 12,
        COLOR_TEXT
    );

    // Bottom status bar.
    fillRect(
        surface,
        0,
        height - 27,
        width,
        27,
        COLOR_HEADER
    );

    drawText(
        surface,
        "ROADLIFE ENGINE 0.4",
        16,
        height - 20,
        COLOR_MUTED
    );

    drawText(
        surface,
        "OPENGL 4.2",
        width / 2 - 40,
        height - 20,
        COLOR_MUTED
    );

    drawText(
        surface,
        "F1 CLOSE  |  TAB COMPLETE  |  WHEEL/PAGE SCROLL",
        width - 390,
        height - 20,
        COLOR_MUTED
    );

    // Upload surface to OpenGL texture.
    glBindTexture(GL_TEXTURE_2D, texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        surface->pixels
    );

    SDL_DestroySurface(surface);

    // Save OpenGL state.
    GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);

    GLint previousProgram = 0;
    GLint previousVAO = 0;
    GLint previousActiveTexture = 0;
    GLint previousTexture = 0;
    GLint previousBlendSrc = 0;
    GLint previousBlendDst = 0;

    glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVAO);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &previousActiveTexture);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &previousBlendSrc);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &previousBlendDst);

    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);

    // Draw overlay.
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(shaderProgram);

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(vao);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Restore OpenGL state.
    glBindVertexArray(static_cast<GLuint>(previousVAO));
    glUseProgram(static_cast<GLuint>(previousProgram));

    glBindTexture(
        GL_TEXTURE_2D,
        static_cast<GLuint>(previousTexture)
    );

    glBlendFunc(
        GL_SRC_ALPHA,
        static_cast<GLenum>(previousBlendDst)
    );

    glBlendFuncSeparate(
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA,
        static_cast<GLenum>(previousBlendSrc),
        static_cast<GLenum>(previousBlendDst)
    );

    glActiveTexture(static_cast<GLenum>(previousActiveTexture));

    if (!depthWasEnabled)
        glDisable(GL_DEPTH_TEST);

    if (!blendWasEnabled)
        glDisable(GL_BLEND);
}

