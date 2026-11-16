#include "ShadowMap.h"

#include <iostream>

ShadowMap::ShadowMap(int resolution)
    : resolution(resolution)
{
    glGenFramebuffers(1, &framebuffer);
    glGenTextures(1, &depthTexture);

    glBindTexture(GL_TEXTURE_2D, depthTexture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT24,
        resolution,
        resolution,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_NEAREST
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_NEAREST
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        depthTexture,
        0
    );

    const GLenum noColorBuffers[] = { GL_NONE };
    glDrawBuffers(1, noColorBuffers);
    glReadBuffer(GL_NONE);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Shadow framebuffer incomplete: "
                  << status << '\n';
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

ShadowMap::~ShadowMap()
{
    if (depthTexture)
        glDeleteTextures(1, &depthTexture);

    if (framebuffer)
        glDeleteFramebuffers(1, &framebuffer);
}

bool ShadowMap::isValid() const
{
    return framebuffer != 0 && depthTexture != 0;
}

void ShadowMap::begin()
{
    glGetIntegerv(
        GL_FRAMEBUFFER_BINDING,
        &previousFramebuffer
    );

    glGetIntegerv(
        GL_VIEWPORT,
        previousViewport
    );

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glViewport(
        0,
        0,
        resolution,
        resolution
    );

    glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMap::end()
{
    glBindFramebuffer(
        GL_FRAMEBUFFER,
        static_cast<GLuint>(previousFramebuffer)
    );

    glViewport(
        previousViewport[0],
        previousViewport[1],
        previousViewport[2],
        previousViewport[3]
    );
}

GLuint ShadowMap::getDepthTexture() const
{
    return depthTexture;
}

int ShadowMap::getResolution() const
{
    return resolution;
}
