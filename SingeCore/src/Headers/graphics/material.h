#pragma once

#include "graphics/shaders.h"
#include "graphics/texture.h"
#include "graphics/renderMesh.h"
#include "graphics/camera.h"
#include "graphics/colors.h"

typedef struct _material* Material;

struct _material {
	Shader* Shaders;
	size_t Count;
	Color Color;
	Texture MainTexture;
};

struct _materialMethods {
	Material(*CreateMaterial)(void);
	Material(*Create)(const Shader, const Texture);
	// Attempts to load the material definition at the given path
	Material(*Load)(const char* path);
	// Saves the provided  material to a material definition file at the given path so it can be loaded with Materials.Load, returns true 
	// when export was successful, otherwise false
	bool (*Save)(const Material, const char* path);
	/// <summary>
	/// Creates a new instance of the provided material
	/// </summary>
	Material(*Instance)(Material);
	/// <summary>
	/// Enables the shader and loads the color and texture if they exist in the shader
	/// </summary>
	void (*Draw)(Material, RenderMesh, Camera);
	/// <summary>
	/// Creates a new instance of the provided texture, disposes the old one and reassigns the main texture of the provided material
	/// </summary>
	void (*SetMainTexture)(Material, const Texture);
	/// <summary>
	/// Creates a new instance of the provided shade, disposes the old one and reassigns the main texture of the provided shader
	/// </summary>
	void (*SetShader)(Material, const Shader, size_t index);
	void (*SetColor)(Material, const Color);
	void (*SetColors)(Material, const float r, const float g, const float b, const float a);
	void (*Dispose)(Material);
};

const extern struct _materialMethods Materials;