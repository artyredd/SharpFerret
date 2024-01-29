#pragma once
#include "engine/graphics/colors.h"
#include "core/math/vectors.h"
#include "engine/graphics/transform.h"
#include <engine/graphics/framebuffers.h>

#define DEFAULT_LIGHT_INTENSITY 1.0f

#define DEFAULT_AMBIENT_LIGHT_INTENSITY 1.0f
#define DEFAULT_DIFFUSE_LIGHT_INTENSITY 1.0f
#define DEFAULT_SPECULAR_LIGHT_INTENSITY 1.0f

#define DEFAULT_LIGHT_RADIUS 0.87f // in degrees for spot light, in distance units for point and directional
#define DEFAULT_LIGHT_RANGE 5.0f // in distance units
#define DEFAULT_LIGHT_EDGE_SOFTNESS 0.057f;

struct _shadowMapSettings {
	size_t ResolutionX;
	size_t ResolutionY;
};

extern struct _shadowMapSettings ShadowMaps;

typedef int LightType;

static const struct _lightTypes {
	/// <summary>
	/// Casts light in all directions
	/// </summary>
	LightType Point;
	/// <summary>
	/// Casts light in a particular direction infinitely
	/// </summary>
	LightType Directional;
	/// <summary>
	/// Casts light within a cone in a particular direction
	/// </summary>
	LightType Spot;
} LightTypes = {
	.Point = 0,
	.Directional = 1,
	.Spot = 2
};

typedef struct _light* Light;

struct _light {
	/// <summary>
	/// Wether or not this light is enabled, when false it is not used to render objects
	/// </summary>
	bool Enabled;
	/// <summary>
	/// The overall intensity of the light, this is multiplied last after all other portions
	/// </summary>
	float Intensity;
	/// <summary>
	/// The ambient color and intensity of the light
	/// </summary>
	color Ambient;
	/// <summary>
	/// The diffuse color and intensity of the light
	/// </summary>
	color Diffuse;
	/// <summary>
	/// The specular color and intensity of the light
	/// </summary>
	color Specular;
	/// <summary>
	/// How far the light should reach, objects further than the provided distace are not lit
	/// by this light
	/// </summary>
	float Range;
	/// <summary>
	/// The radius of the light, this only applies to spot lights
	/// </summary>
	float Radius;
	/// <summary>
	/// How soft the edges of spot lights should be
	/// </summary>
	float EdgeSoftness;
	/// <summary>
	/// Whether or not the light should render shadows as orthographic or perspective
	/// </summary>
	bool Orthographic;
	/// <summary>
	/// The type of light that should should be rendered as
	/// 0 = point, 1 = directional, 2 = spotlight
	/// </summary>
	LightType Type;
	/// <summary>
	/// The transform for this light
	/// </summary>
	Transform Transform;
	/// <summary>
	/// The framebuffer that is contains the shadow map for this light, this may contain a 2d texture(direction) or cubemap(point, spotlight)
	/// </summary>
	FrameBuffer FrameBuffer;
	/// <summary>
	/// The matrix that should be set for this light to calculate fragment position during runtime
	/// </summary>
	matrix4 ViewMatrix;
};

struct _lightMethods {
	Light(*Create)(LightType);
	void (*CreateFrameBuffer)(Light light);
	void (*Dispose)(Light);
};

extern const struct _lightMethods Lights;