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
static void RefreshMatrices(Light);

const struct _lightMethods Lights = {
	.Create = &Create,
	.Dispose = &Dispose,
	.RefreshMatrices = &RefreshMatrices,
	.CreateFrameBuffer = &CreateFrameBuffer
};

static Light Create(LightType type)
{
	Light light = SafeAlloc(sizeof(struct _light));

	SetVector4Macro(light->Ambient, DEFAULT_AMBIENT_LIGHT_INTENSITY, DEFAULT_AMBIENT_LIGHT_INTENSITY, DEFAULT_AMBIENT_LIGHT_INTENSITY, 1);
	SetVector4Macro(light->Diffuse, DEFAULT_DIFFUSE_LIGHT_INTENSITY, DEFAULT_DIFFUSE_LIGHT_INTENSITY, DEFAULT_DIFFUSE_LIGHT_INTENSITY, 1);
	SetVector4Macro(light->Specular, DEFAULT_SPECULAR_LIGHT_INTENSITY, DEFAULT_SPECULAR_LIGHT_INTENSITY, DEFAULT_SPECULAR_LIGHT_INTENSITY, 1);

	light->Radius = DEFAULT_LIGHT_RADIUS;
	light->Range = DEFAULT_LIGHT_RANGE;
	light->EdgeSoftness = DEFAULT_LIGHT_EDGE_SOFTNESS;
	light->Type = type;
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

	TextureType type = TextureTypes.Default;

	// if the light isn't a directional light we need to use a cubemap to render the shadowmaps
	if (light->Type isnt LightTypes.Directional)
	{
		type = TextureTypes.CubeMap;
	}
	
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

	SafeFree(light);
}

static void RefreshMatrices(Light light)
{
	// check to see if we need to refresh any of the light's matrices
	// the light matrices array is used to in the shadow geometry shader to transform into various directions
	// in order to render shadow maps in a single pass
	Transforms.Refresh(light->Transform);

	// get the right and up directions, compare to the previous directions
	// if they are the same we know for a fact the light's matrices have not changed
	// otherwise recalculate all matrices
	vec3 right;
	vec3 up;
	Transforms.GetDirection(light->Transform, Directions.Right, right);
	Transforms.GetDirection(light->Transform, Directions.Up, up);

	// compare them, if they are the same we know the transform and directions never changed
	if (Vector3Equals(right, light->PreviousState[0]) &&
		Vector3Equals(up, light->PreviousState[1]))
	{
		return;
	}

	// since either the up or right diretcions changed we have to recalc everying
	// store the state
	Vectors3CopyTo(right, light->PreviousState[0]);
	Vectors3CopyTo(up, light->PreviousState[1]);

	vec3 left;
	vec3 down;
	vec3 forward;
	vec3 back;

	// while we could use hard coded directions here, we want to be able to have realtime shadows that rotate with a light caster
	// so we shouldn't use cardinal here and instead should get the direction from the transform itself
	Transforms.GetDirection(light->Transform, Directions.Left, left);
	
	Transforms.GetDirection(light->Transform, Directions.Down, down);
	Transforms.GetDirection(light->Transform, Directions.Forward, forward);
	Transforms.GetDirection(light->Transform, Directions.Back, back);

	float* position = light->Transform->Position;

	// update the light matrices
	Matrices.LookAt(light->LightMatrices[0], position, right, Vector3.Down);
	Matrices.LookAt(light->LightMatrices[1], position, left, Vector3.Down);

	Matrices.LookAt(light->LightMatrices[2], position, up, Vector3.Forward);
	Matrices.LookAt(light->LightMatrices[3], position, down, Vector3.Back);

	Matrices.LookAt(light->LightMatrices[4], position, forward, Vector3.Down);
	Matrices.LookAt(light->LightMatrices[5], position, back, Vector3.Down);
}