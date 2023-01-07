#include "csharp.h"
#include "graphics/shaders.h"
#include "singine/memory.h"
#include "GL/glew.h"
#include "helpers/quickmask.h"
#include "helpers/macros.h"
#include "singine/config.h"
#include "singine/parsing.h"
#include "singine/guards.h"
#include <stdlib.h>
#include <Headers/graphics/texture.h>


static void DisableSetting(Shader shader, ShaderSetting setting);
static void EnableSetting(Shader shader, ShaderSetting setting);
static void SetSetting(Shader shader, ShaderSetting setting, bool enabled);
static bool HasSetting(Shader shader, ShaderSetting setting);
static bool TryGetUniform(Shader shader, Uniform uniform, int* out_handle);
static bool TryGetUniformArray(Shader, Uniform, size_t index, int* out_handle);
static bool TryGetUniformArrayField(Shader shader, Uniform uniform, size_t index, Uniform field, int* out_handle);
static Shader Instance(Shader shader);
static void Dispose(Shader shader);
static Shader CreateShader(void);
static Shader CreateEmpty(void);
static void Enable(Shader);
static void Disable(Shader);
static bool UniformSetVector2(Shader shader, Uniform uniform, vec2 value);
static bool UniformSetVector3(Shader shader, Uniform uniform, vec3 value);
static bool UniformSetVector4(Shader shader, Uniform uniform, vec4 value);
static bool UniformSetMatrix(Shader shader, Uniform uniform, mat4 value);
static bool UniformSetFloat(Shader shader, Uniform uniform, float value);
static bool UniformSetInt(Shader shader, Uniform uniform, int value);
static bool UniformFieldSetInt(Shader shader, Uniform uniform, size_t index, Uniform field, int value);
static bool UniformFieldSetVector2(Shader shader, Uniform uniform, size_t index, Uniform field, vec2 value);
static bool UniformFieldSetFloat(Shader shader, Uniform uniform, size_t index, Uniform field, float value);
static bool UniformFieldSetVector3(Shader shader, Uniform uniform, size_t index, Uniform field, vec3 value);
static bool UniformFieldSetVector4(Shader shader, Uniform uniform, size_t index, Uniform field, vec4 value);
static bool UniformFieldSetMatrix(Shader shader, Uniform uniform, size_t index, Uniform field, mat4 value);
static void SetTextureUniform(Shader shader, Uniform uniform, Texture texture, unsigned int slot);

const struct _uniforms Uniforms = {
	.MVP = {.Index = 0, .Name = "MVP" },
	.ModelMatrix = {.Index = 1, .Name = "model" },
	.ViewMatrix = {.Index = 2, .Name = "view" },
	.ProjectionMatrix = {.Index = 3, .Name = "projection" },
	.CameraPosition = {.Index = 4, .Name = "cameraPosition" },
	.LightCount = {.Index = 5, .Name = "LightCount" },
	.Light = {
		 .Type = {.Index = 0, .Name = "lightType" },
		 .Ambient = {.Index = 1, .Name = "ambient" },
		 .Diffuse = {.Index = 2, .Name = "diffuse" },
		 .Specular = {.Index = 3, .Name = "specular" },
		 .Range = {.Index = 4, .Name = "range" },
		 .Radius = {.Index = 5, .Name = "radius" },
		 .Position = {.Index = 6, .Name = "position" },
		 .Direction = {.Index = 7, .Name = "direction" },
		 .EdgeSoftness = {.Index = 8, .Name = "edgeSoftness" },
		 .Enabled = {.Index = 9, .Name = "enabled" },
		 .Intensity = {.Index = 10, .Name = "intensity"}
	},
	.Material = {
		.DiffuseMap = {.Index = 6, .Name = "material.diffuseMap" },
		.SpecularMap = {.Index = 7, .Name = "material.specularMap" },
		.Color = {.Index = 8, .Name = "material.color"},
		.Ambient = {.Index = 9, .Name = "material.ambient"},
		.Diffuse = {.Index = 10, .Name = "material.diffuse" },
		.Specular = {.Index = 11, .Name = "material.specular"},
		.Shininess = {.Index = 12, .Name = "material.shininess" },
		.UseDiffuseMap = {.Index = 13, .Name = "material.useDiffuseMap" },
		.UseSpecularMap = {.Index = 14, .Name = "material.useSpecularMap" },
		.UseReflectionMap = { .Index = 15, .Name = "material.useReflectionMap"},
		.ReflectionMap = {.Index = 16, .Name = "material.reflectionMap"},
		.UseAreaMap = {.Index = 17, .Name = "material.useAreaMap"},
		.AreaMap = {.Index = 18, .Name = "material.areaMap"},
		.Reflectivity = {.Index = 19, .Name = "material.reflectivity"},
	},
	.LightViewMatrix = {.Index = 20, .Name = "LightViewMatrix" },
	.LightShadowMaps = {.Index = 21, .Name = "LightShadowMaps" },
	.Lights = {
		.Index = 22 + MAX_LIGHTS,
		.Name = "Lights",
		.Size = (sizeof(struct _lightUniforms) / sizeof(struct _uniform)),
		.Count = MAX_LIGHTS
	},
};

const struct _shaderMethods Shaders = {
	.Create = &CreateShader,
	.Instance = &Instance,
	.TryGetUniform = &TryGetUniform,
	.Dispose = &Dispose,
	.CreateEmpty = &CreateEmpty,
	.Enable = &Enable,
	.Disable = &Disable,
	.DisableSetting = &DisableSetting,
	.EnableSetting = &EnableSetting,
	.SetSetting = &SetSetting,
	.HasSetting = &HasSetting,
	.TryGetUniformArray = &TryGetUniformArray,
	.TryGetUniformArrayField = &TryGetUniformArrayField,
	.SetVector2 = &UniformSetVector2,
	.SetVector3 = &UniformSetVector3,
	.SetVector4 = &UniformSetVector4,
	.SetMatrix = &UniformSetMatrix,
	.SetFloat = &UniformSetFloat,
	.SetInt = &UniformSetInt,
	.SetArrayFieldVector2 = &UniformFieldSetVector2,
	.SetArrayFieldVector3 = &UniformFieldSetVector3,
	.SetArrayFieldVector4 = &UniformFieldSetVector4,
	.SetArrayFieldMatrix = &UniformFieldSetMatrix,
	.SetArrayFieldFloat = &UniformFieldSetFloat,
	.SetArrayFieldInt = &UniformFieldSetInt,
};

// the boolean flags are stored in a bit mask
#define UseCameraPerspectiveFlag	FLAG_0
#define UseCullingFlag				FLAG_1
#define UseTransparencyFlag			FLAG_2
#define WriteToStencilBufferFlag	FLAG_3
#define UseDepthTestFlag			FLAG_4
#define UseStencilBufferFlag		FLAG_5
#define CustomStencilAttributesFlag FLAG_6

#define DEFAULT_SHADER_SETTINGS 0

TYPE_ID(ShaderUniforms);
TYPE_ID(Shader);

const struct _shaderSettings ShaderSettings = {
	.UseCameraPerspective = UseCameraPerspectiveFlag,
	.BackfaceCulling = UseCullingFlag,
	.Transparency = UseTransparencyFlag,
	.WriteToStencilBuffer = WriteToStencilBufferFlag,
	.UseDepthTest = UseDepthTestFlag,
	.CustomStencilAttributes = CustomStencilAttributesFlag,
	.UseStencilBuffer = UseStencilBufferFlag
};

static void OnDispose(Shader shader)
{
	REGISTER_TYPE(Shader);
	REGISTER_TYPE(ShaderUniforms);

	Memory.Free(shader->Uniforms, ShaderUniformsTypeId);
	Memory.Free(shader->VertexPath, Memory.String);
	Memory.Free(shader->FragmentPath, Memory.String);
	Memory.Free(shader->GeometryPath, Memory.String);
	Memory.Free(shader->Name, Memory.String);

	if (shader->Handle->Handle > 0)
	{
		glDeleteProgram(shader->Handle->Handle);
	}
}

static void Dispose(Shader shader)
{
	if (shader is null)
	{
		return;
	}

	// see OnDispose
	SharedHandles.Dispose(shader->Handle, shader, &OnDispose);

	Memory.Free(shader, ShaderTypeId);
}

static Shader CreateShaderWithUniforms(bool allocUniforms)
{
	Memory.RegisterTypeName(nameof(Shader), &ShaderTypeId);

	Shader newShader = Memory.Alloc(sizeof(struct _shader), ShaderTypeId);

	if (allocUniforms)
	{
		newShader->Handle = SharedHandles.Create();

		newShader->Uniforms = Memory.Alloc(sizeof(struct _shaderUniforms), ShaderUniformsTypeId);

		Memory.ZeroArray(newShader->Uniforms->Handles, MAX_UNIFORMS);
	}

	newShader->Settings = DEFAULT_SHADER_SETTINGS;
	newShader->Enabled = true;
	newShader->CullingType = DEFAULT_CULLING_TYPE;

	return newShader;
}

static Shader CreateShader()
{
	return CreateShaderWithUniforms(true);
}

static Shader CreateEmpty()
{
	return  CreateShaderWithUniforms(false);
}

static Shader Instance(Shader shader)
{
	if (shader is null)
	{
		return null;
	}

	Shader newShader = CreateShaderWithUniforms(false);

	// value types
	CopyMember(shader, newShader, Uniforms);
	CopyMember(shader, newShader, Settings);
	CopyMember(shader, newShader, Enabled);

	// references
	CopyMember(shader, newShader, Name);
	CopyMember(shader, newShader, VertexPath);
	CopyMember(shader, newShader, FragmentPath);
	CopyMember(shader, newShader, Handle);
	CopyMember(shader, newShader, StencilFunction);
	CopyMember(shader, newShader, StencilValue);
	CopyMember(shader, newShader, StencilMask);
	CopyMember(shader, newShader, DepthFunction);

	// if the shader were given is an empty shader that doesn't have a handle we dont need to increment the instance count
	if (shader->Handle isnt null)
	{
		++(shader->Handle->ActiveInstances);
	}

	return newShader;
}

static bool TryGetUniform(Shader shader, Uniform uniform, int* out_handle)
{
	// no point in trying to check if a null shader contains a uniform
	if (shader is null) return false;

	// all indices of the handle array start as zero, if it's zero then it needs to be fetched
	int result = shader->Uniforms->Handles[uniform.Index];

	// check to see if we should fetch the handle
	if (result is 0)
	{
		// fetch the handle and set it in the array
		result = glGetUniformLocation(shader->Handle->Handle, uniform.Name);
		shader->Uniforms->Handles[uniform.Index] = result;
	}

	// set the out handle
	*out_handle = result;

	// if -1 was returned then the handle was either not found, or was compiled-away by GLSL for non-use of the uniform
	return result isnt - 1;
}

static bool TryGetUniformArray(Shader shader, Uniform uniform, size_t index, int* out_handle)
{
	// no point in trying to check if a null shader contains a uniform
	if (shader is null) return false;

	// get the stored handle
	int* handle = &shader->Uniforms->Handles[uniform.Index + index];

	*out_handle = *handle;

	// a handle of -1 denotes a non-existent uniform
	if (*handle is 0)
	{
#define UniformBufferSize 1024

		char buffer[UniformBufferSize];

		// compose the uniform
		sprintf_s(buffer, UniformBufferSize, "%s[%lli]", uniform.Name, index);
		*handle = glGetUniformLocation(shader->Handle->Handle, buffer);
	}

	return *handle isnt - 1;
}

static bool TryGetUniformArrayField(Shader shader, Uniform uniform, size_t index, Uniform field, int* out_handle)
{
	// no point in trying to check if a null shader contains a uniform
	if (shader is null) return false;

	// all indices of the handle array start as zero, if it's zero then it needs to be fetched
	int* handles = shader->Uniforms->Handles + uniform.Index;

	// the uniform is the array name
	// size is the number of fields in the struct
	// so the i'th struct is index * number of members
	size_t offset = index * uniform.Size;

	// the field.index is the i'th field of the struct
	int* result = handles + offset + field.Index;

	// check to see if we should fetch the handle
	if (*result is 0)
	{
		char buffer[1024];

		// compose the uniform
		sprintf_s(buffer, UniformBufferSize, "%s[%lli].%s", uniform.Name, index, field.Name);
		*result = glGetUniformLocation(shader->Handle->Handle, buffer);
	}

	// set the out handle
	*out_handle = *result;

	// if -1 was returned then the handle was either not found, or was compiled-away by GLSL for non-use of the uniform
	return *result isnt - 1;
}

static void Enable(Shader shader)
{
	glUseProgram(shader->Handle->Handle);
}

#pragma warning(disable: 4100)
static void Disable(Shader shader) {/* reserved */ }
#pragma warning(default: 4100)

static void DisableSetting(Shader shader, ShaderSetting setting)
{
	GuardNotNull(shader);

	ClearFlag(shader->Settings, setting);
}

static void EnableSetting(Shader shader, ShaderSetting setting)
{
	GuardNotNull(shader);

	SetFlag(shader->Settings, setting);
}

static void SetSetting(Shader shader, ShaderSetting setting, bool enabled)
{
	GuardNotNull(shader);

	AssignFlag(shader->Settings, setting, enabled);
}

static bool HasSetting(Shader shader, ShaderSetting setting)
{
	GuardNotNull(shader);

	return HasFlag(shader->Settings, setting);
}

static void SetTextureUniform(Shader shader, Uniform uniform, Texture texture, unsigned int slot)
{
	if (texture isnt null)
	{
		int uniformHandle;
		if (Shaders.TryGetUniform(shader, uniform, &uniformHandle))
		{
			GraphicsDevice.ActivateTexture(texture->Type, texture->Handle->Handle, uniformHandle, slot);
		}
	}
}


#define SetUniformBase(uniformGetter,method) int handle;\
if (uniformGetter)\
{\
	method;\
	return true;\
}\
return false;

#define SetUniformMacro(method) SetUniformBase(Shaders.TryGetUniform(shader, uniform, &handle), method)
#define SetUniformFieldMacro(method) SetUniformBase(Shaders.TryGetUniformArrayField(shader, uniform, index, field, &handle), method)

static bool UniformSetVector2(Shader shader, Uniform uniform, vec2 value)
{
	SetUniformMacro(glUniform2fv(handle, 1, value));
}

static bool UniformSetVector3(Shader shader, Uniform uniform, vec3 value)
{
	SetUniformMacro(glUniform3fv(handle, 1, value));
}

static bool UniformSetVector4(Shader shader, Uniform uniform, vec4 value)
{
	SetUniformMacro(glUniform4fv(handle, 1, value));
}

static bool UniformSetMatrix(Shader shader, Uniform uniform, mat4 value)
{
	SetUniformMacro(glUniformMatrix4fv(handle, 1, false, &value[0][0]));
}

static bool UniformSetFloat(Shader shader, Uniform uniform, float value)
{
	SetUniformMacro(glUniform1f(handle, value));
}

static bool UniformSetInt(Shader shader, Uniform uniform, int value)
{
	SetUniformMacro(glUniform1i(handle, value));
}

static bool UniformFieldSetInt(Shader shader, Uniform uniform, size_t index, Uniform field, int value)
{
	SetUniformFieldMacro(glUniform1i(handle, value));
}

static bool UniformFieldSetFloat(Shader shader, Uniform uniform, size_t index, Uniform field, float value)
{
	SetUniformFieldMacro(glUniform1f(handle, value));
}

static bool UniformFieldSetVector2(Shader shader, Uniform uniform, size_t index, Uniform field, vec2 value)
{
	SetUniformFieldMacro(glUniform2fv(handle, 1, value));
}

static bool UniformFieldSetVector3(Shader shader, Uniform uniform, size_t index, Uniform field, vec3 value)
{
	SetUniformFieldMacro(glUniform3fv(handle, 1, value));
}

static bool UniformFieldSetVector4(Shader shader, Uniform uniform, size_t index, Uniform field, vec4 value)
{
	SetUniformFieldMacro(glUniform4fv(handle, 1, value));
}

static bool UniformFieldSetMatrix(Shader shader, Uniform uniform, size_t index, Uniform field, mat4 value)
{
	SetUniformFieldMacro(glUniformMatrix4fv(handle, 1, false, &value[0][0]));
}
