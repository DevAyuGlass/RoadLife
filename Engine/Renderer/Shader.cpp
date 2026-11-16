
#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

GLuint Shader::compile(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char log[2048]{};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader compilation error:\n" << log << '\n';

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

Shader::Shader(const char* vertexSource, const char* fragmentSource)
{
    GLuint vertex = compile(GL_VERTEX_SHADER, vertexSource);
    GLuint fragment = compile(GL_FRAGMENT_SHADER, fragmentSource);

    if (!vertex || !fragment)
    {
        if (vertex) glDeleteShader(vertex);
        if (fragment) glDeleteShader(fragment);
        return;
    }

    program = glCreateProgram();

    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    GLint success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        char log[2048]{};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "Shader linking error:\n" << log << '\n';

        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader()
{
    if (program)
        glDeleteProgram(program);
}

void Shader::use() const
{
    glUseProgram(program);
}

void Shader::setMat4(const char* name, const glm::mat4& value) const
{
    glUniformMatrix4fv(
        glGetUniformLocation(program, name),
        1, GL_FALSE, glm::value_ptr(value)
    );
}

void Shader::setVec3(const char* name, const glm::vec3& value) const
{
    glUniform3fv(
        glGetUniformLocation(program, name),
        1, glm::value_ptr(value)
    );
}

void Shader::setBool(const char* name, bool value) const
{
    glUniform1i(glGetUniformLocation(program, name), value ? 1 : 0);
}

void Shader::setInt(const char* name, int value) const
{
    glUniform1i(
        glGetUniformLocation(program, name),
        value
    );
}

void Shader::setFloat(const char* name, float value) const
{
    glUniform1f(glGetUniformLocation(program, name), value);
}

bool Shader::isValid() const
{
    return program != 0;
}

