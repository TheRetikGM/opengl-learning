#include "Framebuffer.h"
#include <glad/glad.h>
#include <iostream>
#include <vector>
#include "DebugColors.h"
#include <stb_image_write.h>
#include <ctime>

Framebuffer::Framebuffer(const int& width, const int& height)
{
	glGenFramebuffers(1, &ID);
	glBindFramebuffer(GL_FRAMEBUFFER, ID);

	glGenTextures(1, &ColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ColorBuffer, 0);

	glGenRenderbuffers(1, &DepthStencilBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, DepthStencilBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, DepthStencilBuffer);

	if (checkStatus() != 0)
	{
		glDeleteRenderbuffers(1, &DepthStencilBuffer);
		glDeleteTextures(1, &ColorBuffer);
		glDeleteFramebuffers(1, &ID);
		std::cerr << "Framebuffer.h: " ERROR " Could not create framebuffer object." << std::endl;
		ID = 0;
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	this->width = width;
	this->height = height;
}
Framebuffer::~Framebuffer()
{
	destroy();
}
void Framebuffer::destroy()
{
	glDeleteRenderbuffers(1, &DepthStencilBuffer);
	glDeleteTextures(1, &ColorBuffer);
	glDeleteFramebuffers(1, &ID);
}
int Framebuffer::checkStatus()
{
	if (!checkBind())
		glBindFramebuffer(GL_FRAMEBUFFER, ID);
	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);	

	switch (status)
	{
	case GL_FRAMEBUFFER_COMPLETE:
		return 0;
		break;
	default:
		fprintf(stderr, "Framebuffer.h: " ERROR " code: %#04x\n", (int)status);
		return -1;
		break;
	}	
}
void Framebuffer::UpdateSize(const unsigned int& newWidth, const unsigned int& newHeight)
{
	if (!checkBind())
		glBindFramebuffer(GL_FRAMEBUFFER, ID);

	glBindTexture(GL_TEXTURE_2D, ColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newWidth, newHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDeleteRenderbuffers(1, &DepthStencilBuffer);
	glGenRenderbuffers(1, &DepthStencilBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, DepthStencilBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, newWidth, newHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, DepthStencilBuffer);
}
void Framebuffer::Use()
{
	glBindFramebuffer(GL_FRAMEBUFFER, ID);	
}
void Framebuffer::UseDefault()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Framebuffer::clearBuffers(float r, float g, float b, float a)
{
	if (!checkBind())
		glBindFramebuffer(GL_FRAMEBUFFER, ID);
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);	
}
void Framebuffer::clearColorBuffer(float r, float g, float b, float a)
{
	if (!checkBind())
		glBindFramebuffer(GL_FRAMEBUFFER, ID);
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);	
}
void Framebuffer::clearDepthBuffer()
{
	if (!checkBind())
		glBindFramebuffer(GL_FRAMEBUFFER, ID);
	glClear(GL_DEPTH_BUFFER_BIT);	
}
void Framebuffer::clearStencilBuffer()
{
	if (!checkBind())
		glBindFramebuffer(GL_FRAMEBUFFER, ID);
	glClear(GL_STENCIL_BUFFER_BIT);	
}
void Framebuffer::clearDepthStencilBuffer()
{
	if (!checkBind())
		glBindFramebuffer(GL_FRAMEBUFFER, ID);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);	
}
unsigned int Framebuffer::getWidth()
{
	return width;
}
unsigned int Framebuffer::getHeight()
{
	return height;
}
bool Framebuffer::checkBind()
{
	int id = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &id);
	if ((unsigned int)id == ID)
		return true;
	return false;
}
unsigned int Framebuffer::getColorBuffer()
{
	return ColorBuffer;
}
unsigned int Framebuffer::getDepthSnectilBuffer()
{
	return DepthStencilBuffer;
}
void Framebuffer::screenshot()
{	
	int id = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &id);	

	unsigned char *data = new unsigned char[width * height * 4];
	this->Use();
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_flip_vertically_on_write(1);
	stbi_write_png(getScreenshotName().c_str(), width, height, 4, data, 4 * width);
	delete data;

	glBindFramebuffer(GL_FRAMEBUFFER, (unsigned int)id);
}

MSFramebuffer::MSFramebuffer(const int& width, const int& height, const unsigned int samples) 
	: Nsamples(samples), intermediateFBO(NULL)
{
	glGenFramebuffers(1, &ID);
	glBindFramebuffer(GL_FRAMEBUFFER, ID);

	glGenTextures(1, &ColorBuffer);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ColorBuffer);	
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, ColorBuffer, 0);	

	glGenRenderbuffers(1, &DepthStencilBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, DepthStencilBuffer);	
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, DepthStencilBuffer);

	if (checkStatus() != 0)
	{
		glDeleteRenderbuffers(1, &DepthStencilBuffer);
		glDeleteTextures(1, &ColorBuffer);
		glDeleteFramebuffers(1, &ID);
		std::cerr << "Framebuffer.h: " ERROR " Could not create multisampled framebuffer object." << std::endl;
		ID = 0;
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	this->width = width;
	this->height = height;

	intermediateFBO = new Framebuffer(width, height);
}
MSFramebuffer::~MSFramebuffer()
{
	destroy();
	delete intermediateFBO;
}
void MSFramebuffer::screenshot()
{
	int id = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &id);	

	unsigned char *data = new unsigned char[width * height * 4];
	getColorBuffer();
	intermediateFBO->Use();
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_flip_vertically_on_write(1);
	stbi_write_png(getScreenshotName().c_str(), width, height, 4, data, 4 * width);
	delete data;

	glBindFramebuffer(GL_FRAMEBUFFER, (unsigned int)id);
}
void MSFramebuffer::UpdateSize(const unsigned int& newWidth, const unsigned int& newHeight)
{
	if (!checkBind())
		glBindFramebuffer(GL_FRAMEBUFFER, ID);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ColorBuffer);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Nsamples, GL_RGBA, newWidth, newHeight, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	glDeleteRenderbuffers(1, &DepthStencilBuffer);
	glGenRenderbuffers(1, &DepthStencilBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, DepthStencilBuffer);	
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, Nsamples, GL_DEPTH24_STENCIL8, newWidth, newHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, DepthStencilBuffer);
}
unsigned int MSFramebuffer::getColorBuffer()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, ID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO->ID);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	return intermediateFBO->getColorBuffer();
}
unsigned int MSFramebuffer::getDepthSnectilBuffer()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, ID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO->ID);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	return intermediateFBO->getDepthSnectilBuffer();
}
unsigned int MSFramebuffer::getColorBufferMS()
{
	return ColorBuffer;
}

void Framebuffer::clearAllBuffers(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
std::string Framebuffer::getScreenshotName()
{
	time_t tim = time(NULL);
	struct tm *ltm = localtime(&tim);
	
	char buf[50];
	snprintf(buf, sizeof(buf), "screenshot_%02i-%02i-%i_%02i%02i%02i.png",
	 ltm->tm_mon + 1, ltm->tm_mday, ltm->tm_year + 1900, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
	return std::string(buf);
}
void Framebuffer::screenshot_defaultFBO(unsigned int width, unsigned int height)
{
	int id = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &id);

	unsigned char *data = new unsigned char[width * height * 4];
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_flip_vertically_on_write(1);
	stbi_write_png(getScreenshotName().c_str(), width, height, 4, data, 4 * width);
	delete data;

	glBindFramebuffer(GL_FRAMEBUFFER, (unsigned int)id);
}