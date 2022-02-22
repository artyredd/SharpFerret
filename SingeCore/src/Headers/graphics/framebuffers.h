#pragma once

#pragma once
#include "graphics/sharedBuffer.h"
#include "graphics/graphicsDevice.h"
#include "graphics/texture.h"
#include "graphics/renderbuffers.h"

typedef struct _frameBuffer* FrameBuffer;

struct _frameBuffer {
	// the underlying render buffer handle for the graphics device
	SharedHandle Handle;
	/// <summary>
	/// The attached texture, if present
	/// </summary>
	Texture Texture;
	/// <summary>
	/// The attached render buffer, if present
	/// </summary>
	RenderBuffer RenderBuffer;
};

struct _frameBufferMethods {
	/// <summary>
	/// The default framebuffer, when used all draw, read, write calls go directly to the graphics device
	/// </summary>
	const FrameBuffer Default;
	FrameBuffer(*Create)(void);
	// Set the provided render buffer as the buffer that should be used 
	// if null is provided the default framebuffer is used
	void (*Use)(FrameBuffer);
	void (*AttachTexture)(FrameBuffer, Texture, unsigned int offset);
	void (*AttachRenderBuffer)(FrameBuffer, RenderBuffer);
	void (*Dispose)(FrameBuffer);
};

extern const struct _frameBufferMethods FrameBuffers;