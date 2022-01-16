#include "csharp.h"
#include "graphics/shaders.h"
#include "singine/memory.h"
#include "GL/glew.h"
#include "helpers/quickmask.h"
#include "helpers/macros.h"
#include "singine/config.h"
#include "singine/parsing.h"
#include "singine/guards.h"


static void DisableSetting(Shader shader, ShaderSetting setting);
static void EnableSetting(Shader shader, ShaderSetting setting);
static void SetSetting(Shader shader, ShaderSetting setting, bool enabled);
static bool HasSetting(Shader shader, ShaderSetting setting);
static bool TryGetUniform(Shader shader, Uniform uniform, int* out_handle);
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
	.HasSetting = &HasSetting
};

#define UseCameraPerspectiveFlag FLAG_0
#define UseCullingFlag FLAG_1
#define UseTransparencyFlag FLAG_2

#define DEFAULT_SHADER_SETTINGS 0 //(UseCameraPerspectiveFlag | UseCullingFlag)

const struct _shaderSettings ShaderSettings = {
	.UseCameraPerspective = UseCameraPerspectiveFlag,
	.BackfaceCulling = UseCullingFlag,
	.Transparency = UseTransparencyFlag
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

		newShader->Uniforms = SafeAlloc(sizeof(struct _uniforms));

		ResetFlags(newShader->Uniforms->AvailableUniforms);
		ResetFlags(newShader->Uniforms->UnavailableUniforms);
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

	// if the shader were given is an empty shader that doesn't have a handle we dont need to increment the instance count
	if (shader->Handle isnt null)
	{
		++(shader->Handle->ActiveInstances);
	}

	return newShader;
}

static bool TryGetUniform(Shader shader, Uniform uniform, int* out_handle)
{
	if (shader is null) return false;

	// check to see if the shader has already loaded the uniform's handle
	if (HasFlag(shader->Uniforms->AvailableUniforms, FlagN(uniform.Index)))
	{
		// if we have the flag already loaded return the handle
		*out_handle = shader->Uniforms->Handles[uniform.Index];

		return true;
	}

	*out_handle = 0;

	// check to see if we have already attempted to search for the uniform, if we have we should not try again to save resources
	if (HasFlag(shader->Uniforms->UnavailableUniforms, FlagN(uniform.Index)))
	{
		return false;
	}

	// since we don't have a flag for the shader we should attempt to load it
	int handle = glGetUniformLocation(shader->Handle->Handle, uniform.Name);

	if (handle is - 1)
	{
		SetFlag(shader->Uniforms->UnavailableUniforms, FlagN(uniform.Index));
		return false;
	}

	// Set the flag so we return the handle next time without having to attempt to load it from the shader
	SetFlag(shader->Uniforms->AvailableUniforms, FlagN(uniform.Index));

	shader->Uniforms->Handles[uniform.Index] = handle;

	*out_handle = handle;

	return true;
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