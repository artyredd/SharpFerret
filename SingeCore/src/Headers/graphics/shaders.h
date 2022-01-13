#pragma once
#include "csharp.h"
#include "math/vectors.h"
#include "sharedBuffer.h"

#define UNIFORM_NAME_MVP "MVP"
#define UNIFORM_NAME_Texture0 "myTextureSampler"
#define UNIFORM_NAME_Color "mainColor"

typedef const struct _uniform Uniform;

struct _uniform {
	const char* Name;
	unsigned int Index;
};

struct _uniforms {
	Uniform MVP;
	Uniform Texture0;
	Uniform Color;
};

extern const struct _uniforms Uniforms;

struct _shaderUniforms {
	/// <summary>
	/// Mask that define which handles are available for this shader, MVp, texture, position etc.., the absence of a flag here denotes EITHER it hasn't
	/// been loaded YET or that it's not available
	/// </summary>
	unsigned int AvailableUniforms;
	/// <summary>
	/// Mask that contains flags that denote that a flag is not present in a shader AFTER atempting to load it
	/// </summary>
	unsigned int UnavailableUniforms;
	/// <summary>
	/// The Uniform handles, this array is never initialized, do not rely on it having 0's in any given index
	/// </summary>
	int Handles[16];
};

typedef unsigned int ShaderSetting;

struct _shaderSettings
{
	/// <summary>
	/// Whether or not the material should cull it's back faces when it's drawn
	/// </summary>
	ShaderSetting BackfaceCulling;
	/// <summary>
	/// Whether or not the material should be rendered with blending that will allow the alpha layer to be drawn
	/// </summary>
	ShaderSetting Transparency;
	/// <summary>
	/// Whether or not the material should use the camera's perspective to change the rendered object's shape, 
	/// this should be enabled for 3d object and disabled for 2d like GUI
	/// </summary>
	ShaderSetting UseCameraPerspective;
};

extern const struct _shaderSettings ShaderSettings;

typedef struct _shader* Shader;

struct _shader {
	char* VertexPath;
	char* FragmentPath;
	SharedHandle Handle;
	struct _shaderUniforms* Uniforms;
	/// <summary>
	/// This is a quick mask that stores advanced toggles for this shader and how it should be drawn, these are applied to only this shader
	/// </summary>
	unsigned int Settings;
};

struct _shaderMethods {
	Shader(*CreateEmpty)(void);
	Shader(*Create)(void);
	/// <summary>
	/// Creates a new instance of the provided shader
	/// </summary>
	Shader(*Instance)(Shader);
	bool (*TryGetUniform)(Shader, Uniform, int* out_handle);
	void (*DisableSetting)(Shader, const ShaderSetting);
	void (*EnableSetting)(Shader, const ShaderSetting);
	void (*SetSetting)(Shader, const ShaderSetting, const bool enabled);
	bool (*HasSetting)(Shader, const ShaderSetting);
	/// <summary>
	/// Disposes and frees this shader and any managed resources it controls
	/// </summary>
	void (*Dispose)(Shader);
	void (*Enable)(Shader);
	void (*Disable)(Shader);
};

const extern struct _shaderMethods Shaders;