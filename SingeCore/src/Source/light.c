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

	SetVector4(light->Ambient, DEFAULT_AMBIENT_LIGHT_INTENSITY, DEFAULT_AMBIENT_LIGHT_INTENSITY, DEFAULT_AMBIENT_LIGHT_INTENSITY, 1);
	SetVector4(light->Diffuse, DEFAULT_DIFFUSE_LIGHT_INTENSITY, DEFAULT_DIFFUSE_LIGHT_INTENSITY, DEFAULT_DIFFUSE_LIGHT_INTENSITY, 1);
	SetVector4(light->Specular, DEFAULT_SPECULAR_LIGHT_INTENSITY, DEFAULT_SPECULAR_LIGHT_INTENSITY, DEFAULT_SPECULAR_LIGHT_INTENSITY, 1);

	light->Radius = 1.0f;
	light->Range = 5.0f;
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