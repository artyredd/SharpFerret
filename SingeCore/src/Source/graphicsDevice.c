#include "graphics/graphicsDevice.h"
#include "GL/glew.h"

static void EnableBlending(void);
static void EnableCulling(void);
static void DisableBlending(void);
static void DisableCulling(void);

const struct _graphicsDeviceMethods GraphicsDevice = {
	.EnableBlending = &EnableBlending,
	.EnableCulling = &EnableCulling,
	.DisableBlending = &DisableBlending,
	.DisableCulling = &DisableCulling
};

bool blendingEnabled = false;
bool cullingEnabled = false;

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