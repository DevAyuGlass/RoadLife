#pragma once

#include "../IRenderer.h"

class OpenGLRenderer final : public IRenderer
{
public:
    bool initialize() override;
    void beginFrame(int width, int height) override;
    void shutdown() override;

private:
    bool initialized = false;
};
