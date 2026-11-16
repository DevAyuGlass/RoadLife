cmake_minimum_required(VERSION 3.25)

project(RoadLife VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(EPOXY REQUIRED IMPORTED_TARGET epoxy)

add_executable(RoadLife
    Engine/Core/main.cpp
    Engine/Core/Application.cpp
    Engine/Core/Time.cpp
    Engine/Platform/Window/Window.cpp
    Engine/Camera/Camera.cpp
    Engine/Input/InputManager.cpp
    Engine/Renderer/OpenGL/OpenGLRenderer.cpp
    Engine/Renderer/Shader.cpp
    Engine/Renderer/Mesh.cpp
    Engine/Renderer/ShadowMap.cpp
    Engine/World/Scene.cpp
    Engine/Console/DeveloperConsole.cpp
)

target_include_directories(RoadLife PRIVATE Engine)

target_link_libraries(RoadLife PRIVATE
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    OpenGL::GL
    glm::glm
    PkgConfig::EPOXY
)

if(MSVC)
    target_compile_options(RoadLife PRIVATE /W4 /permissive-)
else()
    target_compile_options(RoadLife PRIVATE -Wall -Wextra -Wpedantic)
endif()
