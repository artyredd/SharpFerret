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

	SetVector4(light->Color, 1,1,1,1);

	light->Intensity = 0.5f;
	light->Radius = 1.0f;
	light->Range = 10.0f;
	light->Type = LightTypes.Point;

	light->Transform = Transforms.Create();

	return light;
}

static void Dispose(Light light)
{
	if (light is null) return;

	Transforms.Dispose(light->Transform);

	SafeFree(light);
}