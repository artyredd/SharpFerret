#pragma once
#include "csharp.h"
#include "math/vectors.h"
#include "sharedBuffer.h"
#include "graphics/graphicsDevice.h"

#define UNIFORM_NAME_MVP "MVP"
#define UNIFORM_NAME_ProjectionMatrix "projectionMatrix"
#define UNIFORM_NAME_ViewMatrix "viewMatrix"
#define UNIFORM_NAME_ModelMatrix "modelMatrix"
#define UNIFORM_NAME_Texture0 "myTextureSampler"
#define UNIFORM_NAME_Color "mainColor"
#define UNIFORM_NAME_SpecularColor "specularColor"
#define UNIFORM_NAME_AmbientColor "ambientColor"
#define UNIFORM_NAME_SpecularMap "specularMap"
#define UNIFORM_NAME_CameraPosition "cameraPosition"
#define UNIFORM_NAME_DiffuseColor "diffuseColor"
#define UNIFORM_NAME_LightsArray "Lights"
#define UNIFORM_NAME_LightCount "LightCount"
#define UNIFORM_NAME_Shininess "shininess"

typedef const struct _uniform Uniform;

struct _uniform {
	const char* Name;
	unsigned int Index;
};

struct _uniforms {
	Uniform ViewMatrix;
	Uniform ProjectionMatrix;
	Uniform MVP;
	Uniform Texture0;
	Uniform Color;
	Uniform Specular;
	Uniform Ambient;
	Uniform SpecularMap;
	Uniform CameraPosition;
	Uniform ModelMatrix;
	Uniform Diffuse;
	Uniform Lights;
	Uniform LightCount;
	Uniform Shininess;
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

#define MAX_SHADER_SETTINGS 9

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
	/// <summary>
	/// Whether or not depth testing is used when this shader is used to render an object
	/// </summary>
	ShaderSetting UseDepthTest;
	/// <summary>
	/// Whether or not the fragments rendered by this shader should write or read to the stencil buffer, when this is disabled
	/// all fragments are drawn and no fragments are written to the stencil buffer
	/// </summary>
	ShaderSetting UseStencilBuffer;
	/// <summary>
	/// Whether or not fragments that pass the shader's buffer function should be written to the stencil buffer
	/// </summary>
	ShaderSetting WriteToStencilBuffer;
	/// <summary>
	/// Whether or not the stencil attributes, function, comparison value, and mask should be set to custom values when this shader is enabled
	/// </summary>
	ShaderSetting CustomStencilAttributes;
};

extern const struct _shaderSettings ShaderSettings;

typedef struct _shader* Shader;

struct _shader {
	char* Name;
	char* VertexPath;
	char* FragmentPath;
	SharedHandle Handle;
	struct _shaderUniforms* Uniforms;
	/// <summary>
	/// This is a quick mask that stores advanced toggles for this shader and how it should be drawn, these are applied to only this shader
	/// </summary>
	unsigned int Settings;
	/// <summary>
	/// If UseStencilBuffer within the settings is true then this is used to compare a fragments value with StencilValue to determine whether
	/// a fragment is drawn
	/// </summary>
	Comparison StencilFunction;
	/// <summary>
	/// The value each fragment should be compared to using the stencil function, if the result is true the fragment is rendered, otherwise it's not
	/// </summary>
	unsigned int StencilValue;
	/// <summary>
	/// The mask that should be AND'd with the fragment's stencil buffer value before it's compared with StencilComparisonValue to determine if it should
	/// be rendered
	/// </summary>
	unsigned int StencilMask;
};

struct _shaderMethods {
	Shader(*CreateEmpty)(void);
	Shader(*Create)(void);
	/// <summary>
	/// Creates a new instance of the provided shader
	/// </summary>
	Shader(*Instance)(Shader);
	bool (*TryGetUniform)(Shader, Uniform, int* out_handle);
	bool (*TryGetUniformArray)(Shader, Uniform, size_t index, int* out_handle);
	bool (*TryGetUniformArrayField)(Shader, Uniform, size_t index, const char* field, int* out_handle);
	void (*DisableSetting)(Shader, const ShaderSetting);
	void (*EnableSetting)(Shader, const ShaderSetting);
	void (*SetSetting)(Shader, const ShaderSetting, const bool enabled);
	bool (*HasSetting)(Shader, const ShaderSetting);
	/// <summary>
	/// Disposes and frees this shader and any managed resources it controls
	/// </summary>
	void (*Dispose)(Shader);
	/// <summary>
	/// Enables the provided shader on the graphics device
	/// </summary>
	void (*Enable)(Shader);
	/// <summary>
	/// Disables the shader on the graphics device
	/// </summary>
	void (*Disable)(Shader);
};

const extern struct _shaderMethods Shaders;