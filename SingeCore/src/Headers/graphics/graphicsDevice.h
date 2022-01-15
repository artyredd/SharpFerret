#pragma once
#include "csharp.h"

struct _graphicsDeviceMethods
{
	void (*EnableBlending)(void);
	void (*EnableCulling)(void);
	void (*DisableBlending)(void);
	void (*DisableCulling)(void);
};

extern const struct _graphicsDeviceMethods GraphicsDevice;