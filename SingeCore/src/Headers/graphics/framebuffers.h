#pragma once
#include "graphics/sharedBuffer.h"
#include "graphics/graphicsDevice.h"
#include "graphics/texture.h"
#include "graphics/renderbuffers.h"

typedef unsigned int FrameBufferType;

struct _frameBufferTypes {
	/// <summary>
	/// Neither reads or draws should affect the frame buffer
	/// </summary>
	FrameBufferType None;
	/// <summary>
	/// Both reads and draws should affect the frame buffer
	/// </summary>
	FrameBufferType Default;
	/// <summary>
	/// Only Reads should affect the frame buffer
	/// </summary>
	FrameBufferType Read;
	/// <summary>
	/// Only draws should affect the frame buffer
	/// </summary>
	FrameBufferType Draw;
};

extern const struct _frameBufferTypes FrameBufferTypes;

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
	/// <summary>
	/// The width of this frame buffer
	/// </summary>
	size_t Width;
	/// <summary>
	/// The height of this frame buffer
	/// </summary>
	size_t Height;
};

struct _frameBufferMethods {
	/// <summary>
	/// The default framebuffer, when used all draw, read, write calls go directly to the graphics device
	/// </summary>
	FrameBuffer Default;
	FrameBuffer(*Create)(FrameBufferType type);
	// Set the provided render buffer as the buffer that should be used 
	// if null is provided the default framebuffer is used
	void (*Use)(FrameBuffer);
	void (*AttachTexture)(FrameBuffer, Texture, unsigned int offset);
	void (*AttachRenderBuffer)(FrameBuffer, RenderBuffer);
	void (*Dispose)(FrameBuffer);
};

extern const struct _frameBufferMethods FrameBuffers;