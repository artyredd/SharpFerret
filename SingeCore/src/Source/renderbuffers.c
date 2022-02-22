#include "graphics/renderbuffers.h"
#include "singine/memory.h"

static void Dispose(RenderBuffer);
static RenderBuffer Create(size_t width, size_t height, TextureFormat format);
static RenderBuffer Instance(RenderBuffer);

const struct _renderBufferMethods RenderBuffers = {
	.Dispose = &Dispose,
	.Create  = &Create,
	.Instance = &Instance
};

static void OnDispose(RenderBuffer state)
{
	GraphicsDevice.DeleteRenderBuffer(state->Handle->Handle);
}

static void Dispose(RenderBuffer buffer)
{
	if (buffer isnt null)
	{
		SharedHandles.Dispose(buffer->Handle, buffer, OnDispose);
	}
	
	SafeFree(buffer);
}

static RenderBuffer Create(size_t width, size_t height, TextureFormat format)
{
	RenderBuffer buffer = SafeAlloc(sizeof(struct _renderBuffer));

	buffer->Handle = SharedHandles.Create();

	unsigned int handle = GraphicsDevice.GenerateRenderBuffer();

	buffer->Handle->Handle = handle;

	GraphicsDevice.AllocRenderBuffer(handle, format, width, height);

	// unbind the render buffer so we don't accidently mutate it with following calls
	GraphicsDevice.UseRenderBuffer(0);

	buffer->Format = format;
	buffer->Height = height;
	buffer->Width = width;

	return buffer;
}

static RenderBuffer Instance(RenderBuffer buffer)
{
	RenderBuffer newBuffer = SafeAlloc(sizeof(struct _renderBuffer));

	newBuffer->Handle = SharedHandles.Instance(buffer->Handle);
	
	newBuffer->Format = buffer->Format;
	newBuffer->Width = buffer->Width;
	newBuffer->Height = buffer->Height;

	return newBuffer;
}