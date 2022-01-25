#pragma once

#include "graphics/shaders.h"
#include "graphics/texture.h"
#include "graphics/renderMesh.h"
#include "graphics/camera.h"
#include "graphics/colors.h"
#include "graphics/scene.h"

typedef struct _material* Material;

struct _material {
	/// <summary>
	/// The array of shaders that should be used to render this material
	/// </summary>
	Shader* Shaders;
	/// <summary>
	/// The number of shaders within this material
	/// </summary>
	size_t Count;
	/// <summary>
	/// The ambient color of this material
	/// </summary>
	Color AmbientColor;
	/// <summary>
	/// The base color for this material, default is Colors.White
	/// </summary>
	Color Color;
	/// <summary>
	/// The diffuse color of the object
	/// </summary>
	Color DiffuseColor;
	/// <summary>
	/// The specular color of this material
	/// </summary>
	Color SpecularColor;
	/// <summary>
	/// How reflective the material should be
	/// </summary>
	float Shininess;
	/// <summary>
	/// The main texture for the object, this is synonomous with TEXTURE0 and is typically the UV texture for the object
	/// </summary>
	Texture MainTexture;
	/// <summary>
	/// The specular texture that should be used to define what parts of an object should receive
	/// specular lighting
	/// </summary>
	Texture SpecularTexture;
	/// <summary>
	/// The name, or path of this material
	/// </summary>
	char* Name;
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
	void (*Draw)(Material material, RenderMesh mesh, Scene scene);
	/// <summary>
	/// Creates a new instance of the provided texture, disposes the old one and reassigns the main texture of the provided material
	/// </summary>
	void (*SetMainTexture)(Material, const Texture);
	/// <summary>
	/// Creates a new instance of the provided texture, disposes the old one and reassigns the main texture of the provided material
	/// </summary>
	void (*SetSpecularTexture)(Material, const Texture);
	/// <summary>
	/// Creates a new instance of the provided shade, disposes the old one and reassigns the main texture of the provided shader
	/// </summary>
	void (*SetShader)(Material, const Shader, size_t index);
	void (*SetColor)(Material, const Color);
	void (*SetColors)(Material, const float r, const float g, const float b, const float a);
	/// <summary>
	/// Sets the name of this material, freeing the previous one if it exists
	/// </summary>
	void (*SetName)(Material, const char* name);
	void (*Dispose)(Material);
};

const extern struct _materialMethods Materials;