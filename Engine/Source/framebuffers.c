#include "engine/graphics/framebuffers.h"
#include "core/memory.h"
#include "engine/defaults.h"
#include "GL/glew.h"

static void Dispose(FrameBuffer);
static FrameBuffer Create(FrameBufferType);
static void Use(FrameBuffer);
static void AttachTexture(FrameBuffer, RawTexture, unsigned int offset);
static void AttachRenderBuffer(FrameBuffer, RenderBuffer);
static void Clear(FrameBuffer);
static void ClearThenUse(FrameBuffer);

// the handle for the default framebuffer
static struct _sharedHandle DefaultHandle = {
	.Handle = 0,
	// Active instances is large here because this is alloced at compile time and cannot be disposed
	// without throwing an exception, this should never be disposed
	.ActiveInstances = 0xDEADBEEF
};

// the default frame buffer, all calls to handle 0 go directly to graphics device
static struct _frameBuffer Default = {
	.Handle = &DefaultHandle,
	null,
	null,
	.Height = DEFAULT_VIEWPORT_RESOLUTION_Y,
	.Width = DEFAULT_VIEWPORT_RESOLUTION_X,
	.ClearMask = (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)
};

const struct _frameBufferMethods FrameBuffers = {
	.Default = &Default,
	.Dispose = &Dispose,
	.Create = &Create,
	.Use = &Use,
	.AttachTexture = AttachTexture,
	.AttachRenderBuffer = AttachRenderBuffer,
	.ClearAndUse = &ClearThenUse,
	.Clear = &Clear
};

DEFINE_TYPE_ID(FrameBuffer);

#define FrameBufferType_None 0
#define FrameBufferType_Default 4
#define FrameBufferType_Read 1
#define FrameBufferType_Draw 2

const struct _frameBufferTypes FrameBufferTypes = {
	.None = FrameBufferType_None,
	.Default = FrameBufferType_Default,
	.Read = FrameBufferType_Read,
	.Draw = FrameBufferType_Draw
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
		RawTextures.Dispose(buffer->Texture);
	}

	if (buffer->RenderBuffer isnt null)
	{
		RenderBuffers.Dispose(buffer->RenderBuffer);
	}

	Memory.Free(buffer, FrameBufferTypeId);
}

static FrameBuffer Create(FrameBufferType type)
{
	Memory.RegisterTypeName("FrameBuffer", &FrameBufferTypeId);
	FrameBuffer buffer = Memory.Alloc(sizeof(struct _frameBuffer), FrameBufferTypeId);

	buffer->Handle = SharedHandles.Create();

	buffer->Handle->Handle = GraphicsDevice.GenerateFrameBuffer();

	GraphicsDevice.UseFrameBuffer(buffer->Handle->Handle);

	// check if we need to disable the draw or read buffers
	if ((type & FrameBufferTypes.Draw) is 0)
	{
		GraphicsDevice.SetDrawBuffer(ColorBufferTypes.None);
	}

	if ((type & FrameBufferTypes.Read) is 0)
	{
		GraphicsDevice.SetReadBuffer(ColorBufferTypes.None);
	}

	GraphicsDevice.UseFrameBuffer(0);

	return buffer;
}

#define UseFrameBufferMacro() GraphicsDevice.SetResolution(0, 0, buffer->Width, buffer->Height);\
GraphicsDevice.UseFrameBuffer(buffer->Handle->Handle);

static void Use(FrameBuffer buffer)
{
	UseFrameBufferMacro();
}

static void StoreDimensions(FrameBuffer buffer, size_t width, size_t height)
{
	// only store the width and height if there values are 0
	if (buffer->Width is 0 || buffer->Height is 0)
	{
		buffer->Width = width;
		buffer->Height = height;
	}
	else
	{
		// if the height and width have already been set ensure that the dimensions
		// provided exactly match, it would make no sense to have a 1920x1080 color buffer
		// but a 1024x1024 depth buffer
		if (buffer->Width isnt width || buffer->Height isnt height)
		{
			fprintf(stderr, "The resoltion of the current buffer %llix%lli does not match the provided resoltuion of %llix%lli, all attachments of the frame buffer must have the same resolution.", buffer->Width, buffer->Height, width, height);
			throw(ResolutionMismatchException);
		}
	}
}

static void AttachTexture(FrameBuffer buffer, RawTexture texture, unsigned int offset)
{
	FrameBufferComponent componentType = FrameBufferComponents.Texture;

	if (texture->Type.Value.AsUInt is TextureTypes.CubeMap.Value.AsUInt)
	{
		componentType = FrameBufferComponents.Cubemap;
	}

	if (texture->Format is TextureFormats.Depth24Stencil8)
	{
		GraphicsDevice.AttachFrameBufferComponent(componentType, FrameBufferAttachments.DepthStencil, texture->Handle->Handle);
	}
	else if (texture->Format is TextureFormats.RGB)
	{
		GraphicsDevice.AttachFrameBufferComponent(componentType, FrameBufferAttachments.Color + offset, texture->Handle->Handle);
	}
	else if (texture->Format is TextureFormats.DepthComponent)
	{
		GraphicsDevice.AttachFrameBufferComponent(componentType, FrameBufferAttachments.Depth, texture->Handle->Handle);
	}

	buffer->Texture = RawTextures.Instance(texture);

	// check to see if we need to store the height
	StoreDimensions(buffer, texture->Rect.Width, texture->Rect.Height);
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

	// check to see if we need to store the height
	StoreDimensions(buffer, renderBuffer->Width, renderBuffer->Height);
}

static void Clear(FrameBuffer buffer)
{
	GraphicsDevice.ClearCurrentFrameBuffer(buffer->ClearMask);
}

static void ClearThenUse(FrameBuffer buffer)
{
	UseFrameBufferMacro();

	GraphicsDevice.ClearCurrentFrameBuffer(buffer->ClearMask);
}