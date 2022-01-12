#pragma once

#include "graphics/shaders.h"
#include "graphics/texture.h"
#include "graphics/renderMesh.h"
#include "graphics/camera.h"
#include "graphics/colors.h"

struct _materialState
{
	/// <summary>
	/// This is a quick mask that stores advanced toggles for this material and how it should be drawn, these are applied to all shaders that are used
	/// for this material
	/// </summary>
	unsigned int Settings;
};

typedef struct _material* Material;

struct _material {
	Shader* Shaders;
	size_t Count;
	Color Color;
	Texture MainTexture;
	struct _materialState State;
};

struct _materialMethods {
	Material(*CreateMaterial)(void);
	Material(*Create)(const Shader, const Texture);
	Material(*Load)(const char* path);
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
	void (*DisableSetting)(Material, const ShaderSetting);
	void (*EnableSetting)(Material, const ShaderSetting);
	void (*SetSetting)(Material, const ShaderSetting, const bool enabled);
	bool (*HasSetting)(Material, const ShaderSetting);
	void (*Dispose)(Material);
};

const extern struct _materialMethods Materials;