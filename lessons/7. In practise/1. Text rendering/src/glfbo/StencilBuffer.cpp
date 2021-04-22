#include "glfbo.h"
#include <iostream>
#include "config.h"
#include "DebugColors.h"

using namespace glfbo;

StencilBuffer::StencilBuffer(unsigned int width, unsigned int height, 
                      BufferType type,
                      unsigned int samples) : Width(width), Height(height), Samples(samples), Type(type)
{
    if (type == BufferType::Cubemap) {
#ifdef DEBUG    
        std::cout << DC_WARNING << " StencilBuffer: Bad buffer type. Using BufferType::Renderbuffer.\n";
#endif
        type = Type = BufferType::Renderbuffer;
    }

#ifdef DEBUG
    if (samples == 1)
        std::cout << DC_WARNING << " StencilBuffer: samples == 1 will not create multisampled stencil buffer.\n";
#endif

    if (type == BufferType::Renderbuffer) {
        glGenRenderbuffers(1, &ID);
        glBindRenderbuffer(GL_RENDERBUFFER, ID);

        if (samples <= 1) {
            samples = 0;
        }
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 0, GL_STENCIL_INDEX8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
    else {
        glGenTextures(1, &ID);
        GLint target;

        if (samples > 1) {
            target = GL_TEXTURE_2D_MULTISAMPLE;            
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_STENCIL_INDEX8, width, height, GL_TRUE);
        }
        else {
            Samples = 0;
            target = GL_TEXTURE_2D;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX8, width, height, 0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, 0);
        }
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(target, 0);
    }
}

StencilBuffer::~StencilBuffer()
{
    if (Type == BufferType::Texture)
        glDeleteTextures(1, &ID);
    else if (Type == BufferType::Renderbuffer)
        glDeleteRenderbuffers(1, &ID);
}

GLenum StencilBuffer::BindTarget()
{
    if (Type == BufferType::Renderbuffer)
        return GL_RENDERBUFFER;
    else if (Type == BufferType::Texture) {
        if (Samples > 1)
            return GL_TEXTURE_2D_MULTISAMPLE;
        else
            return GL_TEXTURE_2D;
    }
    else
    {
#ifdef DEBUG
        std::cout << DC_ERROR << " StencilBuffer::BindTarget(): Bad buffer type. Returning GL_RENDERBUFFER.\n";
#endif
        return GL_RENDERBUFFER;
    }
}