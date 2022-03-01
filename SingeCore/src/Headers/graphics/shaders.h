#pragma once
#include "csharp.h"
#include "math/vectors.h"
#include "sharedBuffer.h"
#include "graphics/graphicsDevice.h"
#include "graphics/texture.h"

#define DEFAULT_CULLING_TYPE CullingTypes.Back

#define MAX_UNIFORMS 1024
#define MAX_POINT_LIGHTS 50

// represents a uniform within a shader and the information used to load it and cache it effectively
typedef const struct _uniform Uniform;

struct _uniform {
	// the name of the uniform, this can be full or partial, depending on which method is used to try to load the uniform
	const char* Name;
	// the index within the handle array that this uniform is stored(for single uniforms) or the start index of many of this uniform(for count > 0)
	unsigned int Index;
	// the number of uniforms supported of this kind, for an array with a length of 100 this count would be 100, for single uniforms this is 0 or 1
	unsigned int Count;
	// the width in integers for a single uniform of this type, for a struct uniform with 5 fields, the length of this would be 5(one integer per field)
	unsigned int Size;
};

struct _lightUniforms {
	Uniform Type;
	Uniform Ambient;
	Uniform Diffuse;
	Uniform Specular;
	Uniform Range;
	Uniform Radius;
	Uniform Position;
	Uniform Direction;
	Uniform EdgeSoftness;
	Uniform Enabled;
	Uniform Intensity;
};

struct _materialUniforms {
	Uniform Color;
	Uniform Ambient;
	Uniform Diffuse;
	Uniform Specular;
	Uniform Shininess;
	Uniform DiffuseMap;
	Uniform SpecularMap;
	Uniform UseDiffuseMap;
	Uniform UseSpecularMap;
	Uniform UseReflectionMap;
	Uniform ReflectionMap;
	Uniform UseAreaMap;
	Uniform AreaMap;
	Uniform Reflectivity;
};

struct _uniforms {
	Uniform ViewMatrix;
	Uniform ProjectionMatrix;
	Uniform MVP;
	Uniform CameraPosition;
	Uniform ModelMatrix;
	Uniform Lights;
	struct _lightUniforms Light;
	Uniform LightCount;
	struct _materialUniforms Material;
	Uniform LightShadowMaps;
	Uniform LightShadowCubeMaps;
	Uniform LightMatrices;
	Uniform LightCubmapMatrices;
};

// Global uniforms likely to be widely used across many shaders to provide basic functionality
extern const struct _uniforms Uniforms;

struct _shaderUniforms {
	int Handles[MAX_UNIFORMS];
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
	// Whether or not this shader should be used
	bool Enabled;
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
	/// The type of culling type that should be used when this shader is enabled
	/// </summary>
	Comparison CullingType;
	Comparison DepthFunction;
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
	bool (*TryGetUniformArrayField)(Shader, Uniform, size_t index, Uniform field, int* out_handle);
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
	bool(*SetVector2)(Shader, Uniform, vec2 value);
	bool(*SetVector3)(Shader, Uniform, vec3 value);
	bool(*SetVector4)(Shader, Uniform, vec4 value);
	bool(*SetMatrix)(Shader, Uniform, mat4 value);
	bool(*SetFloat)(Shader, Uniform, float value);
	bool(*SetInt)(Shader, Uniform, int value);
	bool (*SetArrayFieldInt)(Shader shader, Uniform uniform, size_t index, Uniform field, int value);
	bool (*SetArrayFieldVector2)(Shader shader, Uniform uniform, size_t index, Uniform field, vec2 value);
	bool (*SetArrayFieldFloat)(Shader shader, Uniform uniform, size_t index, Uniform field, float value);
	bool (*SetArrayFieldVector3)(Shader shader, Uniform uniform, size_t index, Uniform field, vec3 value);
	bool (*SetArrayFieldVector4)(Shader shader, Uniform uniform, size_t index, Uniform field, vec4 value);
	bool (*SetArrayFieldMatrix)(Shader shader, Uniform uniform, size_t index, Uniform field, mat4 value);
	bool(*SetTexture)(Shader shader, Uniform uniform, Texture texture, unsigned int slot);
};

const extern struct _shaderMethods Shaders;