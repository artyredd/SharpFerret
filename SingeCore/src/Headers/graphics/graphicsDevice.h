#pragma once
#include "csharp.h"
#include <stdint.h>
#include "graphics/imaging.h"
#include "graphics/textureDefinitions.h"


typedef unsigned int Comparison;

struct _comparisons {
	Comparison Always;
	Comparison Never;
	Comparison Equal;
	Comparison NotEqual;
	Comparison GreaterThan;
	Comparison LessThan;
	Comparison GreaterThanOrEqual;
	Comparison LessThanOrEqual;
};

extern const struct _comparisons Comparisons;

struct _graphicsDeviceMethods
{
	void (*EnableBlending)(void);
	void (*EnableCulling)(void);
	void (*DisableBlending)(void);
	void (*DisableCulling)(void);
	void (*EnableStencilWriting)(void);
	void (*DisableStencilWriting)(void);
	void(*SetStencilMask)(const unsigned int mask);
	unsigned int (*GetStencilMask)(void);
	/// <summary>
	/// Sets the function that determins whether or not a fragment gets drawn, a fragment will only be drawn when the Comparison would return true
	/// For example: the stencil value for a fragment is 1, and SetStencilFunction(Comparisons.Equal, 1), would mean the fragment would only be drawn
	/// when it's stencil value is 1, otherwise it's not drawn
	/// </summary>
	void (*SetStencilFull)(const Comparison, const unsigned int valueToCompareTo, const unsigned int mask);
	/// <summary>
	/// Sets the function that determins whether or not a fragment gets drawn, a fragment will only be drawn when the Comparison would return true
	/// For example: the stencil value for a fragment is 1, and SetStencilFunction(Comparisons.Equal, 1), would mean the fragment would only be drawn
	/// when it's stencil value is 1, otherwise it's not drawn
	/// </summary>
	void (*SetStencil)(const Comparison);
	void (*ResetStencilFunction)(void);
	void (*EnableDepthTesting)(void);
	void (*DisableDepthTesting)(void);
	void (*SetDepthTest)(const Comparison);
	/// <summary>
	/// Generates a new texture buffer and binds it to the provided type on the graphics device, returns the handle to the generated texture
	/// </summary>
	unsigned int (*CreateTexture)(TextureType);
	/// <summary>
	/// deletes the provided texture on the graphics device
	/// </summary>
	void (*DeleteTexture)(unsigned int handle);
	/// <summary>
	/// Loads the provided image to the graphics device to the currently bound texture
	/// </summary>
	void (*LoadTexture)(const TextureType, TextureFormat, BufferFormat, Image);
	/// <summary>
	/// Modifies the currenly bound texture with the provided setting and value
	/// </summary>
	void (*ModifyTexture)(const TextureType, TextureSetting, const TextureValue);
	/// <summary>
	/// Binds the provided texture to an open texture slot on the graphics device
	/// </summary>
	void (*ActivateTexture)(const TextureType, const unsigned int textureHandle, const int uniformHandle, const unsigned int slot);
	/// <summary>
	/// Verifies that cleanup was properly performed before program exit
	/// </summary>
	bool (*TryVerifyCleanup)(void);
};

extern const struct _graphicsDeviceMethods GraphicsDevice;