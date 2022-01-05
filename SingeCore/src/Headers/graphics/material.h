#pragma once

#include "graphics/shaders.h"
#include "graphics/texture.h"

typedef struct _material* Material;

struct _material {
	Shader Shader;
	vec4 Color;
	Texture MainTexture;
};

struct _materialMethods {
	Material(*CreateMaterial)(void);
	Material(*Create)(Shader, Texture);
	/// <summary>
	/// Creates a new instance of the provided material
	/// </summary>
	Material(*Instance)(Material);
	void (*Draw)(Material);
	void (*Dispose)(Material);
};

const extern struct _materialMethods Materials;