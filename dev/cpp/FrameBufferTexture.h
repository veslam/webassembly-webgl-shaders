class FrameBufferTexture {

public:
	FrameBufferTexture(int w, int h);
	~FrameBufferTexture();

	void useFrameBuffer();
	void useTexture();

private:
	int width;
	int height;

	GLuint texture;
	GLuint framebuffer;
};



class SwapFrameBufferTexture {

public:
	SwapFrameBufferTexture(int w, int h);
	~SwapFrameBufferTexture();

	FrameBufferTexture* getToRead();
	FrameBufferTexture* getToWrite();
	void swap();

	void setFuncRenderBase(std::function<void(void)> func);

private:
	FrameBufferTexture* items[2];
	int newerIdx = -1;

	std::function<void(void)> pfunc;
	void renderBase();
};