#pragma once

#include "graphics/shaders.h"
#include "graphics/texture.h"
#include "graphics/renderMesh.h"
#include "graphics/camera.h"

typedef unsigned int MaterialSetting;

struct _materialSettings
{
	/// <summary>
	/// Whether or not the material should cull it's back faces when it's drawn
	/// </summary>
	MaterialSetting BackfaceCulling;
	/// <summary>
	/// Whether or not the material should be rendered with blending that will allow the alpha layer to be drawn
	/// </summary>
	MaterialSetting Transparency;
	/// <summary>
	/// Whether or not the material should use the camera's perspective to change the rendered object's shape, 
	/// this should be enabled for 3d object and disabled for 2d like GUI
	/// </summary>
	MaterialSetting UseCameraPerspective;
};

extern const struct _materialSettings MaterialSettings;

struct _materialState
{
	/// <summary>
	/// This is a quick mask that stores advanced toggles for this material and how it should be drawn
	/// </summary>
	unsigned int Settings;
};

typedef struct _material* Material;

struct _material {
	Shader Shader;
	vec4 Color;
	Texture MainTexture;
	struct _materialState State;
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
	void (*Draw)(Material, RenderMesh, Camera);
	/// <summary>
	/// Creates a new instance of the provided texture, disposes the old one and reassigns the main texture of the provided material
	/// </summary>
	void (*SetMainTexture)(Material, Texture);
	/// <summary>
	/// Creates a new instance of the provided shade, disposes the old one and reassigns the main texture of the provided shader
	/// </summary>
	void (*SetShader)(Material, Shader);
	void (*DisableSetting)(Material, MaterialSetting);
	void (*EnableSetting)(Material, MaterialSetting);
	void (*SetSetting)(Material, MaterialSetting, bool enabled);
	bool (*HasSetting)(Material, MaterialSetting);
	void (*Dispose)(Material);
};

const extern struct _materialMethods Materials;