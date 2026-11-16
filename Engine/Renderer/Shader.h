
#pragma once

#include <epoxy/gl.h>
#include <glm/glm.hpp>

class Shader
{
public:
    Shader(const char* vertexSource, const char* fragmentSource);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void use() const;

    void setMat4(const char* name, const glm::mat4& value) const;
    void setVec3(const char* name, const glm::vec3& value) const;
    void setBool(const char* name, bool value) const;
    void setInt(const char* name, int value) const;
    void setFloat(const char* name, float value) const;

    bool isValid() const;

private:
    GLuint program = 0;

    GLuint compile(GLenum type, const char* source);
};

