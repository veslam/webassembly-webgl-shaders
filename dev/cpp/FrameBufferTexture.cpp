// (createFrameBuffer)

#include "FrameBufferTexture.h"
// #include "GLUtils.cpp"

FrameBufferTexture::FrameBufferTexture(int w, int h) {
	width = w;
	height = h;

    texture = 0;
    framebuffer = 0;

    glGenFramebuffers(1, &framebuffer);
    glGenTextures(1, &texture);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

}

FrameBufferTexture::~FrameBufferTexture() {
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteTextures(1, &texture);
	texture = 0;
	framebuffer = 0;
}

void FrameBufferTexture::useFrameBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glViewport(0, 0, width, height);
}

void FrameBufferTexture::useTexture() {
	glBindTexture(GL_TEXTURE_2D, texture);
}





SwapFrameBufferTexture::SwapFrameBufferTexture(int w, int h) {
	items[0] = new FrameBufferTexture(w, h);
	items[1] = new FrameBufferTexture(w, h);
}

SwapFrameBufferTexture::~SwapFrameBufferTexture() {}

FrameBufferTexture* SwapFrameBufferTexture::getToRead() {
	if (-1 == newerIdx) {
		return items[0];
	} else {
		return items[newerIdx];
	}
}

FrameBufferTexture* SwapFrameBufferTexture::getToWrite() {
	if (-1 == newerIdx) {
		return items[0];
	} else {
		return items[1 - newerIdx];
	}
}
void SwapFrameBufferTexture::swap() {
	if (-1 == newerIdx) {
		newerIdx = 0;
	} else {
		newerIdx = 1 - newerIdx;
	}

	items[1 - newerIdx]->useFrameBuffer();
	glActiveTexture(GL_TEXTURE0);
	items[newerIdx]->useTexture();
	renderBase();
}

void SwapFrameBufferTexture::setFuncRenderBase(std::function<void(void)> func) {
	pfunc = func;
}

void SwapFrameBufferTexture::renderBase() {
	pfunc();
}
