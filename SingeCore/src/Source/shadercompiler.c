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
#include "singine/hashing.h"
#include <string.h>
#include "singine/strings.h"

static Shader CompileShader(const char* vertexPath, const char* fragmentPath);
static Shader Load(const char* path);
static bool Save(Shader shader, const char* path);

const struct _shaderCompilerMethods ShaderCompilers = {
	.CompileShader = &CompileShader,
	.Load = &Load,
	.Save = &Save
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

#define CompiledHandleDictionarySize 1024

Shader CompiledShaders[CompiledHandleDictionarySize];
/// <summary>
/// The number of handles within the compiled handles dict
/// </summary>
size_t HandleDictionaryCount = 0;
size_t HandleDictionaryLength = CompiledHandleDictionarySize;

// checks if the combination of the two shader souce files have already been compiled
// if they are a handle is returned
static bool TryGetStoredShader(const char* vertexPath, const char* fragmentPath, Shader* out_shader)
{
	// hash the two paths
	size_t hash = Hashing.Hash(vertexPath);

	hash = Hashing.ChainHash(fragmentPath, hash);

	// mod the hash with the array size
	size_t index = hash % CompiledHandleDictionarySize;

	Shader storedShader = CompiledShaders[index];

	*out_shader = storedShader;

	return storedShader isnt null;
}

// stored the given handle within the compiled handle dictionary, returns true when no collision occurs
static bool TryStoreShader(const char* vertexPath, const char* fragmentPath, Shader shader)
{
	// hash the two paths
	size_t hash = Hashing.Hash(vertexPath);

	hash = Hashing.ChainHash(fragmentPath, hash);

	// mod the hash with the array size
	size_t index = hash % CompiledHandleDictionarySize;

	Shader* position = &CompiledShaders[index];

	// this may cause issues with disposed handles, we'll see
	if (*position is null)
	{
		*position = shader;

		return true;
	}

	return false;
}

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

	// check to see if we have already compiled these shaders
	Shader shader;
	if (TryGetStoredShader(vertexPath, fragmentPath, &shader))
	{
		return Shaders.Instance(shader);
	}

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

	shader = Shaders.Create();

	shader->Handle->Handle = programHandle;

	// make a copy of the provided strings
	shader->VertexPath = Strings.DuplicateTerminated(vertexPath);
	shader->FragmentPath = Strings.DuplicateTerminated(fragmentPath);

	// since we didnt find the shader in the dictionary store it
	if (TryStoreShader(vertexPath, fragmentPath, shader) is false)
	{
		// this should never fail becuase in order for this block to be executed
		// we must not have previously stored the shader
		throw(UnexpectedOutcomeException);
	}

	return shader;
}


#define MaxPathLength 512

struct _shaderInfo {
	char* FragmentPath;
	char* VertexPath;
	unsigned int Settings;
};

#define TokenCount 5

#define ExportTokenFormat "\n%s: %s\n"

#define VertexShaderComment "# The path to the vertex shader that should be used for this shader"
#define VertexShaderToken "vertexShader"
#define FragmentShaderComment "# The path to the fragment shader that should be used for this shader"
#define FragmentShaderToken "fragmentShader"
#define UseBackfaceCullingComment "# whether or not backface culling should be enabled for this shader"
#define UseBackfaceCullingToken "enableBackfaceCulling"
#define UseCameraPerspectiveComment "# whether or not this shader should use camera perspective, (GUI elements for example shouldnt)"
#define UseCameraPerspectiveToken "useCameraPerspective"
#define UseTransparencyComment "# whether or not blending (transparency) should be enabled"
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
	if (path is null)
	{
		fprintf(stderr, "Shader path was null");
		throw(InvalidArgumentException);
	}

	Shader shader = null;

	// create a place to store info needed to compile shader
	struct _shaderInfo info = {
		.FragmentPath = null,
		.VertexPath = null,
		.Settings = 0
	};

	if (Configs.TryLoadConfig(path, (const ConfigDefinition)&ShaderConfigDefinition, &info))
	{
		// check to see if we already compiled a shader similar to this one
		shader = CompileShader(info.VertexPath, info.FragmentPath);

		// if we didn't load the shader but compiled it instead, set the name to the path given
		if (shader->Name is null)
		{
			// copy the path as the shader name
			shader->Name = Strings.DuplicateTerminated(path);
		}

		shader->Settings = info.Settings;
	}

	// always free these strings, CompileShader will make copies if it needs to
	SafeFree(info.FragmentPath);
	SafeFree(info.VertexPath);

	if (shader is null)
	{
		fprintf(stderr, "Failed to load the shader from path: %s", path);
	}

	return shader;
}

static bool Save(Shader shader, const char* path)
{
	File file;
	if (Files.TryOpen(path, FileModes.Create, &file) is false)
	{
		return false;
	}

	fprintf(file, VertexShaderComment);
	fprintf(file, ExportTokenFormat, VertexShaderToken, shader->VertexPath);

	fprintf(file, FragmentShaderComment);
	fprintf(file, ExportTokenFormat, FragmentShaderToken, shader->FragmentPath);

	if (HasFlag(shader->Settings, ShaderSettings.BackfaceCulling))
	{
		fprintf(file, UseBackfaceCullingComment);
		fprintf(file, ExportTokenFormat, UseBackfaceCullingToken, "true");
	}

	if (HasFlag(shader->Settings, ShaderSettings.UseCameraPerspective))
	{
		fprintf(file, UseCameraPerspectiveComment);
		fprintf(file, ExportTokenFormat, UseCameraPerspectiveToken, "true");
	}

	if (HasFlag(shader->Settings, ShaderSettings.Transparency))
	{
		fprintf(file, UseTransparencyComment);
		fprintf(file, ExportTokenFormat, UseTransparencyToken, "true");
	}

	return Files.TryClose(file);
}