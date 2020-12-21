#pragma once
#include <iostream>
#include <glad/glad.h>
#include <vector>
#include <initializer_list>
#include <unordered_map>
#include <string>

namespace glfbo
{
    enum class BufferType : uint8_t
    {
        Texture = 0,
        Renderbuffer = 1,
        Cubemap = 2
    };

    struct vector3
    {
        float x, y, z;
    };
    enum class ImageType : size_t
    {
        PNG = 0,
        JPG = 1
    };
    static const std::string ImageTypeStrings[] = { "png", "jpg" };
    
    std::string GetScreenshotName(ImageType type);
    GLint GetFramebufferBinding();

    class ColorBuffer
    {
    public:
        unsigned int ID;
        const unsigned int Width, Height;
        unsigned int Samples;
        BufferType Type;

        ColorBuffer(unsigned int width, unsigned int height, 
                    BufferType type,
                    GLint internal_format,
                    unsigned int samples,                    
                    GLint wrap_s,
                    GLint wrap_t,
                    GLint wrap_r,
                    vector3 bordercolor);
        ColorBuffer(unsigned int width, unsigned int height, 
                    BufferType type = BufferType::Texture,
                    GLint internal_format = GL_RGB,
                    unsigned int samples = 0,                    
                    GLint wrap_str = GL_CLAMP_TO_EDGE,                    
                    vector3 bordercolor = vector3{1.0f, 1.0f, 1.0f});
        ~ColorBuffer();

        GLenum BindTarget();
        GLint Get_internal_format();
    private:
        GLint int_format;
    };
    class DepthBuffer
    {
    public:
        unsigned int ID;
        const unsigned int Width, Height;
        unsigned int Samples;
        BufferType Type;

        DepthBuffer(unsigned int width, unsigned int height,
                    BufferType type,
                    GLint internal_format,
                    unsigned int samples,
                    GLint wrap_s,
                    GLint wrap_t,
                    GLint wrap_r,
                    vector3 bordercolor);
        DepthBuffer(unsigned int width, unsigned int height,
                    BufferType type = BufferType::Renderbuffer,
                    GLint internal_format = GL_DEPTH_COMPONENT24,
                    unsigned int samples = 0,
                    GLint wrap_str = GL_CLAMP_TO_EDGE,
                    vector3 border_color = vector3{1.0f, 1.0f, 1.0f});
        ~DepthBuffer();

        GLenum BindTarget();
        GLint Get_internal_format();
    private:
        GLint int_format;
    };
    class StencilBuffer
    {
    public:
        unsigned int ID;
        const unsigned int Width, Height;
        unsigned int Samples;
        BufferType Type;
        
        StencilBuffer(unsigned int width, unsigned int height, 
                      BufferType type = BufferType::Renderbuffer,
                      unsigned int samples = 0);

        ~StencilBuffer();

        GLenum BindTarget();
    };
    class DepthStencilBuffer
    {
    public:
        unsigned int ID;
        const unsigned int Width, Height;
        const unsigned int Samples;
        BufferType Type;

        DepthStencilBuffer(unsigned int width, unsigned int height,
                           BufferType type,
                           GLint internal_format,
                           unsigned int samples,
                           GLint wrap_s,
                           GLint wrap_t,
                           GLint wrap_r,
                           vector3 bordercolor);
        DepthStencilBuffer(unsigned int width, unsigned int height,
                           BufferType type = BufferType::Renderbuffer,
                           GLint internal_format = GL_DEPTH24_STENCIL8,
                           unsigned int samples = 0,
                           GLint wrap_str = GL_CLAMP_TO_EDGE,
                           vector3 bordercolor = vector3{1.0f, 1.0f, 1.0f});
        ~DepthStencilBuffer();

        GLenum BindTarget();
        GLint Get_internal_format();
    private:
        GLint int_format;
    };

    class Framebuffer
    {
    protected:
        uint8_t jpg_export_quality = 90;

    public:
        unsigned int ID;                
        std::unordered_map<unsigned int, ColorBuffer*> ColorBufs;
        DepthBuffer *DepthBuf;
        StencilBuffer *StencilBuf;
        DepthStencilBuffer *DepthStencilBuf;

        Framebuffer(std::unordered_map<unsigned int, ColorBuffer*> cBufs, DepthBuffer* dBuf, StencilBuffer* sBuf);
        Framebuffer(unsigned int attachment, ColorBuffer* cBuf, DepthBuffer* dBuf, StencilBuffer* sBuf);
        Framebuffer(unsigned int attachment, ColorBuffer* cBuf, DepthStencilBuffer* dsBuf);
        Framebuffer(std::unordered_map<unsigned int, ColorBuffer*> cBufs, DepthStencilBuffer* dsBuf);
        Framebuffer();
        ~Framebuffer();

        int CheckStatus();
        void Set_ReadDrawBuffer(GLenum mode);
        /* Specify color attachments for drawing */        
        void Set_DrawBuffers(std::initializer_list<unsigned int> indexes);
        /* Returns color buffer ID which is binded to specified color attachment number */
        unsigned int Get_ColorBuffer(unsigned int attachment);
        unsigned int Get_ColorBufferWidth(unsigned int attachment);
        unsigned int Get_ColorBufferHeight(unsigned int attachment);
        void Set_JpgExportQuality(uint8_t quality);
        uint8_t Get_JpgExportQuality();
        virtual void Attach_ColorBuffer(unsigned int attachment, ColorBuffer* buffer);
        virtual void Attach_DepthBuffer(DepthBuffer* buffer);
        virtual void Attach_StencilBuffer(StencilBuffer* buffer);
        virtual void Attach_DepthStencilBuffer(DepthStencilBuffer* buffer);
        virtual std::string Export(std::string dir, ImageType type, unsigned int attachment);
    };
    
    class MSFramebuffer : public Framebuffer
    {
    public:        

        MSFramebuffer(unsigned int samples, std::unordered_map<unsigned int, ColorBuffer*> cBufs, DepthBuffer* dBuf, StencilBuffer* sBuf);
        MSFramebuffer(unsigned int samples, unsigned int attachment, ColorBuffer* cBuf, DepthBuffer* dBuf, StencilBuffer* sBuf);
        MSFramebuffer(unsigned int samples, unsigned int attachment, ColorBuffer* cBuf, DepthStencilBuffer* dsBuf);
        MSFramebuffer(unsigned int samples, std::unordered_map<unsigned int, ColorBuffer*> cBufs, DepthStencilBuffer* dsBuf);
        MSFramebuffer(unsigned int samples);
        ~MSFramebuffer();

        /* Returns number of samples */
        unsigned int Get_Samples();        
        /* Resolve color buffer binded to specified attachment and returns resolved color buffer ID. */
        unsigned int Get_ResolvedColorBuffer(unsigned int attachment);
        /* Creates intermediate Framebuffer object for specified color attachment. */        
        void Create_intFBO(unsigned int attachment);
        /* Returns pointer to intermediate framebuffer of specified color attachment. */
        Framebuffer* Get_intFBO(unsigned int attachment);
        void Attach_ColorBuffer(unsigned int attachment, ColorBuffer* buffer) override;     
        void Attach_DepthBuffer(DepthBuffer* buffer) override;
        void Attach_StencilBuffer(StencilBuffer* buffer) override;
        void Attach_DepthStencilBuffer(DepthStencilBuffer* buffer) override;
        std::string Export(std::string dir, ImageType type, unsigned int attachment) override;

    private:
        std::unordered_map<unsigned int, Framebuffer*> intFBOs;
        unsigned int samples;

        void handleSamples(unsigned int s);
    };

}