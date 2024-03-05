#pragma once
#include "engine/graphics/sharedBuffer.h"
#include "engine/graphics/graphicsDevice.h"
#include "engine/graphics/rawTexture.h"
#include "engine/graphics/renderbuffers.h"

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
	RawTexture Texture;
	/// <summary>
	/// The attached render buffer, if present
	/// </summary>
	RenderBuffer RenderBuffer;
	/// <summary>
	/// The width of this frame buffer
	/// </summary>
	ulong Width;
	/// <summary>
	/// The height of this frame buffer
	/// </summary>
	ulong Height;
	/// <summary>
	/// The mask that is used to clear the frame buffer when .Clear() or .ClearThenUse() is called, this is normall set when
	/// you create the framebuffer
	/// </summary>
	unsigned int ClearMask;
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
	void (*Clear)(FrameBuffer);
	void (*ClearAndUse)(FrameBuffer);
	void (*AttachTexture)(FrameBuffer, RawTexture, unsigned int offset);
	void (*AttachRenderBuffer)(FrameBuffer, RenderBuffer);
	void (*Dispose)(FrameBuffer);
};

extern const struct _frameBufferMethods FrameBuffers;