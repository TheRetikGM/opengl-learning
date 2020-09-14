#pragma once
class Framebuffer
{
public:
	/* opengl texture2D */
	unsigned int ColorBuffer;
	/* renderbuffer depth 24 bits stencil 8 bits */
	unsigned int DepthStencilBuffer;
	/* id of this framebuffer */
	unsigned int ID;

	Framebuffer(const int& width, const int& height);
	~Framebuffer();

	/* updates width and height of buffers */
	void UpdateSize(const unsigned int& newWidth, const unsigned int& newHeight);
	/* sets opengl to use this framebuffer */
	void Use();
	/* sets opengl to use default buffer */
	void UseDefault();
	void clearBuffers(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
	void clearColorBuffer(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
	void clearDepthBuffer();
	void clearStencilBuffer();
	void clearDepthStencilBuffer();
	unsigned int getWidth();
	unsigned int getHeight();

private:
	unsigned int width;
	unsigned int height;	

	int checkStatus();
	bool checkBind();
};

