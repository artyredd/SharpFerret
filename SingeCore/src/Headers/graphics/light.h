#pragma once
#include "graphics/colors.h"
#include "math/vectors.h"
#include "graphics/transform.h"

#define DEFAULT_AMBIENT_LIGHT_INTENSITY 1.0f
#define DEFAULT_DIFFUSE_LIGHT_INTENSITY 0.5f
#define DEFAULT_SPECULAR_LIGHT_INTENSITY 1.0f

#define DEFAULT_LIGHT_RADIUS 0.87f // in degrees
#define DEFAULT_LIGHT_RANGE 5.0f // in distance units
#define DEFAULT_LIGHT_EDGE_SOFTNESS 0.057f;

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
	/// The ambient color and intensity of the light
	/// </summary>
	Color Ambient;
	/// <summary>
	/// The diffuse color and intensity of the light
	/// </summary>
	Color Diffuse;
	/// <summary>
	/// The specular color and intensity of the light
	/// </summary>
	Color Specular;
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
	/// The type of light that should should be rendered as
	/// 0 = point, 1 = directional, 2 = spotlight
	/// </summary>
	LightType Type;
	/// <summary>
	/// The transform for this light
	/// </summary>
	Transform Transform;
};

struct _lightMethods {
	Light(*Create)(void);
	void (*Dispose)(Light);
};

extern const struct _lightMethods Lights;