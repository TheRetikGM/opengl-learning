#pragma once
class Framebuffer
{
public:
	/* id of this framebuffer */
	unsigned int ID;

	Framebuffer(const int& width, const int& height);
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

	static void clearAllBuffers(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);

protected:
	Framebuffer() : width(0), height(0), ColorBuffer(0), ID(0), DepthStencilBuffer(0) {}
	unsigned int width;
	unsigned int height;

	/* opengl texture2D */
	unsigned int ColorBuffer;
	/* renderbuffer depth 24 bits stencil 8 bits */
	unsigned int DepthStencilBuffer;

	virtual int checkStatus();
	virtual bool checkBind();
	virtual void destroy();
};

/* Multisampled Framebuffer */
class MSFramebuffer : public Framebuffer
{
public:
	/* Number of samples */
	const unsigned int Nsamples;

	MSFramebuffer(const int& width, const int& height, const unsigned int samples);
	virtual ~MSFramebuffer();

	void UpdateSize(const unsigned int& newWidth, const unsigned int& newHeight) override;
	/* Returns resolved color buffer */
	unsigned int getColorBuffer() override;
	unsigned int getDepthSnectilBuffer() override;
	/* Reurns unresolved colorbuffer. ( with multisample informations ) */
	unsigned int getColorBufferMS();
protected:
	MSFramebuffer() : Nsamples(0) {}
private:	
	Framebuffer* intermediateFBO;
};