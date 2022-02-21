#pragma once
#include "graphics/sharedBuffer.h"
#include "graphics/graphicsDevice.h"
#include "graphics/texture.h"

typedef struct _renderBuffer* RenderBuffer;

struct _renderBuffer {
	// the underlying render buffer handle for the graphics device
	SharedHandle Handle;
	/// <summary>
	/// The format for this buffer
	/// </summary>
	TextureFormat Format;
};

struct _renderBufferMethods {
	RenderBuffer (*Create)(size_t width, size_t height, TextureFormat format);
	RenderBuffer(*Instance)(RenderBuffer);
	void (*Dispose)(RenderBuffer);
};

extern const struct _renderBufferMethods RenderBuffers;