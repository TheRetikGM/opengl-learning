#include "glfbo.h"
#include "config.h"
#include "DebugColors.h"
#include <iostream>

using namespace glfbo;

ColorBuffer::ColorBuffer(unsigned int width, unsigned int height, 
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
        std::cout << DC_WARNING << " ColorBuffer: samples == 1 will not create multisampled color buffer.\n";
#endif
    if (type == BufferType::Texture) {
        glGenTextures(1, &ID);        
        GLint target;
        if (samples > 1) {
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ID);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internal_format, width, height, GL_TRUE);
            target  = GL_TEXTURE_2D_MULTISAMPLE;
        } 
        else {
            Samples = 0;
            glBindTexture(GL_TEXTURE_2D, ID);
            glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, GL_RGBA, GL_FLOAT, 0);            
            target  = GL_TEXTURE_2D;
        }
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap_s);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap_t);        
        glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, (float *)&bordercolor);
        glBindTexture(target, 0); 
    }
    else if (type == BufferType::Renderbuffer) {
        glGenRenderbuffers(1, &ID);
        glBindRenderbuffer(GL_RENDERBUFFER, ID);
        
        if (samples <= 1) {
            samples = 0;
        }         
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internal_format, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

    }
    else if (type == BufferType::Cubemap) {
        glGenTextures(GL_TEXTURE_CUBE_MAP, &ID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

        for (int i = 0; i < 6; i++)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal_format, width , height, 0, GL_RGBA, GL_FLOAT, 0);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrap_s);
	    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrap_t);
	    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrap_r);
	    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);            
    }
    else {
        ID = 0;
    } 
}

ColorBuffer::ColorBuffer(unsigned int width, unsigned int height, 
            BufferType type,
            GLint internal_format,
            unsigned int samples,            
            GLint wrap_str,                    
            vector3 bordercolor) 
            : ColorBuffer(width, height, type, internal_format, samples, wrap_str, wrap_str, wrap_str, bordercolor) {}

ColorBuffer::~ColorBuffer()
{
    if (Type == BufferType::Cubemap || Type == BufferType::Texture)
        glDeleteTextures(1, &ID);
    else
        glDeleteRenderbuffers(1, &ID);        
}

GLenum ColorBuffer::BindTarget()
{
    if (Type == BufferType::Cubemap) {
        return GL_TEXTURE_CUBE_MAP;
    } 
    else if (Type == BufferType::Texture) {
        if (Samples > 1)
            return GL_TEXTURE_2D_MULTISAMPLE;
        else 
            return GL_TEXTURE_2D;
    }
    else {
        return GL_RENDERBUFFER;
    }
}
GLint ColorBuffer::Get_internal_format()
{
    return int_format;
}