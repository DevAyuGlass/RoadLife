#pragma once

#include GL.h

class ShadowMap
{
public:
    explicit ShadowMap(int resolution = 1024);
    ~ShadowMap();

    ShadowMap(const ShadowMap&) = delete;
    ShadowMap& operator=(const ShadowMap&) = delete;

    bool isValid() const;

    void begin();
    void end();

    GLuint getDepthTexture() const;
    int getResolution() const;

private:
    GLuint framebuffer = 0;
    GLuint depthTexture = 0;

    int resolution = 1024;

    GLint previousFramebuffer = 0;
    GLint previousViewport[4] = {};
};
