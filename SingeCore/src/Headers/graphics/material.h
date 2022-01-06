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
	/// <summary>
	/// Enables the shader and loads the color and texture if they exist in the shader
	/// </summary>
	void (*Draw)(Material);
	/// <summary>
	/// Creates a new instance of the provided texture, disposes the old one and reassigns the main texture of the provided material
	/// </summary>
	void (*SetMainTexture)(Material, Texture);
	/// <summary>
	/// Creates a new instance of the provided shade, disposes the old one and reassigns the main texture of the provided shader
	/// </summary>
	void (*SetShader)(Material, Shader);
	void (*Dispose)(Material);
};

const extern struct _materialMethods Materials;