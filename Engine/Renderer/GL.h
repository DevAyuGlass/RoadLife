#pragma once

// One GL include point keeps the engine portable between desktop OpenGL
// and Android OpenGL ES 3.
#if defined(__ANDROID__)
    #include <GLES3/gl3.h>
#else
    #include <epoxy/gl.h>
#endif
