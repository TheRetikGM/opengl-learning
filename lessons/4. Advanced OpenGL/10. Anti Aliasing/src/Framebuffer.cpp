#include "Framebuffer.h"
#include <glad/glad.h>
#include <iostream>
#include <vector>

#define RED		"\x1B[31m"
#define GREEN	"\x1B[32m"
#define DEFAULT	"\033[0m"
#define ERROR	RED "[ERROR]" DEFAULT

Framebuffer::Framebuffer(const int& width, const int& height)
{
	glGenFramebuffers(1, &ID);
	glBindFramebuffer(GL_FRAMEBUFFER, ID);

	glGenTextures(1, &ColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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
void		 Framebuffer::destroy()
{
	glDeleteRenderbuffers(1, &DepthStencilBuffer);
	glDeleteTextures(1, &ColorBuffer);
	glDeleteFramebuffers(1, &ID);
}
int			 Framebuffer::checkStatus()
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
void		 Framebuffer::UpdateSize(const unsigned int& newWidth, const unsigned int& newHeight)
{
	if (!checkBind())
		glBindFramebuffer(GL_FRAMEBUFFER, ID);

	glBindTexture(GL_TEXTURE_2D, ColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, newWidth, newHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDeleteRenderbuffers(1, &DepthStencilBuffer);
	glGenRenderbuffers(1, &DepthStencilBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, DepthStencilBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, newWidth, newHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, DepthStencilBuffer);
}
void		 Framebuffer::Use()
{
	glBindFramebuffer(GL_FRAMEBUFFER, ID);	
}
void		 Framebuffer::UseDefault()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void	   	 Framebuffer::clearBuffers(float r, float g, float b, float a)
{
	if (!checkBind())
		glBindFramebuffer(GL_FRAMEBUFFER, ID);
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);	
}
void		 Framebuffer::clearColorBuffer(float r, float g, float b, float a)
{
	if (!checkBind())
		glBindFramebuffer(GL_FRAMEBUFFER, ID);
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);	
}
void		 Framebuffer::clearDepthBuffer()
{
	if (!checkBind())
		glBindFramebuffer(GL_FRAMEBUFFER, ID);
	glClear(GL_DEPTH_BUFFER_BIT);	
}
void		 Framebuffer::clearStencilBuffer()
{
	if (!checkBind())
		glBindFramebuffer(GL_FRAMEBUFFER, ID);
	glClear(GL_STENCIL_BUFFER_BIT);	
}
void		 Framebuffer::clearDepthStencilBuffer()
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
bool		 Framebuffer::checkBind()
{
	int id = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &id);
	if (id == ID)
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
void		 MSFramebuffer::UpdateSize(const unsigned int& newWidth, const unsigned int& newHeight)
{
	if (!checkBind())
		glBindFramebuffer(GL_FRAMEBUFFER, ID);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ColorBuffer);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Nsamples, GL_RGB, newWidth, newHeight, GL_TRUE);
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

void		 Framebuffer::clearAllBuffers(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
