#include "graphics/graphicsDevice.h"
#include "GL/glew.h"
#include <stdlib.h>
#include "graphics/textureDefinitions.h"

const struct _frameBufferAttachments FrameBufferAttachments = {
	.Depth = GL_DEPTH_ATTACHMENT,
	.Color = GL_COLOR_ATTACHMENT0,
	.DepthStencil = GL_DEPTH_STENCIL_ATTACHMENT
};

const struct _frameBufferComponents FrameBufferComponents = {
	.Texture = 0,
	.RenderBuffer = 1
};

struct _comparisons Comparisons = {
	.Always = { "always", GL_ALWAYS },
	.Never = { "never", GL_NEVER },
	.Equal = { "equal", GL_EQUAL },
	.NotEqual = { "notEqual", GL_NOTEQUAL },
	.GreaterThan = { "greater", GL_GREATER },
	.LessThan = { "less", GL_LESS },
	.LessThanOrEqual = { "lessOrEqual", GL_LEQUAL },
	.GreaterThanOrEqual = { "greaterOrEqual", GL_GEQUAL }
};

const struct _bufferTypes ColorBufferTypes = {
	.None = GL_NONE,
	.FrontLeft = GL_FRONT_LEFT, 
	.FrontRight = GL_FRONT_RIGHT, 
	.BackLeft = GL_BACK_LEFT, 
	.BackRight = GL_BACK_RIGHT, 
	.Front = GL_FRONT, 
	.Back = GL_BACK, 
	.Left = GL_LEFT,
	.Right = GL_RIGHT,
	.Color = GL_COLOR_ATTACHMENT0
};

static void EnableBlending(void);
static void DisableBlending(void);

static void EnableCulling(void);
static void DisableCulling(void);

static void EnableWritingToStencilBuffer(void);
static void DisableWritingToStencilBuffer(void);
static unsigned int GetStencilMask(void);
static void SetStencilMask(const unsigned int mask);
static void SetStencilFunction(const Comparison);
static void SetStencilFunctionFull(const Comparison, const unsigned int valueToCompareTo, const unsigned int mask);
static void ResetStencilFunction(void);

static void EnableDepthTesting(void);
static void DisableDepthTesting(void);
static void SetDepthTest(const Comparison);

static void ActivateTexture(const TextureType, const unsigned int textureHandle, const int uniformHandle, const unsigned int slot);
static unsigned int CreateTexture(const TextureType);
static void LoadTexture(const TextureType, TextureFormat, BufferFormat, Image, unsigned int offset);
static void LoadBufferTexture(const TextureType, const TextureFormat, const BufferFormat, size_t width, size_t height, unsigned int offset);
static void ModifyTexture(const TextureType, TextureSetting, const TextureValue);
static void DeleteTexture(unsigned int handle);
static bool TryVerifyCleanup(void);

static unsigned int GenerateBuffer(void);
static void DeleteBuffer(unsigned int handle);

static unsigned int GenerateRenderBuffer(void);
static void DeleteRenderBuffer(unsigned int handle);
static void UseRenderBuffer(unsigned int handle);

static unsigned int GenerateFrameBuffer(void);
static void DeleteFrameBuffer(unsigned int handle);

static void UseFrameBuffer(unsigned int handle);

static void AttachFrameBufferComponent(FrameBufferComponent componentType, FrameBufferAttachment attachmentType, unsigned int attachmentHandle);

static void AllocRenderBuffer(unsigned int handle, TextureFormat format, size_t width, size_t height);

static void SetResolution(signed long long int x, signed long long int y, size_t width, size_t height);

static void SetReadBuffer(ColorBufferType);
static void SetDrawBuffer(ColorBufferType);

const struct _graphicsDeviceMethods GraphicsDevice = {
	.EnableBlending = &EnableBlending,
	.EnableCulling = &EnableCulling,
	.DisableBlending = &DisableBlending,
	.DisableCulling = &DisableCulling,
	.EnableStencilWriting = &DisableWritingToStencilBuffer,
	.DisableStencilWriting = DisableWritingToStencilBuffer,
	.SetStencilMask = &SetStencilMask,
	.GetStencilMask = GetStencilMask,
	.SetStencil = &SetStencilFunction,
	.ResetStencilFunction = &ResetStencilFunction,
	.SetStencilFull = &SetStencilFunctionFull,
	.EnableDepthTesting = &EnableDepthTesting,
	.DisableDepthTesting = &DisableDepthTesting,
	.SetDepthTest = &SetDepthTest,
	.ActivateTexture = &ActivateTexture,
	.CreateTexture = CreateTexture,
	.DeleteTexture = DeleteTexture,
	.TryVerifyCleanup = TryVerifyCleanup,
	.LoadTexture = LoadTexture,
	.ModifyTexture = ModifyTexture,
	.GenerateBuffer = &GenerateBuffer,
	.DeleteBuffer = &DeleteBuffer,
	.GenerateRenderBuffer = &GenerateRenderBuffer,
	.DeleteRenderBuffer = &DeleteRenderBuffer,
	.GenerateFrameBuffer = &GenerateFrameBuffer,
	.DeleteFrameBuffer = &DeleteFrameBuffer,
	.UseFrameBuffer = &UseFrameBuffer,
	.LoadBufferTexture = &LoadBufferTexture,
	.AttachFrameBufferComponent = AttachFrameBufferComponent,
	.UseRenderBuffer = &UseRenderBuffer,
	.AllocRenderBuffer = AllocRenderBuffer,
	.SetResolution = &SetResolution,
};

bool blendingEnabled = false;
bool cullingEnabled = false;
bool writingToStencilBufferEnabled = false;
bool depthTestingEnabled = false;

unsigned int stencilMask = 0xFF;
unsigned int stencilComparisonFunction;
unsigned int stencilComparisonValue;
unsigned int stencilComparisonMask;
Comparison defaultStencilComparison = { "always", GL_ALWAYS };
unsigned int defaultStencilComparisonValue = 1;
unsigned int defaultStencilComparisonMask = 0xFF;
unsigned int depthComparison;

unsigned int nextTexture = 0;
unsigned int maxTextures = GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS;

// 0 is no/default
unsigned int currentFrameBuffer = 0;

// texture instance counts
size_t activeTextures = 0;

static void EnableDepthTesting(void)
{
	if (depthTestingEnabled is false)
	{
		glEnable(GL_DEPTH_TEST);
		depthTestingEnabled = true;
	}
}

static void DisableDepthTesting(void)
{
	if (depthTestingEnabled)
	{
		glDisable(GL_DEPTH_TEST);
		depthTestingEnabled = false;
	}
}

static void EnableBlending(void)
{
	if (blendingEnabled is false)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		blendingEnabled = true;
	}
}

static void DisableBlending(void)
{
	if (blendingEnabled)
	{
		glDisable(GL_BLEND);
		blendingEnabled = false;
	}
}

static void EnableCulling(void)
{
	if (cullingEnabled is false)
	{
		glEnable(GL_CULL_FACE);
		cullingEnabled = true;
	}
}

static void DisableCulling(void)
{
	if (cullingEnabled)
	{
		glDisable(GL_CULL_FACE);

		cullingEnabled = false;
	}
}

static void EnableWritingToStencilBuffer(void)
{
	if (writingToStencilBufferEnabled is false)
	{
		SetStencilMask(stencilMask);

		writingToStencilBufferEnabled = true;
	}
}

static void DisableWritingToStencilBuffer(void)
{
	if (writingToStencilBufferEnabled)
	{
		SetStencilMask(0x00);

		writingToStencilBufferEnabled = false;
	}
}

static unsigned int GetStencilMask(void)
{
	return stencilMask;
}

static void SetStencilMask(const unsigned int mask)
{
	stencilMask = mask;
}

static void SetStencilFunction(const Comparison comparison)
{
	if (comparison.Value.AsUInt isnt stencilComparisonFunction || stencilComparisonValue isnt 1 || stencilComparisonMask isnt 0xFF)
	{
		glStencilFunc(comparison.Value.AsUInt, 1, 0xFF);

		stencilComparisonFunction = comparison.Value.AsUInt;

		stencilComparisonValue = 1;

		stencilMask = 0xFF;
	}
}

static void SetStencilFunctionFull(const Comparison comparison, const unsigned int valueToCompareTo, const unsigned int mask)
{
	if (comparison.Value.AsUInt isnt stencilComparisonFunction || valueToCompareTo isnt stencilComparisonValue || mask != stencilComparisonMask)
	{
		glStencilFunc(comparison.Value.AsUInt, valueToCompareTo, mask);

		stencilComparisonFunction = comparison.Value.AsUInt;

		stencilComparisonValue = valueToCompareTo;

		stencilComparisonMask = mask;
	}
}

static void ResetStencilFunction(void)
{
	SetStencilFunctionFull(defaultStencilComparison, defaultStencilComparisonValue, defaultStencilComparisonMask);
}

static void SetDepthTest(const Comparison comparison)
{
	if (depthComparison isnt comparison.Value.AsUInt)
	{
		glDepthFunc(comparison.Value.AsUInt);
		depthComparison = comparison.Value.AsUInt;
	}
}

static void ActivateTexture(const TextureType textureType, const unsigned int textureHandle, const int uniformHandle, const unsigned int slot)
{
	unsigned int type = textureType.Value.AsUInt;
	//glEnable(type);
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(type, textureHandle);
	glUniform1i(uniformHandle, slot);
	//glDisable(type);
}

static unsigned int CreateTexture(TextureType type)
{
	unsigned int handle;
	glGenTextures(1, &handle);

	glBindTexture(type.Value.AsUInt, handle);

	// keep track of how many textures we create
	++(activeTextures);

	return handle;
}

static void DeleteTexture(unsigned int handle)
{
	glDeleteTextures(1, &handle);

	// keep track of how many textures we destroy
	--(activeTextures);
}

static void LoadTexture(TextureType type, TextureFormat colorFormat, BufferFormat pixelFormat, Image image, unsigned int offset)
{
	glTexImage2D(type.Value.AsUInt + offset, 0, colorFormat, image->Width, image->Height, 0, colorFormat, pixelFormat, image->Pixels);
}

static void LoadBufferTexture(const TextureType type, const TextureFormat colorFormat, const BufferFormat pixelFormat, size_t width, size_t height, unsigned int offset)
{
	glTexImage2D(type.Value.AsUInt + offset, 0, colorFormat, (int)width, (int)height, 0, colorFormat, pixelFormat, null);
}

static void ModifyTexture(TextureType type, TextureSetting setting, const TextureValue value)
{
	glTexParameteri(type.Value.AsUInt, setting, value.Value.AsInt);
}

static void UseFrameBuffer(unsigned int handle)
{
	if (handle isnt currentFrameBuffer)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, handle);
		currentFrameBuffer = handle;
	}
}

static void AttachFrameBufferComponent(FrameBufferComponent componentType, FrameBufferAttachment attachmentType, unsigned int attachmentHandle)
{
	if (componentType is FrameBufferComponents.Texture)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D, attachmentHandle, 0);
	}
	else if (componentType is FrameBufferComponents.RenderBuffer)
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, attachmentHandle);
	}
}

static void UseRenderBuffer(unsigned int handle)
{
	glBindRenderbuffer(GL_RENDERBUFFER, handle);
}

static void AllocRenderBuffer(unsigned int handle, TextureFormat format, size_t width, size_t height)
{
	glBindRenderbuffer(GL_RENDERBUFFER, handle);
	glRenderbufferStorage(GL_RENDERBUFFER,format, (GLsizei)width, (GLsizei)height);
}

static void SetReadBuffer(ColorBufferType mode)
{
	glReadBuffer(mode);
}

static void SetDrawBuffer(ColorBufferType mode)
{
	glDrawBuffer(mode);
}

size_t viewportWidth;
size_t viewportHeight;
signed long long int viewportX;
signed long long int viewportY;


static void SetResolution(signed long long int x, signed long long int y, size_t width, size_t height)
{
	if (viewportWidth isnt width || 
		viewportHeight isnt height ||
		viewportX isnt x ||
		viewportY isnt y)
	{
		glViewport((int)x, (int)y, (unsigned int)width, (unsigned int)height);

		viewportX = x;
		viewportY = y;
		viewportWidth = width;
		viewportHeight = height;
	}
}

#define BufferObjectBase(name, generateMethod, deleteMethod) size_t active ## name ## s= 0;\
static unsigned int Generate ## name()\
{\
unsigned int handle;\
	generateMethod\
++( active ## name ## s); \
return handle; \
}\
static void Delete ## name(unsigned int handle)\
{\
	deleteMethod\
	--( active ## name ## s);\
}\

BufferObjectBase(Buffer, glGenBuffers(1, &handle); , glDeleteBuffers(1, &handle););
BufferObjectBase(RenderBuffer, glGenRenderbuffers(1, &handle); , glDeleteRenderbuffers(1, &handle););
BufferObjectBase(FrameBuffer, glGenFramebuffers(1, &handle);, glDeleteFramebuffers(1, &handle););

static bool TryVerifyCleanup(void)
{
	bool result = true;

	// verify that all textures were destroyed
	fprintf(stderr, "Orphaned Textures: %lli"NEWLINE, activeTextures);

	result &= activeTextures is 0;

	fprintf(stderr, "Orphaned Mesh Buffers: %lli"NEWLINE, activeBuffers);

	result &= activeBuffers is 0;

	fprintf(stderr, "Orphaned Render Buffers: %lli"NEWLINE, activeRenderBuffers);

	result &= activeRenderBuffers is 0;

	fprintf(stderr, "Orphaned Frame Buffers: %lli"NEWLINE, activeFrameBuffers);

	result &= activeFrameBuffers is 0;

	return result;
}
