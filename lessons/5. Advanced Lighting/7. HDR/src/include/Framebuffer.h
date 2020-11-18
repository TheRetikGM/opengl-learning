#pragma once
#include <string>
#include <glad/glad.h>
#include "config.h"
class Framebuffer
{
public:
	/* id of this framebuffer */
	unsigned int ID;

	Framebuffer(const int& width, const int& height, const GLint& internal_format = GL_RGBA);
	virtual ~Framebuffer();

	/* updates width and height of buffers */
	virtual void UpdateSize(const unsigned int& newWidth, const unsigned int& newHeight);
	/* sets opengl to use this framebuffer */
	void Use();
	/* sets opengl to use default buffer */
	void UseDefault();
	void clearBuffers(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
	void clearColorBuffer(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
	void clearDepthBuffer();
	void clearStencilBuffer();
	void clearDepthStencilBuffer();
	virtual unsigned int getWidth();
	virtual unsigned int getHeight();
	virtual unsigned int getColorBuffer();
	virtual unsigned int getDepthSnectilBuffer();
	static std::string getScreenshotName();
	/* saves image of framebuffers color buffer */
	virtual std::string screenshot(std::string path = BINARY_DIR);

	static void clearAllBuffers(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
	/* saves image of default framebuffer */
	static std::string screenshot_defaultFBO(unsigned int width, unsigned int height, std::string path = BINARY_DIR);

protected:
	Framebuffer() : width(0), height(0), ColorBuffer(0), ID(0), DepthStencilBuffer(0) {}
	unsigned int width;
	unsigned int height;

	/* opengl texture2D */
	unsigned int ColorBuffer;
	/* renderbuffer depth 24 bits stencil 8 bits */
	unsigned int DepthStencilBuffer;

	virtual bool checkBind();
	virtual void destroy();
};

/* Multisampled Framebuffer */
class MSFramebuffer : public Framebuffer
{
public:
	/* Number of samples */
	const unsigned int Nsamples;

	MSFramebuffer(const int& width, const int& height, const unsigned int samples = 2, const GLint& internal_format = GL_RGBA);
	virtual ~MSFramebuffer();

	std::string screenshot(std::string = BINARY_DIR) override;
	void UpdateSize(const unsigned int& newWidth, const unsigned int& newHeight) override;
	/* Returns resolved color buffer */
	unsigned int getColorBuffer() override;
	unsigned int getDepthSnectilBuffer() override;
	/* Reurns unresolved colorbuffer. ( with multisample informations ) */
	unsigned int getColorBufferMS();	
protected:
	MSFramebuffer() : Nsamples(0), intermediateFBO(NULL) {}
private:
	Framebuffer* intermediateFBO;
};

struct Resolution
{
	unsigned int Width;
	unsigned int Height;
};
class DepthmapFramebuffer
{
public:
	unsigned int ID;
	Resolution resolution;

	DepthmapFramebuffer(const Resolution& resolution);
	DepthmapFramebuffer(const unsigned int& width, const unsigned int& height);
	~DepthmapFramebuffer();

	void Use() const;
	void Use(GLenum target) const;	
	unsigned int GetDepthTexture() const;
	void ClearDepthBuffer() const;
private:	
	unsigned int depth_texture;
};

class DepthCubeFramebuffer
{
	unsigned int depthCubemap;

	void init();
	
public:
	unsigned int ID;
	const int Width, Height;

	DepthCubeFramebuffer(int width, int height);
	~DepthCubeFramebuffer();

	unsigned int getDepthCubemap() const;
	void clearDepth() const;
	void Use() const;
};