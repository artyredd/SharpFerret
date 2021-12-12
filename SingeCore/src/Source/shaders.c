#include "csharp.h"
#include "graphics/shaders.h"

#include "io/file.h"
#include "singine/memory.h"
#include "GL/glew.h"

#define LOG_BUFFER_SIZE 1024

typedef int ShaderType;

static struct _shaderTypes
{
	ShaderType none;
	ShaderType Invalid;
	ShaderType Vertex;
	ShaderType Fragment;
	ShaderType Compute;
	ShaderType TessControl;
	ShaderType TessEvaluation;
	ShaderType Geometry;
} ShaderTypes = {
	0,
	GL_INVALID_ENUM,
	GL_VERTEX_SHADER,
	GL_FRAGMENT_SHADER,
	GL_COMPUTE_SHADER,
	GL_TESS_CONTROL_SHADER,
	GL_TESS_EVALUATION_SHADER,
	GL_GEOMETRY_SHADER
};

static bool VerifyHandle(unsigned int handle)
{
	if (handle is null)
	{
		return false;
	}

	if (handle is 0)
	{
		return false;
	}

	if (handle is ShaderTypes.Invalid)
	{
		return false;
	}

	return true;
}

static bool TryCompileShader(const char* data, ShaderType shaderType)
{
	// get a handle for the new shader
	unsigned int handle = glCreateShader(shaderType);

	// make sure the handle is valid
	if (VerifyHandle(handle) is false)
	{
		glDeleteShader(handle);

		return false;
	}

	glGetShaderSource(handle, 1, data, null);
	glCompileShader(handle);
}

static bool VerifyShaderStatus(unsigned int handle)
{
	if (VerifyHandle(handle) is false)
	{
		return false;
	}

	unsigned int compiledStatus;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &compiledStatus);

	if (compiledStatus != true)
	{
		size_t logLength = 0;

		char message[LOG_BUFFER_SIZE];

	}
}

static bool TryCreateProgram(unsigned int* out_handle)
{

}

Shader Compile(const char* vertexPath, const char* fragmentPath)
{
	char* vertexData;

	if (TryReadAll(vertexPath, &vertexData) is false)
	{
		fprintf(stderr, "Failed to open vertex shader path %s"NEWLINE, vertexPath);
		throw(FailedToReadFileException);
	}

	char* fragmentData;

	if (TryReadAll(vertexPath, &fragmentData) is false)
	{
		fprintf(stderr, "Failed to open fragment shader path %s"NEWLINE, vertexPath);

		SafeFree(vertexData);

		throw(FailedToReadFileException);
	}



	SafeFree(vertexData);
	SafeFree(fragmentData);
}