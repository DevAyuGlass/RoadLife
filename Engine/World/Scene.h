
#pragma once

#include "../Renderer/Shader.h"
#include "../Renderer/Mesh.h"
#include "../Renderer/ShadowMap.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <epoxy/gl.h>

class Scene
{
public:
    Scene();
    ~Scene();

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    bool isValid() const;

    void update(float deltaTime);
    void setTimeOfDay(float hours);
    float getTimeOfDay() const;

    void setWireframe(bool enabled);
    bool isWireframe() const;
    void setFogEnabled(bool enabled);
    bool isFogEnabled() const;

    void render(
        const glm::mat4& view,
        const glm::mat4& projection
    );

private:
    struct Object
    {
        glm::vec3 position;
        glm::vec3 scale;
        glm::vec3 color;
        glm::vec3 specular;
    };

    std::unique_ptr<Shader> shader;
    std::unique_ptr<Shader> depthShader;
    std::unique_ptr<Shader> skyShader;

    std::unique_ptr<Mesh> cube;
    std::unique_ptr<Mesh> grid;
    std::unique_ptr<Mesh> ground;

    std::unique_ptr<ShadowMap> shadowMap;

    GLuint skyVAO = 0;

    std::vector<Object> objects;

    float timeOfDay = 14.0f;
    bool wireframe = false;
    bool fogEnabled = true;
    float fogDensity = 0.012f;

    float dayFactor = 1.0f;
    float sunHeight = 0.0f;

    glm::vec3 lightDirection{-0.7f, -0.5f, -0.4f};
    glm::vec3 sunDirection{0.0f, 1.0f, 0.0f};

    glm::vec3 ambientColor{0.18f};
    glm::vec3 lightColor{1.0f};

    glm::vec3 skyTopColor{0.22f, 0.48f, 0.82f};
    glm::vec3 skyHorizonColor{0.72f, 0.82f, 0.92f};
    glm::vec3 sunColor{1.0f, 0.88f, 0.68f};

    glm::mat4 lightSpaceMatrix{1.0f};

    void createCube();
    void createGrid();
    void createGround();

    void updateLighting();
    void renderSky(
        const glm::mat4& view,
        const glm::mat4& projection
    );

    void renderDepthPass();

    void renderObjects(
        const glm::mat4& view,
        const glm::mat4& projection
    );
};

