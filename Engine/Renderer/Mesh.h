
#pragma once

#include <epoxy/gl.h>
#include <vector>

class Mesh
{
public:
    Mesh(const std::vector<float>& vertices, GLenum drawMode);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    void draw() const;

private:
    GLuint vao = 0;
    GLuint vbo = 0;

    GLsizei vertexCount = 0;
    GLenum drawMode = GL_TRIANGLES;
};

