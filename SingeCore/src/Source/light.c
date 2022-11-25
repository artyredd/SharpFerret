#include "graphics/light.h"
#include "singine/memory.h"
#include "singine/defaults.h"
#include "string.h"
#include "GL/glew.h"

struct _shadowMapSettings ShadowMaps = {
	.ResolutionX = SHADOW_MAP_RESOLUTION_X,
	.ResolutionY = SHADOW_MAP_RESOLUTION_Y
};

static Light Create(LightType type);
static void Dispose(Light);
static void CreateFrameBuffer(Light light);

const struct _lightMethods Lights = {
	.Create = &Create,
	.Dispose = &Dispose,
	.CreateFrameBuffer = &CreateFrameBuffer
};

static Light Create(LightType type)
{
	Light light = Memory.Alloc(sizeof(struct _light));

	SetVector4Macro(light->Ambient, DEFAULT_AMBIENT_LIGHT_INTENSITY, DEFAULT_AMBIENT_LIGHT_INTENSITY, DEFAULT_AMBIENT_LIGHT_INTENSITY, 1);
	SetVector4Macro(light->Diffuse, DEFAULT_DIFFUSE_LIGHT_INTENSITY, DEFAULT_DIFFUSE_LIGHT_INTENSITY, DEFAULT_DIFFUSE_LIGHT_INTENSITY, 1);
	SetVector4Macro(light->Specular, DEFAULT_SPECULAR_LIGHT_INTENSITY, DEFAULT_SPECULAR_LIGHT_INTENSITY, DEFAULT_SPECULAR_LIGHT_INTENSITY, 1);

	light->Radius = DEFAULT_LIGHT_RADIUS;
	light->Range = DEFAULT_LIGHT_RANGE;
	light->EdgeSoftness = DEFAULT_LIGHT_EDGE_SOFTNESS;
	light->Type = type;
	light->Enabled = true;
	light->Intensity = DEFAULT_LIGHT_INTENSITY;

	light->Transform = Transforms.Create();

	light->Orthographic = true;

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

	TextureType type = TextureTypes.Default;
	
	Texture depthBuffer;
	Textures.TryCreateBufferTexture(type, TextureFormats.DepthComponent, BufferFormats.Float, ShadowMaps.ResolutionX, ShadowMaps.ResolutionY, &depthBuffer);

	FrameBuffers.AttachTexture(frameBuffer, depthBuffer, 0);

	Textures.Dispose(depthBuffer);

	frameBuffer->ClearMask = ClearMasks.Depth;

	light->FrameBuffer = frameBuffer;
}

static void Dispose(Light light)
{
	if (light is null) return;

	Transforms.Dispose(light->Transform);
	FrameBuffers.Dispose(light->FrameBuffer);

	Memory.Free(light);
}