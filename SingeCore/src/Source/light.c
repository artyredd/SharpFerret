#include "graphics/light.h"
#include "singine/memory.h"
#include "singine/defaults.h"

struct _shadowMapSettings ShadowMaps = {
	.ResolutionX = SHADOW_MAP_RESOLUTION_X,
	.ResolutionY = SHADOW_MAP_RESOLUTION_Y
};

static Light Create(void);
static void Dispose(Light);
static void CreateFrameBuffer(Light light);

const struct _lightMethods Lights = {
	.Create = &Create,
	.Dispose = &Dispose
};

static Light Create(void)
{
	Light light = SafeAlloc(sizeof(struct _light));

	SetVector4Macro(light->Ambient, DEFAULT_AMBIENT_LIGHT_INTENSITY, DEFAULT_AMBIENT_LIGHT_INTENSITY, DEFAULT_AMBIENT_LIGHT_INTENSITY, 1);
	SetVector4Macro(light->Diffuse, DEFAULT_DIFFUSE_LIGHT_INTENSITY, DEFAULT_DIFFUSE_LIGHT_INTENSITY, DEFAULT_DIFFUSE_LIGHT_INTENSITY, 1);
	SetVector4Macro(light->Specular, DEFAULT_SPECULAR_LIGHT_INTENSITY, DEFAULT_SPECULAR_LIGHT_INTENSITY, DEFAULT_SPECULAR_LIGHT_INTENSITY, 1);

	light->Radius = DEFAULT_LIGHT_RADIUS;
	light->Range = DEFAULT_LIGHT_RANGE;
	light->EdgeSoftness = DEFAULT_LIGHT_EDGE_SOFTNESS;
	light->Type = LightTypes.Point;
	light->Enabled = true;

	light->Transform = Transforms.Create();

	CreateFrameBuffer(light);

	return light;
}

static void CreateFrameBuffer(Light light)
{
	if (light->FrameBuffer isnt null)
	{
		FrameBuffers.Dispose(light->FrameBuffer);
	}

	// create a frame buffer that has no color buffer (.None)
	FrameBuffer frameBuffer = FrameBuffers.Create(FrameBufferTypes.None);

	// mark it as the current framebuffer
	FrameBuffers.Use(frameBuffer);

	// create the texture that should hold the shadow map
	Texture depthBuffer;
	Textures.TryCreateBufferTexture(TextureTypes.Default, TextureFormats.DepthComponent, BufferFormats.Float, ShadowMaps.ResolutionX, ShadowMaps.ResolutionY, &depthBuffer);

	FrameBuffers.AttachTexture(frameBuffer, depthBuffer, 0);

	frameBuffer->ClearMask = ClearMasks.Depth;

	Textures.Dispose(depthBuffer);

	light->FrameBuffer = frameBuffer;
}

static void Dispose(Light light)
{
	if (light is null) return;

	Transforms.Dispose(light->Transform);
	FrameBuffers.Dispose(light->FrameBuffer);

	SafeFree(light);
}