#include "Framebuffer.h"
#include <glad/glad.h>
#include <iostream>
#include <vector>

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
		std::cerr << "FRAMEBUFFER::ERROR Could not create framebuffer object." << std::endl;
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	this->width = width;
	this->height = height;
}
Framebuffer::~Framebuffer()
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
		fprintf(stderr, "Framebuffer.h: [ERROR] code: %#04x\n", (int)status);
		return -1;
		break;
	}	
}

/* updates width and height of buffers */
void Framebuffer::UpdateSize(const unsigned int& newWidth, const unsigned int& newHeight)
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
/* sets opengl to use this framebuffer */
void Framebuffer::Use()
{
	glBindFramebuffer(GL_FRAMEBUFFER, ID);	
}
/* sets opengl to use default buffer */
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
	if (id == ID)
		return true;
	return false;
}
