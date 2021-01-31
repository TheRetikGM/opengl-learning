#include "glfbo.h"
#include "config.h"
#include "DebugColors.h"
#include <iostream>

using namespace glfbo;

DepthStencilBuffer::DepthStencilBuffer(unsigned int width, unsigned int height,
                                       BufferType type,
                                       GLint internal_format,
                                       unsigned int samples,
                                       GLint wrap_s,
                                       GLint wrap_t,
                                       GLint wrap_r,
                                       vector3 bordercolor) : Width(width), Height(height), Type(type), Samples(samples), int_format(internal_format)
{
#ifdef DEBUG
    if (samples == 1)
        std::cout << DC_WARNING << "DepthBuffer: samples == 1 will not create multisampled depth buffer.\n";
#endif

    if (type == BufferType::Cubemap)
    {
#ifdef DEBUG
        std::cout << DC_WARNING << " DepthStencilBuffer: Bad buffer type. Using BufferType::Renderbuffer.\n";
#endif
        type = Type = BufferType::Renderbuffer;
    }

    if (type == BufferType::Renderbuffer) {
        glGenRenderbuffers(1, &ID);
        glBindRenderbuffer(GL_RENDERBUFFER, ID);

        if (samples <= 1) {
            samples = 0;
        }        
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internal_format, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
    else {
        glGenTextures(1, &ID);
        GLint target;

        if (samples > 1) {
            target = GL_TEXTURE_2D_MULTISAMPLE;
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internal_format, width, height, GL_TRUE);
        }
        else {
            target = GL_TEXTURE_2D;
            glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, GL_RGBA, GL_FLOAT, 0);
        }
        glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap_s);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap_t);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, (float*)&bordercolor);
        glBindTexture(target, 0);
    }
}

DepthStencilBuffer::DepthStencilBuffer(unsigned int width, unsigned int height,
                                       BufferType type,
                                       GLint internal_format,
                                       unsigned int samples,
                                       GLint wrap_str,
                                       vector3 bordercolor)
    : DepthStencilBuffer(width, height, type, internal_format, samples, wrap_str, wrap_str, wrap_str, bordercolor) {}

DepthStencilBuffer::~DepthStencilBuffer()
{
    if (Type == BufferType::Renderbuffer)
        glDeleteRenderbuffers(1, &ID);
    else
        glDeleteTextures(1, &ID);
}

GLenum DepthStencilBuffer::BindTarget()
{
    if (Type == BufferType::Renderbuffer)
        return GL_RENDERBUFFER;
    else if (Type == BufferType::Texture)
    {
        if (Samples > 1)
            return GL_TEXTURE_2D_MULTISAMPLE;
        else
            return GL_TEXTURE_2D;
    }
    else {
#ifdef DEBUG
        std::cout << DC_WARNING << " DepthStencilBuffer::BindTarget(): Bad buffer type. Returning GL_RENDERBUFFER.\n";
#endif
        return GL_RENDERBUFFER;
    }
}
GLint DepthStencilBuffer::Get_internal_format()
{
    return int_format;
}