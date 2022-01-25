#pragma once
#include "graphics/colors.h"
#include "math/vectors.h"
#include "graphics/transform.h"

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
	/// The color of the light, default color is Colors.White(white)
	/// </summary>
	Color Color;
	/// <summary>
	/// How powerful the light should be
	/// </summary>
	float Intensity;
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