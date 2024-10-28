#ifndef TEXFORMAT_H
#define TEXFORMAT_H

#include <GL/gl.h> // Include the OpenGL header

enum class TextureFormat {
    // 8-bit formats
    RGBA8 = GL_RGBA8,           // 8 bits per channel, 4 channels

    // 16-bit formats
    RGBA16 = GL_RGBA16,         // 16 bits per channel, 4 channels
    RGBA16F = GL_RGBA16F,       // 16 bits floating point, 4 channels

    // 32-bit formats
    RGBA32F = GL_RGBA32F,       // 32 bits floating point, 4 channels
    RGBA32I = GL_RGBA32I,       // 32 bits signed integer, 4 channels
    RGBA32UI = GL_RGBA32UI,     // 32 bits unsigned integer, 4 channels
};

#endif // TEXFORMAT_H
