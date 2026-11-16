
#pragma once

#include "GL.h"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

class Shader
{
public:
    Shader(const char* vertexSource, const char* fragmentSource);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void use() const;

    void setMat4(const char* name, const glm::mat4& value) const;
    void setMat3(const char* name, const glm::mat3& value) const;
    void setVec3(const char* name, const glm::vec3& value) const;
    void setBool(const char* name, bool value) const;
    void setInt(const char* name, int value) const;
    void setFloat(const char* name, float value) const;

    bool isValid() const;

private:
    GLuint program = 0;
    mutable std::unordered_map<std::string, GLint> uniformLocations;

    GLint getLocation(const char* name) const;

    GLuint compile(GLenum type, const char* source);
};

