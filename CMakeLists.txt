cmake_minimum_required(VERSION 3.25)

project(RoadLife VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ROADLIFE_SOURCES
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
)

if(ANDROID)
    # SDL3 is supplied by the official Android AAR through Gradle Prefab.
    find_package(SDL3 CONFIG REQUIRED)

    include(FetchContent)
    set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(glm
        URL https://github.com/g-truc/glm/archive/refs/tags/1.0.1.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(glm)

    add_library(main SHARED ${ROADLIFE_SOURCES})
    target_include_directories(main PRIVATE Engine)
    target_link_libraries(main PRIVATE
        SDL3::SDL3
        glm::glm
        GLESv3
        android
        log
    )
    target_compile_definitions(main PRIVATE GLM_FORCE_RADIANS)
    target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic)
else()
    find_package(SDL3 CONFIG REQUIRED)
    find_package(SDL3_ttf CONFIG REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(glm CONFIG REQUIRED)
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(EPOXY REQUIRED IMPORTED_TARGET epoxy)

    add_executable(RoadLife
        ${ROADLIFE_SOURCES}
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
endif()

