#include "graphics/framebuffers.h"
#include "singine/memory.h"

static void Dispose(FrameBuffer);
static FrameBuffer Create(void);
static void Use(FrameBuffer);
static void AttachTexture(FrameBuffer, Texture, unsigned int offset);
static void AttachRenderBuffer(FrameBuffer, RenderBuffer);

const struct _frameBufferMethods FrameBuffers = {
	.Dispose = &Dispose,
	.Create = &Create,
	.Use = &Use,
	.AttachTexture = AttachTexture,
	.AttachRenderBuffer = AttachRenderBuffer
};

static void OnDispose(FrameBuffer state)
{
	GraphicsDevice.DeleteFrameBuffer(state->Handle->Handle);
}

static void Dispose(FrameBuffer buffer)
{
	if (buffer is null) return;

	SharedHandles.Dispose(buffer->Handle, buffer, OnDispose);

	if (buffer->Texture isnt null)
	{
		Textures.Dispose(buffer->Texture);
	}

	if (buffer->RenderBuffer isnt null)
	{
		RenderBuffers.Dispose(buffer->RenderBuffer);
	}

	SafeFree(buffer);
}

static FrameBuffer Create(void)
{
	FrameBuffer buffer = SafeAlloc(sizeof(struct _frameBuffer));

	buffer->Handle = SharedHandles.Create();

	buffer->Handle->Handle = GraphicsDevice.GenerateFrameBuffer();

	return buffer;
}

static void Use(FrameBuffer buffer)
{
	GraphicsDevice.UseFrameBuffer(buffer->Handle->Handle);
}

static void AttachTexture(FrameBuffer buffer, Texture texture, unsigned int offset)
{
	if (texture->Format is TextureFormats.Depth24Stencil8)
	{
		GraphicsDevice.AttachFrameBufferComponent(FrameBufferComponents.Texture, FrameBufferAttachments.DepthStencil, texture->Handle->Handle);
	}else if(texture->Format is TextureFormats.RGB)
	{
		GraphicsDevice.AttachFrameBufferComponent(FrameBufferComponents.Texture, FrameBufferAttachments.Color + offset, texture->Handle->Handle);
	}

	buffer->Texture = Textures.Instance(texture);
}

static void AttachRenderBuffer(FrameBuffer buffer, RenderBuffer renderBuffer)
{
	if (renderBuffer->Format is TextureFormats.Depth24Stencil8)
	{
		GraphicsDevice.AttachFrameBufferComponent(FrameBufferComponents.RenderBuffer, FrameBufferAttachments.DepthStencil, renderBuffer->Handle->Handle);
	}
	else if (renderBuffer->Format is TextureFormats.RGB)
	{
		GraphicsDevice.AttachFrameBufferComponent(FrameBufferComponents.RenderBuffer, FrameBufferAttachments.Color, renderBuffer->Handle->Handle);
	}

	buffer->RenderBuffer = RenderBuffers.Instance(renderBuffer);
}