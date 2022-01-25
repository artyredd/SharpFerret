#include "graphics/light.h"
#include "singine/memory.h"

static Light Create(void);
static void Dispose(Light);

const struct _lightMethods Lights = {
	.Create = &Create,
	.Dispose = &Dispose
};

static Light Create(void)
{
	Light light = SafeAlloc(sizeof(struct _light));

	light->Transform = Transforms.Create();

	return light;
}

static void Dispose(Light light)
{
	if (light is null) return;

	Transforms.Dispose(light->Transform);

	SafeFree(light);
}