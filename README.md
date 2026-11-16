# RoadLife

RoadLife is an open-source vehicle simulator and game-engine project. The goal is to build a shared simulation experience for desktop and mobile platforms, with multiplayer planned as a later milestone.

The project is currently in an early engine-development stage. The existing renderer is a desktop OpenGL implementation; cross-platform rendering and mobile support are not complete yet.

## Current features

- SDL3 application and window layer
- C++20 codebase with CMake
- Free-fly developer camera with normalized movement, sprint and runtime speed/teleport controls
- OpenGL renderer with shader and mesh abstractions
- Shadow mapping with runtime enable/disable and night-time sampling bypass
- Procedural sky, sun and day/night cycle with quick time presets
- Distance fog with runtime density controls
- Toggleable ground grid and wireframe rendering
- Runtime cube spawning/removal (up to 2,000 scene objects)
- Developer console with command history, autocomplete, status output and runtime controls
- Cached shader uniform locations and CPU-side normal-matrix calculation to reduce per-frame/per-vertex overhead

## Developer console

Open the console with **F1** or **~**. Useful commands:

```text
help
status
camera
camera speed 12
camera teleport 0 3 8
camera reset
wireframe on
fog density 0.02
shadows off
grid off
spawn cube 5 0 -4 1.5
objects
objects clear
time noon
time sunset
```

Hold **Shift** while flying to move faster. Use **Esc** to release or recapture the mouse.

## Build on Arch Linux

Install the development dependencies:

```bash
sudo pacman -S --needed base-devel cmake ninja sdl3 sdl3_ttf glm libepoxy mesa
```

Configure and build:

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

Run on an X11 session:

```bash
SDL_VIDEODRIVER=x11 ./build/RoadLife
```

The current renderer requests an OpenGL 4.2 core context. A compatible graphics driver is required.

## Project structure

```text
Engine/
├── Camera/       # Developer camera
├── Console/      # In-game developer console
├── Core/         # Application lifecycle and timing
├── Platform/     # OS, window and input integration
├── Renderer/     # Graphics resources and rendering
└── World/        # Scene and world simulation
```

## Planned work

1. Separate application lifecycle, timing, windowing and input from the entry point.
2. Define a renderer-independent graphics interface.
3. Add rendering backends suitable for desktop and mobile targets.
4. Implement vehicle physics, controls, audio and world streaming.
5. Add asset pipeline, tools and content documentation.
6. Design host/client/dedicated-server multiplayer architecture.

## Target platforms

- Windows
- GNU/Linux
- macOS
- Android
- iOS

These are project targets, not a claim of current build support. Platform support will be added incrementally and documented as it becomes usable.

## License

RoadLife is distributed under the MIT License. Third-party libraries and assets remain subject to their own licenses.
