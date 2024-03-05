#pragma once
#include "engine/graphics/sharedBuffer.h"
#include "engine/graphics/graphicsDevice.h"
#include "engine/graphics/rawTexture.h"

typedef struct _renderBuffer* RenderBuffer;

struct _renderBuffer {
	// the underlying render buffer handle for the graphics device
	SharedHandle Handle;
	/// <summary>
	/// The format for this buffer
	/// </summary>
	TextureFormat Format;
	/// <summary>
	/// The width of the buffer
	/// </summary>
	ulong Width;
	/// <summary>
	/// The height of the buffer
	/// </summary>
	ulong Height;
};

struct _renderBufferMethods {
	RenderBuffer (*Create)(ulong width, ulong height, TextureFormat format);
	RenderBuffer(*Instance)(RenderBuffer);
	void (*Dispose)(RenderBuffer);
};

extern const struct _renderBufferMethods RenderBuffers;