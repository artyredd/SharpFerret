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

const struct _uniforms Uniforms = {
	.MVP = {.Index = 0, .Name = UNIFORM_NAME_MVP },
	.Texture0 = {.Index = 1, .Name = UNIFORM_NAME_Texture0 },
	.Color = {.Index = 2, .Name = UNIFORM_NAME_Color },
	.Ambient = {.Index = 3, .Name = UNIFORM_NAME_AmbientColor},
	.Specular = {.Index = 4, .Name = UNIFORM_NAME_SpecularColor},
	.SpecularMap = {.Index = 5, .Name = UNIFORM_NAME_SpecularMap},
	.CameraPosition = {.Index = 6, .Name = UNIFORM_NAME_CameraPosition },
	.ModelMatrix = {.Index = 7, .Name = UNIFORM_NAME_ModelMatrix },
	.Diffuse = {.Index = 8, .Name = UNIFORM_NAME_DiffuseColor },
	.LightCount = {.Index = 10, .Name = UNIFORM_NAME_LightCount },
	.ViewMatrix = {.Index = 11, .Name = UNIFORM_NAME_ViewMatrix },
	.ProjectionMatrix = {.Index = 12, .Name = UNIFORM_NAME_ProjectionMatrix },
	.Shininess = {.Index = 13, .Name = UNIFORM_NAME_Shininess },
	.Lights = {.Index = 100, .Name = UNIFORM_NAME_LightsArray, .Size = (sizeof(struct _lightUniforms) / sizeof(struct _uniform)) },
	.LightFields = {
		 .Type = {.Index = 0, .Name = "lightType" },
		 .Ambient = {.Index = 1, .Name = "ambient" },
		 .Diffuse = {.Index = 2, .Name = "diffuse" },
		 .Specular = {.Index = 3, .Name = "specular" },
		 .Range = {.Index = 4, .Name = "range" },
		 .Radius = {.Index = 5, .Name = "radius" },
		 .Position = {.Index = 6, .Name = "position" }
	}
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
	.TryGetUniformArrayField = &TryGetUniformArrayField
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
	SafeFree(shader->Uniforms);
	SafeFree(shader->VertexPath);
	SafeFree(shader->FragmentPath);
	SafeFree(shader->Name);

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

	SafeFree(shader);
}

static Shader CreateShaderWithUniforms(bool allocUniforms)
{
	Shader newShader = SafeAlloc(sizeof(struct _shader));

	if (allocUniforms)
	{
		newShader->Handle = SharedHandles.Create();

		newShader->Uniforms = SafeAlloc(sizeof(struct _shaderUniforms));

		ZeroArray(newShader->Uniforms->Handles, MAX_UNIFORMS);
	}

	newShader->Settings = DEFAULT_SHADER_SETTINGS;

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

	// references
	CopyMember(shader, newShader, Name);
	CopyMember(shader, newShader, VertexPath);
	CopyMember(shader, newShader, FragmentPath);
	CopyMember(shader, newShader, Handle);
	CopyMember(shader, newShader, StencilFunction);
	CopyMember(shader, newShader, StencilValue);
	CopyMember(shader, newShader, StencilMask);

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