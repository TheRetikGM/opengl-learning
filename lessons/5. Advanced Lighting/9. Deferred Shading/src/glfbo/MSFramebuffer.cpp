#include <glad/glad.h>
#include "glfbo.h"
#include "config.h"
#include "DebugColors.h"
#include <stb_image_write.h>

#define WARN_EMPTY_BUFFERS DC_WARNING " MSFramebuffer::MSFramebuffer: No buffers specified. Framebuffer should have at least 1 buffer attached."
#define WARN_SAMPLES_LESS_TWO DC_WARNING " MSFramebuffer::MSFramebuffer: Cannot create multisamples framebuffer with samples < 2. Using samples = 2"
#define ERR_ATTACH_BUFFER_NULL(type) DC_ERROR " MSFramebuffer::Attach_" type "(): " type " wasn't attached. (buffer == NULL)"
#define ERR_ATTACH_SAMPLES_NOT_EQ(type) DC_ERROR " MSFramebuffer::Attach_" type "(): " type " wasn't attached. Samples aren't equal to samples of MSFramebuffer."
#define ERR_ATTACH_BUF_INCOMPLETE(type) DC_ERROR " MSFramebuffer::Attach_" type "(): Cannot attach incomplete " type "."
#define ERR_ATTACH_WRONG_BUF_TYPE(type) DC_ERROR " MSFramebuffer::Attach_" type "(): " type " wasn't attached. Wrong buffer type."
#define ERR_ATTACH_TOO_MANY_ATTACHMENTS DC_ERROR " MSFramebuffer::Attach_ColorBuffer(): ColorBuffer wasn't attached. Given number of color attachment > GL_MAX_COLOR_ATTACHMENTS."
#define ERR_COL_ATTACH_NOT_FOUND(func) DC_ERROR " MSFramebuffer::" func "(): Could not find color attachment."
#define ERR_INT_FBO_NOT_FOUND(func) DC_ERROR " MSFramebuffer::" func "(): Could not find intermediate framebuffer for specified color attachment. Did you forget to call Create_intFBO() ?"
#define ERR_EXPORT DC_ERROR " MSFramebuffer::Export(): Could not export ColorBuffer."Framebuffer* ifbo = 


using namespace glfbo;

void MSFramebuffer::handleSamples(unsigned int s)
{
    if (s < 2) {
        #ifdef DEBUG
        std::cout << WARN_SAMPLES_LESS_TWO << std::endl;
        #endif        
        this->samples = s = 2;
    }
    else 
        this->samples = s;
}
MSFramebuffer::MSFramebuffer(unsigned int samples, std::unordered_map<unsigned int, ColorBuffer*> cBufs, DepthBuffer* dBuf, StencilBuffer* sBuf)
{
    glGenFramebuffers(1, &ID);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);

    handleSamples(samples);
    if (cBufs.size() != 0) {
        bool all_null = true;
        for (auto i : cBufs) {
            if (i.second != NULL) {
                all_null = false;
                break;
            }
        }
        if (all_null && !dBuf && !sBuf) {
            #ifdef DEBUG
            std::cout << WARN_EMPTY_BUFFERS << std::endl;
            #endif
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return;    
        }

    }
    else if (!dBuf && !sBuf) {
        #ifdef DEBUG
        std::cout << WARN_EMPTY_BUFFERS << std::endl;
        #endif
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    for (auto i : cBufs)
        Attach_ColorBuffer(i.first, i.second);
    if (dBuf)
        Attach_DepthBuffer(dBuf);
    if (sBuf)
        Attach_StencilBuffer(sBuf);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);    
}
MSFramebuffer::MSFramebuffer(unsigned int samples, unsigned int attachment, ColorBuffer* cBuf, DepthBuffer* dBuf, StencilBuffer* sBuf)
: MSFramebuffer(samples, {{attachment, cBuf}}, dBuf, sBuf) {}
MSFramebuffer::MSFramebuffer(unsigned int samples, std::unordered_map<unsigned int, ColorBuffer*> cBufs, DepthStencilBuffer* dsBuf)
{
    glGenFramebuffers(1, &ID);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);

    handleSamples(samples);
    if (cBufs.size() != 0) {
        bool all_null = true;
        for (auto i : cBufs) {
            if (i.second != NULL) {
                all_null = false;
                break;
            }
        }
        if (all_null && !dsBuf) {
            #ifdef DEBUG
            std::cout << WARN_EMPTY_BUFFERS << std::endl;
            #endif
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return;    
        }

    }
    else if (!dsBuf) {
        #ifdef DEBUG
        std::cout << WARN_EMPTY_BUFFERS << std::endl;
        #endif
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    for (auto i : cBufs)
        Attach_ColorBuffer(i.first, i.second);
    if (dsBuf)
        Attach_DepthStencilBuffer(dsBuf);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
MSFramebuffer::MSFramebuffer(unsigned int samples, unsigned int attachment, ColorBuffer* cBuf, DepthStencilBuffer* dsBuf)
: MSFramebuffer(samples, {{attachment, cBuf}}, dsBuf) {}
MSFramebuffer::MSFramebuffer(unsigned int samples)
{
    glGenFramebuffers(1, &ID);
    handleSamples(samples);
}
MSFramebuffer::~MSFramebuffer()
{
    for (auto i : intFBOs) {
        delete i.second->ColorBufs[0];  
        delete i.second;
    }
    glDeleteFramebuffers(1, &ID);
}

void MSFramebuffer::Attach_ColorBuffer(unsigned int attachment, ColorBuffer* buffer)
{
    GLint maxAttach = 0;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);
    if (attachment > maxAttach) {
        #ifdef DEBUG
        std::cout << ERR_ATTACH_TOO_MANY_ATTACHMENTS << std::endl;
        #endif
        return;
    }
    if (buffer != NULL) {
        if (buffer->Samples != this->samples) {
            #ifdef DEBUG
            std::cout << ERR_ATTACH_SAMPLES_NOT_EQ("ColorBuffer") << std::endl;
            #endif
            return;
        }
        if (buffer->ID == 0) {
            #ifdef DEBUG
            std::cout << ERR_ATTACH_BUF_INCOMPLETE("ColorBuffer") << " At index " << ColorBufs.size() << std::endl;
            #endif
            return;
        }                
        ColorBufs[attachment] = buffer;        
        glBindFramebuffer(GL_FRAMEBUFFER, ID);     
        switch (buffer->Type)
        {
            case BufferType::Texture:                
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_TEXTURE_2D_MULTISAMPLE, buffer->ID, 0);                    
                break;
            case BufferType::Renderbuffer:
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_RENDERBUFFER, buffer->ID);
                break;
            case BufferType::Cubemap:
                glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, buffer->ID, 0);
                break;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else {
        #ifdef DEBUG
        std::cout << ERR_ATTACH_BUFFER_NULL("ColorBuffer") << std::endl;
        #endif
    }    
}
void MSFramebuffer::Attach_DepthBuffer(DepthBuffer* buffer)
{
    DepthBuf = buffer;
    if (buffer != NULL) {
        if (buffer->Samples != this->samples) {
            #ifdef DEBUG
            std::cout << ERR_ATTACH_SAMPLES_NOT_EQ("DepthBuffer") << std::endl;
            #endif
            return;
        }
        if (buffer->ID == 0) {
            #ifdef DEBUG
            std::cout << ERR_ATTACH_BUF_INCOMPLETE("DepthBuffer") << std::endl;            
            #endif
            return;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        if (DepthBuf->Type == BufferType::Texture)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, DepthBuf->ID, 0);
        else if (DepthBuf->Type == BufferType::Renderbuffer)
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, DepthBuf->ID);            
        else if (DepthBuf->Type == BufferType::Cubemap)
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, DepthBuf->ID, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else {
        #ifdef DEBUG
        std::cout << ERR_ATTACH_BUFFER_NULL("DepthBuffer") << std::endl;
        #endif
    }    
}
void MSFramebuffer::Attach_StencilBuffer(StencilBuffer* buffer)
{
    StencilBuf = buffer;
    if (buffer != NULL) {
        if (buffer->Samples != this->samples) {
            #ifdef DEBUG
            std::cout << ERR_ATTACH_SAMPLES_NOT_EQ("StencilBuffer") << std::endl;
            #endif
            return;
        }
        if (buffer->ID == 0) {
            #ifdef DEBUG
            std::cout << ERR_ATTACH_BUF_INCOMPLETE("StencilBuffer") << std::endl;            
            #endif
            return;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        if (StencilBuf->Type == BufferType::Renderbuffer)
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, StencilBuf->ID);
        else if (StencilBuf->Type == BufferType::Texture)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, StencilBuf->ID, 0);
        else {
            #ifdef DEBUG
            std::cout << ERR_ATTACH_WRONG_BUF_TYPE("StencilBuffer") << std::endl;
            #endif
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else {
        #ifdef DEBUG
        std::cout << ERR_ATTACH_BUFFER_NULL("StencilBuffer") << std::endl;
        #endif
    }
}
void MSFramebuffer::Attach_DepthStencilBuffer(DepthStencilBuffer* buffer)
{
    DepthStencilBuf = buffer;
    if (buffer != NULL) {
        if (buffer->Samples != this->samples) {
            #ifdef DEBUG
            std::cout << ERR_ATTACH_SAMPLES_NOT_EQ("DepthStencilBuffer") << std::endl;
            #endif
            return;
        }
        if (buffer->ID == 0) {
            #ifdef DEBUG
            std::cout << ERR_ATTACH_BUF_INCOMPLETE("DepthStencilBuffer") << std::endl;            
            #endif
            return;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        if (DepthStencilBuf->Type == BufferType::Texture)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, DepthStencilBuf->ID, 0);
        else if (DepthStencilBuf->Type == BufferType::Renderbuffer)
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, DepthStencilBuf->ID);
        else {
            #ifdef DEBUG
            std::cout << ERR_ATTACH_WRONG_BUF_TYPE("DepthStencilBuffer") << std::endl;
            #endif
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else {
        #ifdef DEBUG
        std::cout << ERR_ATTACH_BUFFER_NULL("DepthStencilBuffer") << std::endl;
        #endif
    }
}
unsigned int MSFramebuffer::Get_Samples()
{
    return samples;
}
void MSFramebuffer::Create_intFBO(unsigned int attachment)
{
    if (ColorBufs.find(attachment) == ColorBufs.end()) {
        #ifdef DEBUG
        std::cout << ERR_COL_ATTACH_NOT_FOUND("Create_intFBO") << " (attachment == " << attachment << ")" << std::endl;
        #endif
        return;
    }

    if (intFBOs.find(attachment) != intFBOs.end()) {
        if (intFBOs[attachment] != NULL) {
            delete intFBOs[attachment]->ColorBufs[0];
            delete intFBOs[attachment];
        }
    }    

    ColorBuffer *cb = new ColorBuffer(ColorBufs[attachment]->Width, ColorBufs[attachment]->Height,
                                    ColorBufs[attachment]->Type, ColorBufs[attachment]->Get_internal_format(),
                                    0, GL_CLAMP_TO_EDGE);    
    Framebuffer *f = new Framebuffer();
    f->Attach_ColorBuffer(0, cb);    
    f->Set_DrawBuffers({0});

    intFBOs[attachment] = f;
}
unsigned int MSFramebuffer::Get_ResolvedColorBuffer(unsigned int attachment)
{
    if (intFBOs.find(attachment) == intFBOs.end()) {
        #ifdef DEBUG
        std::cout << ERR_INT_FBO_NOT_FOUND("Get_ResolvedColorBuffer") << " (attachment == " << attachment << ")" << std::endl;
        #endif
        return 0;
    }
    GLint binding;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &binding);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, ID);    
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intFBOs[attachment]->ID);    
    glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, ColorBufs[attachment]->Width, ColorBufs[attachment]->Height, 
                      0, 0, ColorBufs[attachment]->Width, ColorBufs[attachment]->Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);    

    glBindFramebuffer(GL_FRAMEBUFFER, binding);
    return intFBOs[attachment]->ColorBufs[0]->ID;
}
std::string MSFramebuffer::Export(std::string dir, ImageType type, unsigned int attachment)
{
    Framebuffer* ifbo = NULL;
    auto it = intFBOs.find(attachment);

    if (it != intFBOs.end())
        ifbo = it->second;
    else {
        #ifdef DEBUG
        std::cerr << ERR_INT_FBO_NOT_FOUND("Export") << " (attachment == " << attachment << ")\n";
        #endif
        return "";
    }

    return ifbo->Export(dir, type, 0);
}
Framebuffer* MSFramebuffer::Get_intFBO(unsigned int attachment)
{
    if (intFBOs.find(attachment) == intFBOs.end())
    {
        #ifdef DEBUG
        std::cerr << ERR_INT_FBO_NOT_FOUND("Get_intFBOs") << " (attachment == " << attachment << ")\n";
        #endif
        return NULL;
    }
    return intFBOs[attachment];
}