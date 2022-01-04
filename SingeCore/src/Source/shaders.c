#include "csharp.h"
#include "graphics/shaders.h"
#include "singine/memory.h"
#include "GL/glew.h"
#include "helpers/quickmask.h"

static bool TryGetUniform(Shader shader, Uniform uniform, int* out_handle);

const struct _uniforms Uniforms = {
	.MVP = {.Index = 0, .Name = UNIFORM_NAME_MVP },
	.Texture0 = {.Index = 1, .Name = UNIFORM_NAME_Texture0 },
};

const struct _shaderMethods Shaders = {
	.TryGetUniform = &TryGetUniform
};

static void Dispose(Shader shader)
{
	glDeleteProgram(shader->Handle);
	SafeFree(shader);
}

static bool TryGetUniform(Shader shader, Uniform uniform, int* out_handle)
{
	// check to see if the shader has already loaded the uniform's handle
	if (HasFlag(shader->Uniforms.AvailableUniforms, FlagN(uniform.Index)))
	{
		// if we have the flag already loaded return the handle
		*out_handle = shader->Uniforms.Handles[uniform.Index];

		return true;
	}

	*out_handle = 0;

	// check to see if we have already attempted to search for the uniform, if we have we should not try again to save resources
	if (HasFlag(shader->Uniforms.UnavailableUniforms, FlagN(uniform.Index)))
	{
		return false;
	}

	// since we don't have a flag for the shader we should attempt to load it
	int handle = glGetUniformLocation(shader->Handle, uniform.Name);

	if (handle is - 1)
	{
		SetFlag(shader->Uniforms.UnavailableUniforms, FlagN(uniform.Index));
		return false;
	}

	// Set the flag so we return the handle next time without having to attempt to load it from the shader
	SetFlag(shader->Uniforms.AvailableUniforms, FlagN(uniform.Index));

	shader->Uniforms.Handles[uniform.Index] = handle;

	*out_handle = handle;

	return true;
}

Shader CreateShader()
{
	Shader newShader = SafeAlloc(sizeof(struct _shader));

	newShader->Handle = 0;
	newShader->Dispose = &Dispose;

	ResetFlags(newShader->Uniforms.AvailableUniforms);
	ResetFlags(newShader->Uniforms.UnavailableUniforms);

	newShader->AfterDraw = null;
	newShader->BeforeDraw = null;
	newShader->DrawMesh = null;

	return newShader;
}