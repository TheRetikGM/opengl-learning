#include <iostream>
#include "glfbo.h"
#include "DebugColors.h"
#include "config.h"
#include <stb_image_write.h>

using namespace glfbo;

Framebuffer::Framebuffer(std::unordered_map<unsigned int, ColorBuffer*> cBufs, DepthBuffer* dBuf, StencilBuffer* sBuf) : DepthBuf(NULL), StencilBuf(NULL), DepthStencilBuf(NULL), ID(0)
{    
    glGenFramebuffers(1, &ID);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);

    if (cBufs.size() == 0 && !dBuf && !sBuf) {
        #ifdef DEBUG
        std::cout << DC_WARNING << " Framebuffer::Framebuffer: No buffers specified. Framebuffer should have at least 1 buffer attached.\n";
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
Framebuffer::Framebuffer(unsigned int attachment, ColorBuffer* cBuf, DepthBuffer* dBuf, StencilBuffer* sBuf) : DepthBuf(NULL), StencilBuf(NULL), DepthStencilBuf(NULL), ID(0)
{
    glGenFramebuffers(1, &ID);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
    
    if (!cBuf && !dBuf && !sBuf) {
        #ifdef DEBUG
        std::cout << DC_WARNING << " Framebuffer::Framebuffer: No buffers specified. Framebuffer should have at least 1 buffer attached.\n";
        #endif
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    if (cBuf)
        Attach_ColorBuffer(attachment, cBuf);
    if (dBuf)
        Attach_DepthBuffer(dBuf);
    if (sBuf)
        Attach_StencilBuffer(sBuf);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
Framebuffer::Framebuffer(std::unordered_map<unsigned int, ColorBuffer*> cBufs, DepthStencilBuffer* dsBuf) : DepthBuf(NULL), StencilBuf(NULL), DepthStencilBuf(NULL), ID(0)
{
    glGenFramebuffers(1, &ID);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
    
    if (cBufs.size() == 0 && !dsBuf) {
        #ifdef DEBUG
        std::cout << DC_WARNING << " Framebuffer::Framebuffer: No buffers specified. Framebuffer should have at least 1 buffer attached.\n";
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
Framebuffer::Framebuffer(unsigned int attachment, ColorBuffer* cBuf, DepthStencilBuffer* dsBuf) : DepthBuf(NULL), StencilBuf(NULL), DepthStencilBuf(NULL), ID(0)
{    
    glGenFramebuffers(1, &ID);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
    
    if (!cBuf && !dsBuf) {
        #ifdef DEBUG
        std::cout << DC_WARNING << " Framebuffer::Framebuffer: No buffers specified. Framebuffer should have at least 1 buffer attached.\n";
        #endif
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    if (cBuf)
        Attach_ColorBuffer(attachment, cBuf);
    if (dsBuf)
        Attach_DepthStencilBuffer(dsBuf);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
Framebuffer::Framebuffer() : DepthBuf(NULL), StencilBuf(NULL), DepthStencilBuf(NULL), ID(0)
{
    glGenFramebuffers(1, &ID);
}
Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &ID);
}

int Framebuffer::CheckStatus()
{
    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	switch (status)
	{
	case GL_FRAMEBUFFER_COMPLETE:
		return 0;
		break;
	default:
        #ifdef DEBUG
		fprintf(stderr, "Framebuffer::Complete(): " DC_ERROR " code: %#04x\n", (int)status);
        #endif
		return status;		
	}
}
void Framebuffer::Set_ReadDrawBuffer(GLenum mode)
{
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
    glDrawBuffer(mode);
    glReadBuffer(mode);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Framebuffer::Set_DrawBuffers(std::initializer_list<unsigned int> indexes)
{       
    GLint maxDrawBuf = 0;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuf);
    
    if (indexes.size() > maxDrawBuf) {
        #ifdef DEBUG
        std::cout << DC_ERROR << " Framebuffer::Set_DrawBuffers(): Couldn't set glDrawBuffers. Number of indexes > GL_MAX_DRAW_BUFFERS.\n";
        #endif
    }
    else {
        GLenum *arr = new GLenum[indexes.size()];

        auto j = indexes.begin();
        for (int i = 0; i < indexes.size(); i++, j++)        
            arr[i] = GL_COLOR_ATTACHMENT0 + *j;

        glBindFramebuffer(GL_FRAMEBUFFER, ID);     
        glDrawBuffers(indexes.size(), (const GLenum*) arr);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        delete[] arr;
    }
}
void Framebuffer::Attach_ColorBuffer(unsigned int attachment, ColorBuffer* buffer)
{    
    GLint maxAttach = 0;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);
    if (attachment > maxAttach) {
        #ifdef DEBUG
        std::cout << DC_ERROR << " Framebuffer::Attach_ColorBuffer(): Color attachment " << attachment << " > GL_MAX_COLOR_ATTACHMENTS. Color buffer wasn't attached.\n";
        #endif
        return;
    }
    else if (buffer != NULL) {                
        if (buffer->ID == 0) {
            #ifdef DEBUG
            std::cout << DC_ERROR << " Cannot attach incomplete ColorBuffer at color attachment " << ColorBufs.size() << ".\n";
            #endif            
            return;
        }
        if (buffer->Samples != 0) {
            #ifdef DEBUG
            std::cout << DC_ERROR << " Framebuffer::Attach_ColorBuffer(): Could not attach multisampled color buffer at color attachment " << ColorBufs.size() << "\n";
            #endif                        
        }        
        ColorBufs[attachment] = buffer;        
        glBindFramebuffer(GL_FRAMEBUFFER, ID);     
        switch (buffer->Type)
        {
            case BufferType::Texture:                
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_TEXTURE_2D, buffer->ID, 0);                    
                break;
            case BufferType::Renderbuffer:
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_RENDERBUFFER, buffer->ID);
                break;
            case BufferType::Cubemap:
                glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, buffer->ID, 0);
                break;
        }
    }
    else {
        #ifdef DEBUG
        std::cout << DC_WARNING << " Framebuffer::Attach_ColorBuffer(): ColorBuffer wasn't attached. (buffer == NULL)\n";
        #endif
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Framebuffer::Attach_DepthBuffer(DepthBuffer* buffer)
{    
    DepthBuf = buffer;    
    if (DepthBuf != NULL)
    {
        if (DepthBuf->ID == 0) {
            #ifdef DEBUG
            std::cout << DC_ERROR << " Cannot attach uncomplete DepthBuffer. ( DepthBuffer::ID == 0 )\n";
            #endif
            return;
        }
        if (DepthBuf->Samples == 0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, ID);
            if (DepthBuf->Type == BufferType::Texture)
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthBuf->ID, 0);
            else if (DepthBuf->Type == BufferType::Renderbuffer)
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, DepthBuf->ID);
            else if (DepthBuf->Type == BufferType::Cubemap)
                glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, DepthBuf->ID, 0);
        }
        else {
        #ifdef DEBUG
            std::cout << DC_ERROR << " Framebuffer::Attach_DepthBuffer(): Cannot attach multisampled depth buffer to Framebuffer. For multisampled framebuffers use MSFramebuffer.\n";
        #endif     
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Framebuffer::Attach_StencilBuffer(StencilBuffer* buffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
    StencilBuf = buffer;
    if (StencilBuf != NULL)
    {
        if (StencilBuf->ID == 0) {
            #ifdef DEBUG
            std::cout << DC_ERROR << " Cannot attach uncomplete StencilBuffer. ( StencilBuffer::ID == 0 )\n";
            #endif
            return;
        }
        if (StencilBuf->Samples == 0) {
            if (StencilBuf->Type == BufferType::Renderbuffer)
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, StencilBuf->ID);
            else if (StencilBuf->Type == BufferType::Texture)
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, StencilBuf->ID, 0);
            else {
                #ifdef DEBUG
                std::cout << DC_ERROR << " Framebuffer::Attach_StencilBuffer(): StencilBuffer not attached. Wrong buffer type.\n";
                #endif
            }            
        }
        else {
            #ifdef DEBUG
            std::cout << DC_ERROR " Framebuffer::Attach_StencilBuffer(): Cannot attach multisampled stencil buffer to Framebuffer. For multisampled framebuffer use MSFramebuffer.\n";
            #endif
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Framebuffer::Attach_DepthStencilBuffer(DepthStencilBuffer* buffer)
{    
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
    DepthStencilBuf = buffer;
    if (DepthStencilBuf != NULL)
    {
        if (DepthStencilBuf->ID == 0) {
            #ifdef DEBUG
            std::cout << DC_ERROR << " Cannot attach uncomplete DepthStencilBuffer. ( DepthStencilBuffer::ID == 0 )\n";
            #endif
            return;
        }
        if (DepthStencilBuf->Samples == 0) {
            if (DepthStencilBuf->Type == BufferType::Texture)
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, DepthStencilBuf->BindTarget(), DepthStencilBuf->ID, 0);
            else if (DepthStencilBuf->Type == BufferType::Renderbuffer)
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, DepthStencilBuf->ID);
            else {
                #ifdef DEBUG
                std::cout << DC_ERROR << " Framebuffer::Attach_DepthStencilBuffer(): DepthStencilBuffer not attached. Wrong buffer type.\n";
                #endif
            }
        }
        else {
            #ifdef DEBUG
            std::cout << DC_ERROR " Framebuffer::Attach_DepthStencilBuffer(): Cannot attach multisampled depth and stencil buffer to Framebuffer. For multisampled framebuffer use MSFramebuffer.\n";
            #endif
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
unsigned int Framebuffer::Get_ColorBuffer(unsigned int attachment)
{
    return ColorBufs[attachment]->ID;
}
unsigned int Framebuffer::Get_ColorBufferWidth(unsigned int attachment)
{
    return ColorBufs[attachment]->Width;
}
unsigned int Framebuffer::Get_ColorBufferHeight(unsigned int attachment)
{
    return ColorBufs[attachment]->Height;
}
std::string Framebuffer::Export(std::string dir, ImageType type, unsigned int attachment)
{
    uint8_t channels = 0;
    GLenum format;
    switch (type) {
        case ImageType::PNG: channels = 4; format = GL_RGBA; break;
        case ImageType::JPG: channels = 3; format = GL_RGB;  break;
    }
    unsigned int width = Get_ColorBufferWidth(attachment);
    unsigned int height = Get_ColorBufferHeight(attachment);

    GLint binding = GetFramebufferBinding();
    glBindFramebuffer(GL_FRAMEBUFFER, ID);

    unsigned char *data = new unsigned char[width * height * channels];
    glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment);
    glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
    stbi_flip_vertically_on_write(1);

    if (dir.length() != 0 && *(dir.end() - 1) != '/')
        dir = dir + "/";
        
    std::string img_name = dir + GetScreenshotName(type);

    switch (type) {
        case ImageType::PNG: stbi_write_png(img_name.c_str(), width, height, channels, (const void*)data, width * channels);
        case ImageType::JPG: stbi_write_jpg(img_name.c_str(), width, height, channels, (const void*)data, jpg_export_quality);
    }

    delete[] data;
    glBindFramebuffer(GL_FRAMEBUFFER, binding);

    return img_name;
}
void Framebuffer::Set_JpgExportQuality(uint8_t quality)
{
    jpg_export_quality = quality;
}
uint8_t Framebuffer::Get_JpgExportQuality()
{
    return jpg_export_quality;
}