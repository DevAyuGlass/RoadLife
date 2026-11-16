
#include "Mesh.h"

Mesh::Mesh(const std::vector<float>& vertices, GLenum mode)
    : vertexCount(static_cast<GLsizei>(vertices.size() / 9)),
      drawMode(mode)
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
        vertices.data(),
        GL_STATIC_DRAW
    );

    constexpr GLsizei stride = 9 * sizeof(float);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, stride,
        reinterpret_cast<void*>(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        2, 3, GL_FLOAT, GL_FALSE, stride,
        reinterpret_cast<void*>(6 * sizeof(float))
    );
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

Mesh::~Mesh()
{
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
}

void Mesh::draw() const
{
    glBindVertexArray(vao);
    glDrawArrays(drawMode, 0, vertexCount);
    glBindVertexArray(0);
}

