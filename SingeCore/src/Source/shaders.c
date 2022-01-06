#include "csharp.h"
#include "graphics/shaders.h"
#include "singine/memory.h"
#include "GL/glew.h"
#include "helpers/quickmask.h"
#include "helpers/macros.h"

static bool TryGetUniform(Shader shader, Uniform uniform, int* out_handle);
static Shader Instance(Shader shader);
static void Dispose(Shader shader);
static Shader CreateShader(void);

const struct _uniforms Uniforms = {
	.MVP = {.Index = 0, .Name = UNIFORM_NAME_MVP },
	.Texture0 = {.Index = 1, .Name = UNIFORM_NAME_Texture0 },
};

const struct _shaderMethods Shaders = {
	.Create = &CreateShader,
	.Instance = &Instance,
	.TryGetUniform = &TryGetUniform,
	.Dispose = &Dispose
};

static void OnDispose(unsigned int handle)
{
	glDeleteProgram(handle);
}

static void Dispose(Shader shader)
{
	if (shader is null)
	{
		return;
	}

	// only free the uniforms if we are closing the shader program all together
	if (shader->Handle->ActiveInstances <= 1)
	{
		SafeFree(shader->Uniforms);
	}

	SharedHandles.Dispose(shader->Handle, &OnDispose);

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

	newShader->AfterDraw = null;
	newShader->BeforeDraw = null;

	return newShader;
}

static Shader CreateShader()
{
	return CreateShaderWithUniforms(true);
}

static Shader Instance(Shader shader)
{
	if (shader is null)
	{
		return null;
	}

	Shader newShader = CreateShaderWithUniforms(false);

	CopyMember(shader, newShader, Handle);

	++(shader->Handle->ActiveInstances);

	CopyMember(shader, newShader, AfterDraw);
	CopyMember(shader, newShader, BeforeDraw);

	CopyMember(shader, newShader, Uniforms);

	return newShader;
}

static bool TryGetUniform(Shader shader, Uniform uniform, int* out_handle)
{
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

