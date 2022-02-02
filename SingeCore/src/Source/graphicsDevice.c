#include "graphics/graphicsDevice.h"
#include "GL/glew.h"
#include <stdlib.h>
#include "graphics/textureDefinitions.h"

const struct _comparisons Comparisons = {
	.Always = GL_ALWAYS,
	.Never = GL_NEVER,
	.Equal = GL_EQUAL,
	.NotEqual = GL_NOTEQUAL,
	.GreaterThan = GL_GREATER,
	.LessThan = GL_LESS,
	.LessThanOrEqual = GL_LEQUAL,
	.GreaterThanOrEqual = GL_GEQUAL
};

static void EnableBlending(void);
static void EnableCulling(void);
static void DisableBlending(void);
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
static void ActivateTexture(const unsigned int textureHandle, const int uniformHandle, const unsigned int slot);
static unsigned int CreateTexture(TextureType);
static void DeleteTexture(unsigned int handle);
static bool TryVerifyCleanup(void);
static void LoadTexture(TextureType, TextureFormat, BufferFormat, Image);
static void ModifyTexture(TextureType, TextureSetting, TextureSettingValue);

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
	.ModifyTexture = ModifyTexture
};

bool blendingEnabled = false;
bool cullingEnabled = false;
bool writingToStencilBufferEnabled = false;
bool depthTestingEnabled = false;

unsigned int stencilMask = 0xFF;
unsigned int stencilComparisonFunction;
unsigned int stencilComparisonValue;
unsigned int stencilComparisonMask;
unsigned int defaultStencilComparison = GL_ALWAYS;
unsigned int defaultStencilComparisonValue = 1;
unsigned int defaultStencilComparisonMask = 0xFF;
unsigned int depthComparison;

unsigned int nextTexture = 0;
unsigned int maxTextures = GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS;

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
	if (comparison isnt stencilComparisonFunction || stencilComparisonValue isnt 1 || stencilComparisonMask isnt 0xFF)
	{
		glStencilFunc(comparison, 1, 0xFF);

		stencilComparisonFunction = comparison;

		stencilComparisonValue = 1;

		stencilMask = 0xFF;
	}
}

static void SetStencilFunctionFull(const Comparison comparison, const unsigned int valueToCompareTo, const unsigned int mask)
{
	if (comparison isnt stencilComparisonFunction || valueToCompareTo isnt stencilComparisonValue || mask != stencilComparisonMask)
	{
		glStencilFunc(comparison, valueToCompareTo, mask);

		stencilComparisonFunction = comparison;

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
	if (depthComparison isnt comparison)
	{
		glDepthFunc(comparison);
		depthComparison = comparison;
	}
}

static void ActivateTexture(const unsigned int textureHandle, const int uniformHandle, const unsigned int slot)
{
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	glUniform1i(uniformHandle, slot);
	glDisable(GL_TEXTURE_2D);
}

static unsigned int CreateTexture(TextureType type)
{
	unsigned int handle;
	glGenTextures(1, &handle);

	glBindTexture(type, handle);

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

static void LoadTexture(TextureType type, TextureFormat colorFormat, BufferFormat pixelFormat, Image image)
{
	glTexImage2D(type, 0, colorFormat, image->Width, image->Height, 0, colorFormat, pixelFormat, image->Pixels);
}

static void ModifyTexture(TextureType type, TextureSetting setting, TextureSettingValue value)
{
	glTexParameteri(type, setting, value);
}

static bool TryVerifyCleanup(void)
{
	bool result = true;

	// verify that all textures were destroyed
	fprintf(stderr, "Orphaned Textures: %lli"NEWLINE, activeTextures);

	result &= activeTextures is 0;

	return result;
}
