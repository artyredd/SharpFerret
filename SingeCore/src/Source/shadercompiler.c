#include "singine/guards.h"

#include "singine/file.h"
#include "singine/memory.h"
#include "GL/glew.h"
#include <stdlib.h>
#include "graphics/shaders.h"
#include "graphics/shadercompiler.h"
#include "singine/config.h"
#include "helpers/quickmask.h"
#include "singine/parsing.h"

static Shader CompileShader(const char* vertexPath, const char* fragmentPath);
static Shader Load(const char* path);

const struct _shaderCompilerMethods ShaderCompilers = {
	.CompileShader = &CompileShader,
	.Load = &Load
};

#define LOG_BUFFER_SIZE 1024

static const char* FailedToCompileMessage = "Failed to compile the %s shader at path: %s"NEWLINE; // [Shadertype][Path]
static const char* FailedToCompileProgramMessage = "Failed to compile the shader program for shader pieces"NEWLINE"\t%s"NEWLINE"\t%s"NEWLINE; // [Path][Path]


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
	if (handle is 0)
	{
		return false;
	}

	if (handle is(unsigned int)ShaderTypes.Invalid)
	{
		return false;
	}

	return true;
}


static void PrintLog(unsigned int handle, File stream, void(*LogProvider)(unsigned int, int, int*, char*))
{
	GuardNotNull(stream);
	GuardNotNull(LogProvider);

	// since the shader failed to compile we should print the error if one exists
	int logLength = 0;

	char message[LOG_BUFFER_SIZE];

	LogProvider(handle, LOG_BUFFER_SIZE, &logLength, message);

	for (size_t i = 0; i < min(LOG_BUFFER_SIZE, logLength); i++)
	{
		int c = message[i];

		if (fputc(c, stream) != c)
		{
			fprintf(stderr, "Failed to write character to stream"NEWLINE);
			fprintf(stderr, "File stream error: %i"NEWLINE, ferror(stream));
			throw(FailedToWriteToStreamException);
		}
	}
}

static void PrintShaderLog(unsigned int handle, File stream)
{
	PrintLog(handle, stream, glGetShaderInfoLog);
}

static void PrintProgramLog(unsigned int handle, File stream)
{
	PrintLog(handle, stream, glGetProgramInfoLog);
}

static bool VerifyShaderStatus(unsigned int handle)
{
	// make sure the handle is valid
	if (VerifyHandle(handle) is false)
	{
		return false;
	}

	// make sure the shader handle we got was compiled succesfully
	int compiledStatus;

	glGetShaderiv(handle, GL_COMPILE_STATUS, &compiledStatus);

	if (compiledStatus is false)
	{
		PrintShaderLog(handle, stderr);
		return false;
	}

	return true;
}

static bool TryCompileShader(const char* data, ShaderType shaderType, unsigned int* out_handle)
{
	// default set the out variable to 0, so if we return early we don't accidently give a wierd handle that may not exist
	*out_handle = 0;

	// get a handle for the new shader
	unsigned int handle = glCreateShader(shaderType);

	// make sure the handle is valid
	if (VerifyHandle(handle) is false)
	{
		glDeleteShader(handle);

		return false;
	}

	glShaderSource(handle, 1, &data, null);
	glCompileShader(handle);

	if (VerifyShaderStatus(handle) is false)
	{
		return false;
	}

	*out_handle = handle;

	return true;
}

static bool VerifyProgramStatus(unsigned int handle)
{
	int compiled = false;

	glGetProgramiv(handle, GL_LINK_STATUS, &compiled);

	if (compiled is false)
	{
		PrintProgramLog(handle, stderr);
	}

	return compiled;
}

static bool TryCompile(const char* path, ShaderType shaderType, unsigned int* out_handle)
{
	*out_handle = 0;

	// read the file's data
	char* vertexData;

	if (Files.TryReadAll(path, &vertexData) is false)
	{
		return false;
	}

	// compile the shader
	bool compiled = TryCompileShader(vertexData, shaderType, out_handle);

	SafeFree(vertexData);

	return compiled;
}

static bool TryCreateProgram(unsigned int* out_handle)
{
	unsigned int handle = 0;

	handle = glCreateProgram();

	*out_handle = handle;

	return handle != 0;
}

static bool TryCompileProgram(unsigned int vertexHandle, unsigned int fragmentHandle, unsigned int* out_handle)
{
	// default the out handle to 0
	// we don't want to set this to non-zero unless we are successfull
	*out_handle = 0;

	unsigned int programHandle;
	if (TryCreateProgram(&programHandle) is false)
	{
		return false;
	}

	// attach the shaders
	glAttachShader(programHandle, vertexHandle);
	glAttachShader(programHandle, fragmentHandle);

	// linke the shaders together
	glLinkProgram(programHandle);

	// make sure the program linked correctly
	if (VerifyProgramStatus(programHandle) is false)
	{
		return false;
	}

	// now that the whole shader has been compiled and linked we can dispose of the individual pieces we used to compile the whole thing
	glDetachShader(programHandle, vertexHandle);
	glDetachShader(programHandle, fragmentHandle);

	glDeleteShader(vertexHandle);
	glDeleteShader(vertexHandle);

	*out_handle = programHandle;

	return true;
}

static Shader CompileShader(const char* vertexPath, const char* fragmentPath)
{
	GuardNotNull(vertexPath);
	GuardNotNull(fragmentPath);

	// compile the vertex shader
	unsigned int vertexHandle;
	if (TryCompile(vertexPath, ShaderTypes.Vertex, &vertexHandle) is false)
	{
		fprintf(stderr, FailedToCompileMessage, "vertex", vertexPath);
		throw(FailedToCompileShaderException);
	}

	// compile the fragment shader
	unsigned int fragmentHandle;
	if (TryCompile(fragmentPath, ShaderTypes.Fragment, &fragmentHandle) is false)
	{
		fprintf(stderr, FailedToCompileMessage, "fragment", fragmentPath);
		throw(FailedToCompileShaderException);
	}

	unsigned int programHandle;
	if (TryCompileProgram(vertexHandle, fragmentHandle, &programHandle) is false)
	{
		fprintf(stderr, FailedToCompileProgramMessage, vertexPath, fragmentHandle);
		throw(FailedToCompileShaderException);
	}


	Shader shader = Shaders.Create();

	shader->Handle->Handle = programHandle;

	return shader;
}


#define MaxPathLength 512

struct _shaderInfo {
	char* FragmentPath;
	char* VertexPath;
	unsigned int Settings;
};

#define TokenCount 5

#define VertexShaderToken "vertexShader"
#define FragmentShaderToken "fragmentShader"
#define UseBackfaceCullingToken "enableBackfaceCulling"
#define UseCameraPerspectiveToken "useCameraPerspective"
#define UseTransparencyToken "enableBlending"

static const char* Tokens[TokenCount] = {
	VertexShaderToken,
	FragmentShaderToken,
	UseBackfaceCullingToken,
	UseCameraPerspectiveToken,
	UseTransparencyToken
};

static const size_t TokenLengths[TokenCount] = {
	sizeof(VertexShaderToken),
	sizeof(FragmentShaderToken),
	sizeof(UseBackfaceCullingToken),
	sizeof(UseCameraPerspectiveToken),
	sizeof(UseTransparencyToken),
};

static bool OnTokenFound(size_t index, const char* buffer, const size_t length, struct _shaderInfo* state)
{
	bool enabled;
	switch (index)
	{
	case 0: // vertex path
		return TryParseString(buffer, length, MaxPathLength, &state->VertexPath);
	case 1: // fragment path
		return TryParseString(buffer, length, MaxPathLength, &state->FragmentPath);
	case 2: // backface culling
		if (TryParseBoolean(buffer, length, &enabled))
		{
			AssignFlag(state->Settings, ShaderSettings.BackfaceCulling, enabled);
			return true;
		}
		return false;
	case 3: // use camera perspective
		if (TryParseBoolean(buffer, length, &enabled))
		{
			AssignFlag(state->Settings, ShaderSettings.UseCameraPerspective, enabled);
			return true;
		}
		return false;
	case 4: // use transparency
		if (TryParseBoolean(buffer, length, &enabled))
		{
			AssignFlag(state->Settings, ShaderSettings.Transparency, enabled);
			return true;
		}
		return false;
	default:
		return false;
	}
}

const struct _configDefinition ShaderConfigDefinition = {
	.Tokens = (const char**)&Tokens,
	.TokenLengths = (const size_t*)&TokenLengths,
	.CommentCharacter = '#',
	.Count = sizeof(Tokens) / sizeof(&Tokens), // tokens is an array of pointers to the total bytes/sizeof pointer is the count
	.OnTokenFound = &OnTokenFound
};

static Shader Load(const char* path)
{
	// create a place to store info needed to compile shader
	struct _shaderInfo info = {
		.Settings = 0
	};

	if (Configs.TryLoadConfig(path, (const ConfigDefinition)&ShaderConfigDefinition, &info))
	{
		Shader compiledShader = CompileShader(info.VertexPath, info.FragmentPath);

		compiledShader->FragmentPath = info.FragmentPath;
		compiledShader->VertexPath = info.VertexPath;

		compiledShader->Settings = info.Settings;

		return compiledShader;
	}

	SafeFree(info.FragmentPath);
	SafeFree(info.VertexPath);

	fprintf(stderr, "Filed to load the shader from path: %s", path);

	return null;
}