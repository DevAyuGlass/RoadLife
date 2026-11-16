
#include "Scene.h"

#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <vector>
#include <cmath>

// ============================================================
// MAIN VERTEX SHADER
// ============================================================

static const char* vertexShader = R"(
#version 420 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uLightSpaceMatrix;

out vec3 vWorldPosition;
out vec3 vNormal;
out vec3 vColor;
out vec4 vLightSpacePosition;

void main()
{
    vec4 worldPosition = uModel * vec4(aPosition, 1.0);

    vWorldPosition = worldPosition.xyz;
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vColor = aColor;

    vLightSpacePosition = uLightSpaceMatrix * worldPosition;

    gl_Position = uProjection * uView * worldPosition;
}
)";

// ============================================================
// MAIN FRAGMENT SHADER
// ============================================================

static const char* fragmentShader = R"(
#version 420 core

in vec3 vWorldPosition;
in vec3 vNormal;
in vec3 vColor;
in vec4 vLightSpacePosition;

uniform vec3 uLightDirection;
uniform vec3 uViewPosition;

uniform vec3 uObjectColor;
uniform vec3 uSpecularColor;

uniform vec3 uAmbientColor;
uniform vec3 uLightColor;

uniform bool uUseLighting;
uniform sampler2D uShadowMap;
uniform bool uFogEnabled;
uniform bool uShadowsEnabled;
uniform float uFogDensity;
uniform vec3 uFogColor;

out vec4 FragColor;

float calculateShadow(vec3 normal, vec3 lightDir)
{
    vec3 projected = vLightSpacePosition.xyz /
                     vLightSpacePosition.w;

    projected = projected * 0.5 + 0.5;

    if (projected.z > 1.0 ||
        projected.x < 0.0 || projected.x > 1.0 ||
        projected.y < 0.0 || projected.y > 1.0)
    {
        return 0.0;
    }

    float currentDepth = projected.z;

    float bias = max(
        0.003 * (1.0 - dot(normal, lightDir)),
        0.0005
    );

    vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);

    float shadow = 0.0;

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float closestDepth = texture(
                uShadowMap,
                projected.xy + vec2(x, y) * texelSize
            ).r;

            shadow += currentDepth - bias > closestDepth
                      ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}

void main()
{
    if (!uUseLighting)
    {
        FragColor = vec4(vColor, 1.0);
        return;
    }

    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(-uLightDirection);
    vec3 viewDir = normalize(uViewPosition - vWorldPosition);

    vec3 ambient = uAmbientColor * uObjectColor;

    float diffuseStrength = max(
        dot(normal, lightDir),
        0.0
    );

    vec3 diffuse =
        diffuseStrength * uObjectColor * uLightColor * 0.75;

    vec3 reflectDir = reflect(-lightDir, normal);

    float specularStrength = pow(
        max(dot(viewDir, reflectDir), 0.0),
        32.0
    );

    vec3 specular =
        specularStrength * uSpecularColor * uLightColor * 0.4;

    float shadow = uShadowsEnabled
        ? calculateShadow(normal, lightDir)
        : 0.0;

    vec3 result = ambient +
        (1.0 - shadow) * (diffuse + specular);

    if (uFogEnabled)
    {
        float distanceToCamera = length(uViewPosition - vWorldPosition);
        float fogFactor = 1.0 - exp(-uFogDensity * uFogDensity * distanceToCamera * distanceToCamera);
        result = mix(result, uFogColor, clamp(fogFactor, 0.0, 1.0));
    }

    FragColor = vec4(result, 1.0);
}
)";

// ============================================================
// DEPTH SHADERS
// ============================================================

static const char* depthVertexShader = R"(
#version 420 core

layout(location = 0) in vec3 aPosition;

uniform mat4 uModel;
uniform mat4 uLightSpaceMatrix;

void main()
{
    gl_Position = uLightSpaceMatrix *
                  uModel *
                  vec4(aPosition, 1.0);
}
)";

static const char* depthFragmentShader = R"(
#version 420 core

void main()
{
}
)";

// ============================================================
// SKY SHADERS
// ============================================================

static const char* skyVertexShader = R"(
#version 420 core

out vec2 vUV;

void main()
{
    vec2 p = vec2(
        (gl_VertexID << 1) & 2,
        gl_VertexID & 2
    );

    vUV = p;

    gl_Position = vec4(
        p * 2.0 - 1.0,
        1.0,
        1.0
    );
}
)";

static const char* skyFragmentShader = R"(
#version 420 core

in vec2 vUV;

uniform mat4 uInvProjection;
uniform mat4 uInvViewRotation;

uniform vec3 uSunDirection;
uniform vec3 uSkyTop;
uniform vec3 uSkyHorizon;
uniform vec3 uSunColor;

out vec4 FragColor;

void main()
{
    vec4 clipPosition = vec4(
        vUV * 2.0 - 1.0,
        1.0,
        1.0
    );

    vec4 viewPosition = uInvProjection * clipPosition;

    vec3 viewDirection = normalize(
        viewPosition.xyz / viewPosition.w
    );

    vec3 worldDirection = normalize(
        mat3(uInvViewRotation) * viewDirection
    );

    float height = smoothstep(
        -0.08,
        0.55,
        worldDirection.y
    );

    vec3 sky = mix(
        uSkyHorizon,
        uSkyTop,
        height
    );

    float sunDot = dot(
        worldDirection,
        normalize(uSunDirection)
    );

    float sunVisibility = smoothstep(
        -0.18,
        0.02,
        uSunDirection.y
    );

    float sunDisc = smoothstep(
        0.99975,
        0.99993,
        sunDot
    );

    float sunHalo = pow(
        max(sunDot, 0.0),
        64.0
    ) * 0.18;

    sky += uSunColor *
           (sunDisc + sunHalo) *
           sunVisibility;

    FragColor = vec4(sky, 1.0);
}
)";

// ============================================================
// TIME OF DAY HELPERS
// ============================================================

static float smoothStep(
    float edge0,
    float edge1,
    float value
)
{
    float t = glm::clamp(
        (value - edge0) / (edge1 - edge0),
        0.0f,
        1.0f
    );

    return t * t * (3.0f - 2.0f * t);
}

// ============================================================
// CONSTRUCTOR
// ============================================================

Scene::Scene()
{
    shader = std::make_unique<Shader>(
        vertexShader,
        fragmentShader
    );

    depthShader = std::make_unique<Shader>(
        depthVertexShader,
        depthFragmentShader
    );

    skyShader = std::make_unique<Shader>(
        skyVertexShader,
        skyFragmentShader
    );

    shadowMap = std::make_unique<ShadowMap>(1024);

    glGenVertexArrays(1, &skyVAO);

    createCube();
    createGrid();
    createGround();

    objects = {
        {
            { 0.0f, 0.0f,  0.0f},
            { 1.0f, 1.0f, 1.0f},
            { 0.85f, 0.25f, 0.20f},
            { 0.9f, 0.9f, 0.9f}
        },
        {
            { 3.0f, 0.0f, -2.0f},
            { 0.8f, 0.8f, 0.8f},
            { 0.20f, 0.55f, 0.95f},
            { 0.7f, 0.8f, 1.0f}
        },
        {
            {-3.0f, 0.0f, -3.0f},
            { 1.2f, 1.2f, 1.2f},
            { 0.25f, 0.80f, 0.40f},
            { 0.8f, 1.0f, 0.8f}
        },
        {
            { 0.0f, 1.5f, -5.0f},
            { 0.7f, 0.7f, 0.7f},
            { 0.95f, 0.75f, 0.20f},
            { 1.0f, 0.9f, 0.6f}
        }
    };

    updateLighting();
}

// ============================================================
// DESTRUCTOR
// ============================================================

Scene::~Scene()
{
    if (skyVAO != 0)
        glDeleteVertexArrays(1, &skyVAO);
}

// ============================================================
// VALIDATION
// ============================================================

bool Scene::isValid() const
{
    return shader && shader->isValid() &&
           depthShader && depthShader->isValid() &&
           skyShader && skyShader->isValid() &&
           skyVAO != 0 &&
           shadowMap && shadowMap->isValid() &&
           cube && grid && ground;
}

// ============================================================
// TIME UPDATE
// ============================================================

void Scene::setTimeOfDay(float hours)
{
    timeOfDay = std::fmod(hours, 24.0f);

    if (timeOfDay < 0.0f)
        timeOfDay += 24.0f;

    updateLighting();
}

float Scene::getTimeOfDay() const
{
    return timeOfDay;
}

void Scene::setWireframe(bool enabled)
{
    wireframe = enabled;
}

bool Scene::isWireframe() const
{
    return wireframe;
}

void Scene::setFogEnabled(bool enabled)
{
    fogEnabled = enabled;
}

bool Scene::isFogEnabled() const
{
    return fogEnabled;
}

void Scene::setFogDensity(float density)
{
    if (std::isfinite(density))
        fogDensity = glm::clamp(density, 0.0001f, 0.1f);
}

float Scene::getFogDensity() const
{
    return fogDensity;
}

void Scene::setShadowsEnabled(bool enabled)
{
    shadowsEnabled = enabled;
}

bool Scene::areShadowsEnabled() const
{
    return shadowsEnabled;
}

void Scene::setGridEnabled(bool enabled)
{
    gridEnabled = enabled;
}

bool Scene::isGridEnabled() const
{
    return gridEnabled;
}

std::size_t Scene::getObjectCount() const
{
    return objects.size();
}

void Scene::spawnCube(const glm::vec3& position, float scale)
{
    static const std::array<glm::vec3, 6> palette = {{
        {0.85f, 0.25f, 0.20f}, {0.20f, 0.55f, 0.95f},
        {0.25f, 0.80f, 0.40f}, {0.95f, 0.75f, 0.20f},
        {0.70f, 0.30f, 0.90f}, {0.20f, 0.80f, 0.80f}
    }};
    const glm::vec3 color = palette[objects.size() % palette.size()];
    const float safeScale = glm::clamp(scale, 0.1f, 20.0f);
    objects.push_back({position, glm::vec3(safeScale), color, glm::vec3(0.8f)});
}

void Scene::clearObjects()
{
    objects.clear();
}

void Scene::update(float deltaTime)
{
    // 24 игровых часов за 20 реальных минут.
    timeOfDay += deltaTime * (24.0f / 1200.0f);

    if (timeOfDay >= 24.0f)
        timeOfDay -= 24.0f;

    updateLighting();
}

// ============================================================
// DYNAMIC LIGHTING
// ============================================================

void Scene::updateLighting()
{
    constexpr float PI = 3.14159265359f;

    float angle =
        (timeOfDay / 24.0f) * 2.0f * PI - PI * 0.5f;

    sunDirection = glm::normalize(glm::vec3(
        std::cos(angle),
        std::sin(angle),
        0.25f
    ));

    sunHeight = sunDirection.y;

    lightDirection = -sunDirection;

    dayFactor = smoothStep(
        -0.12f,
        0.25f,
        sunHeight
    );

    float twilight = 1.0f - smoothStep(
        0.0f,
        0.35f,
        std::abs(sunHeight)
    );

    const glm::vec3 nightTop(
        0.008f, 0.015f, 0.045f
    );

    const glm::vec3 nightHorizon(
        0.025f, 0.035f, 0.075f
    );

    const glm::vec3 dayTop(
        0.22f, 0.48f, 0.82f
    );

    const glm::vec3 dayHorizon(
        0.72f, 0.82f, 0.92f
    );

    const glm::vec3 sunset(
        0.95f, 0.30f, 0.12f
    );

    skyTopColor = glm::mix(
        nightTop,
        dayTop,
        dayFactor
    );

    skyHorizonColor = glm::mix(
        nightHorizon,
        dayHorizon,
        dayFactor
    );

    skyHorizonColor = glm::mix(
        skyHorizonColor,
        sunset,
        twilight * 0.75f
    );

    sunColor = glm::mix(
        glm::vec3(1.0f, 0.35f, 0.12f),
        glm::vec3(1.0f, 0.91f, 0.76f),
        dayFactor
    );

    ambientColor = glm::mix(
        glm::vec3(0.025f, 0.035f, 0.075f),
        glm::vec3(0.18f, 0.18f, 0.18f),
        dayFactor
    );

    lightColor = glm::mix(
        glm::vec3(0.015f, 0.018f, 0.035f),
        glm::vec3(1.0f, 0.92f, 0.82f),
        dayFactor
    );

    lightColor = glm::mix(
        lightColor,
        glm::vec3(1.0f, 0.43f, 0.20f),
        twilight * 0.65f
    );
}

// ============================================================
// SKY RENDERING
// ============================================================

void Scene::renderSky(
    const glm::mat4& view,
    const glm::mat4& projection
)
{
    skyShader->use();

    glm::mat4 inverseProjection = glm::inverse(projection);

    glm::mat4 inverseViewRotation(
        glm::mat3(glm::inverse(view))
    );

    skyShader->setMat4(
        "uInvProjection",
        inverseProjection
    );

    skyShader->setMat4(
        "uInvViewRotation",
        inverseViewRotation
    );

    skyShader->setVec3("uSunDirection", sunDirection);
    skyShader->setVec3("uSkyTop", skyTopColor);
    skyShader->setVec3("uSkyHorizon", skyHorizonColor);
    skyShader->setVec3("uSunColor", sunColor);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glBindVertexArray(skyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

// ============================================================
// CUBE
// ============================================================

void Scene::createCube()
{
    std::vector<float> vertices;

    const std::array<glm::vec3, 8> positions = {{
        {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
        {-1,-1, 1}, {1,-1, 1}, {1,1, 1}, {-1,1, 1}
    }};

    struct Face
    {
        int a, b, c, d;
        glm::vec3 normal;
    };

    const Face faces[] = {
        {4,5,6,7, { 0, 0, 1}},
        {1,0,3,2, { 0, 0,-1}},
        {0,4,7,3, {-1, 0, 0}},
        {5,1,2,6, { 1, 0, 0}},
        {3,7,6,2, { 0, 1, 0}},
        {0,1,5,4, { 0,-1, 0}}
    };

    const int indices[] = {0, 1, 2, 2, 3, 0};

    const glm::vec3 colors[] = {
        {0.85f, 0.25f, 0.20f},
        {0.20f, 0.55f, 0.95f},
        {0.25f, 0.80f, 0.40f},
        {0.95f, 0.75f, 0.20f},
        {0.70f, 0.30f, 0.90f},
        {0.20f, 0.80f, 0.80f}
    };

    for (int faceIndex = 0; faceIndex < 6; ++faceIndex)
    {
        const Face& face = faces[faceIndex];

        const int faceIndices[] = {
            face.a, face.b, face.c, face.d
        };

        for (int index : indices)
        {
            const glm::vec3& p = positions[faceIndices[index]];
            const glm::vec3& n = face.normal;
            const glm::vec3& c = colors[faceIndex];

            vertices.insert(vertices.end(), {
                p.x, p.y, p.z,
                n.x, n.y, n.z,
                c.r, c.g, c.b
            });
        }
    }

    cube = std::make_unique<Mesh>(vertices, GL_TRIANGLES);
}

// ============================================================
// GROUND
// ============================================================

void Scene::createGround()
{
    std::vector<float> vertices;

    const float size = 60.0f;
    const float y = -1.02f;

    const glm::vec3 normal(0.0f, 1.0f, 0.0f);
    const glm::vec3 color(0.30f, 0.32f, 0.29f);

    const glm::vec3 points[] = {
        {-size, y, -size},
        { size, y, -size},
        { size, y,  size},

        {-size, y, -size},
        { size, y,  size},
        {-size, y,  size}
    };

    for (const auto& p : points)
    {
        vertices.insert(vertices.end(), {
            p.x, p.y, p.z,
            normal.x, normal.y, normal.z,
            color.r, color.g, color.b
        });
    }

    ground = std::make_unique<Mesh>(vertices, GL_TRIANGLES);
}

// ============================================================
// GRID
// ============================================================

void Scene::createGrid()
{
    std::vector<float> vertices;

    const int extent = 20;
    const float y = -1.005f;

    for (int i = -extent; i <= extent; ++i)
    {
        float color = (i == 0) ? 0.35f : 0.13f;

        glm::vec3 lineColor(color, color, color);

        const glm::vec3 points[] = {
            {-static_cast<float>(extent), y, static_cast<float>(i)},
            { static_cast<float>(extent), y, static_cast<float>(i)},
            { static_cast<float>(i), y, -static_cast<float>(extent)},
            { static_cast<float>(i), y,  static_cast<float>(extent)}
        };

        for (const auto& point : points)
        {
            vertices.insert(vertices.end(), {
                point.x, point.y, point.z,
                0.0f, 1.0f, 0.0f,
                lineColor.r, lineColor.g, lineColor.b
            });
        }
    }

    grid = std::make_unique<Mesh>(vertices, GL_LINES);
}

// ============================================================
// SHADOW PASS
// ============================================================

void Scene::renderDepthPass()
{
    glm::vec3 lightPosition = -lightDirection * 35.0f;

    glm::mat4 lightProjection = glm::ortho(
        -30.0f, 30.0f,
        -30.0f, 30.0f,
        1.0f, 100.0f
    );

    glm::mat4 lightView = glm::lookAt(
        lightPosition,
        glm::vec3(0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    lightSpaceMatrix = lightProjection * lightView;

    shadowMap->begin();

    depthShader->use();

    depthShader->setMat4(
        "uLightSpaceMatrix",
        lightSpaceMatrix
    );

    for (const auto& object : objects)
    {
        glm::mat4 model(1.0f);

        model = glm::translate(model, object.position);
        model = glm::scale(model, object.scale);

        depthShader->setMat4("uModel", model);

        cube->draw();
    }

    shadowMap->end();
}

// ============================================================
// OBJECT RENDERING
// ============================================================

void Scene::renderObjects(
    const glm::mat4& view,
    const glm::mat4& projection
)
{
    shader->use();

    shader->setMat4("uView", view);
    shader->setMat4("uProjection", projection);
    shader->setMat4("uLightSpaceMatrix", lightSpaceMatrix);

    shader->setVec3("uLightDirection", lightDirection);
    shader->setVec3("uAmbientColor", ambientColor);
    shader->setVec3("uLightColor", lightColor);
    shader->setBool("uFogEnabled", fogEnabled);
    // Avoid sampling a stale shadow map while the sun is below the horizon.
    shader->setBool("uShadowsEnabled", shadowsEnabled && sunHeight > 0.0f);
    shader->setFloat("uFogDensity", fogDensity);
    shader->setVec3("uFogColor", skyHorizonColor);

    glm::vec3 cameraPosition = glm::vec3(
        glm::inverse(view)[3]
    );

    shader->setVec3("uViewPosition", cameraPosition);

    glActiveTexture(GL_TEXTURE1);

    glBindTexture(
        GL_TEXTURE_2D,
        shadowMap->getDepthTexture()
    );

    shader->setInt("uShadowMap", 1);

    // Ground.
    shader->setBool("uUseLighting", true);
    shader->setMat4("uModel", glm::mat4(1.0f));
    shader->setVec3("uObjectColor", {0.30f, 0.32f, 0.29f});
    shader->setVec3("uSpecularColor", {0.05f, 0.05f, 0.05f});

    ground->draw();

    // Grid.
    if (gridEnabled)
    {
        shader->setBool("uUseLighting", false);
        grid->draw();
    }

    // Cubes.
    shader->setBool("uUseLighting", true);

    for (const auto& object : objects)
    {
        glm::mat4 model(1.0f);

        model = glm::translate(model, object.position);
        model = glm::scale(model, object.scale);

        shader->setMat4("uModel", model);
        shader->setVec3("uObjectColor", object.color);
        shader->setVec3("uSpecularColor", object.specular);

        cube->draw();
    }

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
}

// ============================================================
// MAIN RENDER
// ============================================================

void Scene::render(
    const glm::mat4& view,
    const glm::mat4& projection
)
{
    renderSky(view, projection);

    // Keep the shadow map solid even when the visible scene is wireframe.
    if (shadowsEnabled && sunHeight > 0.0f)
        renderDepthPass();

    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    renderObjects(view, projection);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

