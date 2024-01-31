#include "engine/graphics/renderbuffers.h"
#include "core/memory.h"

static void Dispose(RenderBuffer);
static RenderBuffer Create(size_t width, size_t height, TextureFormat format);
static RenderBuffer Instance(RenderBuffer);

const struct _renderBufferMethods RenderBuffers = {
	.Dispose = &Dispose,
	.Create = &Create,
	.Instance = &Instance
};

static void OnDispose(RenderBuffer state)
{
	GraphicsDevice.DeleteRenderBuffer(state->Handle->Handle);
}

DEFINE_TYPE_ID(RenderBuffer);

static void Dispose(RenderBuffer buffer)
{
	if (buffer isnt null)
	{
		SharedHandles.Dispose(buffer->Handle, buffer, OnDispose);
	}

	Memory.Free(buffer, RenderBufferTypeId);
}

static RenderBuffer Create(size_t width, size_t height, TextureFormat format)
{
	Memory.RegisterTypeName(nameof(RenderBuffer), &RenderBufferTypeId);

	RenderBuffer buffer = Memory.Alloc(sizeof(struct _renderBuffer), RenderBufferTypeId);

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
	RenderBuffer newBuffer = Memory.Alloc(sizeof(struct _renderBuffer), RenderBufferTypeId);

	newBuffer->Handle = SharedHandles.Instance(buffer->Handle);

	newBuffer->Format = buffer->Format;
	newBuffer->Width = buffer->Width;
	newBuffer->Height = buffer->Height;

	return newBuffer;
}