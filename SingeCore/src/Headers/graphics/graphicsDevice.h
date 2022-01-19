#pragma once
#include "csharp.h"
#include <stdint.h>

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
};

extern const struct _graphicsDeviceMethods GraphicsDevice;