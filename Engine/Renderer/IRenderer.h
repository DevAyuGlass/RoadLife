#pragma once

class IRenderer
{
public:
    virtual ~IRenderer() = default;

    IRenderer() = default;
    IRenderer(const IRenderer&) = delete;
    IRenderer& operator=(const IRenderer&) = delete;

    virtual bool initialize() = 0;
    virtual void beginFrame(int width, int height) = 0;
    virtual void shutdown() = 0;
};
