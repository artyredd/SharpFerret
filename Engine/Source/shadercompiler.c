#include "core/guards.h"

#include "core/file.h"
#include "core/memory.h"
#include "GL/glew.h"
#include <stdlib.h>
#include "engine/graphics/shaders.h"
#include "engine/graphics/shadercompiler.h"
#include "core/config.h"
#include "core/quickmask.h"
#include "core/parsing.h"
#include "core/hashing.h"
#include <string.h>
#include "core/strings.h"

static Shader CompileShader(const StringArray vertexPaths, const StringArray fragmentPaths, const StringArray geometryPaths);
static Shader Load(const string path);
static bool Save(Shader shader, const string path);

const struct _shaderCompilerMethods ShaderCompilers = {
	.CompileShader = &CompileShader,
	.Load = &Load,
	.Save = &Save
};

#define LOG_BUFFER_SIZE 1024

static const char* FailedToCompileMessage = "Failed to compile the %s shader at path: %s"NEWLINE; // [Shadertype][Path]
static const char* FailedToCompileProgramMessage = "Failed to compile the shader program for shader pieces"; // [Path][Path]


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

static size_t HashShaderPaths(const StringArray vertexPaths, const StringArray fragmentPaths, const StringArray geometryPaths)
{
	size_t hash = Hashing.Hash(vertexPaths->Strings[0]);

	for (size_t i = 1; i < vertexPaths->Count; i++)
	{
		const char* string = vertexPaths->Strings[i];

		hash = Hashing.ChainHash(string, hash);
	}

	for (size_t i = 0; i < fragmentPaths->Count; i++)
	{
		const char* string = fragmentPaths->Strings[i];

		hash = Hashing.ChainHash(string, hash);
	}

	// geometry shaders are optional
	if (geometryPaths->Count isnt 0)
	{
		for (size_t i = 0; i < geometryPaths->Count; i++)
		{
			const char* string = geometryPaths->Strings[i];

			hash = Hashing.ChainHash(string, hash);
		}
	}

	return hash;
}

// checks if the combination of the two shader souce files have already been compiled
// if they are a handle is returned
static bool TryGetStoredShader(const StringArray vertexPaths, const StringArray fragmentPaths, const StringArray geometryPaths, Shader* out_shader)
{
	if (vertexPaths is null or vertexPaths->Count < 1)
	{
		return false;
	}

	if (fragmentPaths is null or fragmentPaths->Count < 1)
	{
		return false;
	}

	// hash the two paths
	size_t hash = HashShaderPaths(vertexPaths, fragmentPaths, geometryPaths);

	// mod the hash with the array size
	size_t index = hash % CompiledHandleDictionarySize;

	Shader storedShader = CompiledShaders[index];

	*out_shader = storedShader;

	return storedShader isnt null;
}

// stored the given handle within the compiled handle dictionary, returns true when no collision occurs
static bool TryStoreShader(const StringArray vertexPaths, const StringArray fragmentPaths, const StringArray geometryPaths, Shader shader)
{
	if (vertexPaths is null or vertexPaths->Count < 1)
	{
		return false;
	}

	if (fragmentPaths is null or fragmentPaths->Count < 1)
	{
		return false;
	}

	// hash the two paths
	size_t hash = HashShaderPaths(vertexPaths, fragmentPaths, geometryPaths);

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

static bool TryCompileShader(const char** dataArray, size_t count, ShaderType shaderType, unsigned int* out_handle)
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

	glShaderSource(handle, (int)count, dataArray, null);
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

static bool TryCompile(const StringArray paths, ShaderType shaderType, unsigned int* out_handle)
{
	*out_handle = 0;

	// read the file's data
	char* dataArray[MAX_SHADER_PIECES];

	for (size_t i = 0; i < paths->Count; i++)
	{
		const char* str = paths->Strings[i];

		string path = empty_stack_array(char, _MAX_PATH);

		strings.AppendCArray(path, str, Strings.Length(str));

		string data = null;
		if (Files.TryReadAll(path, &data) is false)
		{
			fprintf(stderr, "Failed to file the shader source file at path %s"NEWLINE, path->Values);

			return false;
		}

		// HACK ALERT
		// steal the underlying backing array
		dataArray[i] = data->Values;

		// this frees the container but not the underlying array
		Memory.Free(data, typeid(Array));
	}


	// compile the shader
	bool compiled = TryCompileShader(dataArray, paths->Count, shaderType, out_handle);

	for (size_t i = 0; i < paths->Count; i++)
	{
		Memory.Free(dataArray[i], typeid(char));
	}

	return compiled;
}

static bool TryCreateProgram(unsigned int* out_handle)
{
	unsigned int handle = 0;

	handle = glCreateProgram();

	*out_handle = handle;

	return handle != 0;
}

static bool TryCompileProgram(unsigned int vertexHandle, unsigned int fragmentHandle, unsigned int geometryHandle, unsigned int* out_handle)
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

	// geometry shaders are optional
	if (geometryHandle isnt 0)
	{
		glAttachShader(programHandle, geometryHandle);
	}

	// linke the shaders together
	glLinkProgram(programHandle);

	// make sure the program linked correctly
	if (VerifyProgramStatus(programHandle) is false)
	{
		return false;
	}

	// now that the whole shader has been compiled and linked we can dispose of the individual pieces we used to compile the whole thing
	glDetachShader(programHandle, vertexHandle);
	glDeleteShader(vertexHandle);

	glDetachShader(programHandle, fragmentHandle);
	glDeleteShader(fragmentHandle);

	*out_handle = programHandle;

	return true;
}

static Shader CompileShader(const StringArray vertexPaths, const StringArray fragmentPaths, const StringArray geometryPaths)
{
	GuardNotNull(vertexPaths);
	GuardNotNull(fragmentPaths);

	// trim all the paths
	StringArrays.Trim(vertexPaths);
	StringArrays.Trim(fragmentPaths);
	StringArrays.Trim(geometryPaths);

	// check if we have already compiled the provided shader pieces
	Shader shader;
	if (TryGetStoredShader(vertexPaths, fragmentPaths, geometryPaths, &shader))
	{
		return Shaders.Instance(shader);
	}

	// compile the vertex pieces
	unsigned int vertexHandle;

	if (TryCompile(vertexPaths, ShaderTypes.Vertex, &vertexHandle) is false)
	{
		fprintf(stderr, FailedToCompileMessage, "vertex", vertexPaths->Strings[0]);
		throw(FailedToCompileShaderException);
	}

	unsigned int fragmentHandle;

	if (TryCompile(fragmentPaths, ShaderTypes.Fragment, &fragmentHandle) is false)
	{
		fprintf(stderr, FailedToCompileMessage, "fragment", fragmentPaths->Strings[0]);
		throw(FailedToCompileShaderException);
	}

	unsigned int geometryHandle = 0;

	if (geometryPaths->Count > 0 && TryCompile(geometryPaths, ShaderTypes.Geometry, &geometryHandle) is false)
	{
		fprintf(stderr, FailedToCompileMessage, "geometry", geometryPaths->Strings[0]);
		throw(FailedToCompileShaderException);
	}

	// create the program using the pieces
	unsigned int programHandle;
	if (TryCompileProgram(vertexHandle, fragmentHandle, geometryHandle, &programHandle) is false)
	{
		fprintf(stderr, FailedToCompileProgramMessage);
		for (size_t i = 0; i < vertexPaths->Count; i++)
		{
			fprintf(stderr, "\t Vertex Piece: %s"NEWLINE, vertexPaths->Strings[i]);
		}

		for (size_t i = 0; i < fragmentPaths->Count; i++)
		{
			fprintf(stderr, "\t Fragment Piece: %s"NEWLINE, fragmentPaths->Strings[i]);
		}

		for (size_t i = 0; i < geometryPaths->Count; i++)
		{
			fprintf(stderr, "\t Geometry Piece: %s"NEWLINE, geometryPaths->Strings[i]);
		}

		throw(FailedToCompileShaderException);
	}

	shader = Shaders.Create();

	shader->Handle->Handle = programHandle;

	// make a copy of the provided strings
	shader->VertexPath = Strings.DuplicateTerminated(vertexPaths->Strings[0]);
	shader->FragmentPath = Strings.DuplicateTerminated(fragmentPaths->Strings[0]);

	if (geometryPaths isnt null && geometryPaths->Count > 0)
	{
		shader->GeometryPath = Strings.DuplicateTerminated(geometryPaths->Strings[0]);
	}

	// since we didnt find the shader in the dictionary store it
	if (TryStoreShader(vertexPaths, fragmentPaths, geometryPaths, shader) is false)
	{
		// this should never fail becuase in order for this block to be executed
		// we must not have previously stored the shader
		throw(UnexpectedOutcomeException);
	}

	return shader;
}

#define MaxPathLength 512
#define ArrayDelimiter ','  // comma

struct _shaderState {
	struct _stringArray FragmentPieces;
	struct _stringArray VertexPieces;
	struct _stringArray GeometryPieces;
	unsigned int Settings;
	Comparison DepthFunction;
	Comparison StencilComparison;
	CullingType CullingType;
	unsigned int StencilValue;
	unsigned int StencilMask;
	FillMode FillMode;
};

TOKEN_LOAD(vertexShader, struct _shaderState*)
{
	return Strings.TrySplit(buffer, length, ArrayDelimiter, &state->VertexPieces);
}

TOKEN_SAVE(vertexShader, Shader)
{
	if (state->VertexPath isnt null)
	{
		fprintf(stream, "%s", state->VertexPath);
	}
}

TOKEN_LOAD(fragmentShader, struct _shaderState*)
{
	return Strings.TrySplit(buffer, length, ArrayDelimiter, &state->FragmentPieces);
}

TOKEN_SAVE(fragmentShader, Shader)
{
	if (state->FragmentPath isnt null)
	{
		fprintf(stream, "%s", state->FragmentPath);
	}
}

TOKEN_LOAD(geometryShader, struct _shaderState*)
{
	return Strings.TrySplit(buffer, length, ArrayDelimiter, &state->GeometryPieces);
}

TOKEN_SAVE(geometryShader, Shader)
{
	if (state->GeometryPath isnt null)
	{
		fprintf(stream, "%s", state->GeometryPath);
	}
}

TOKEN_LOAD(culling, struct _shaderState*)
{
	if (TryGetCullingType(buffer, length, &state->CullingType))
	{
		if (state->CullingType.Value.AsUInt isnt CullingTypes.None.Value.AsUInt)
		{
			SetFlag(state->Settings, ShaderSettings.BackfaceCulling);
		}

		return true;
	}
	return false;
}

TOKEN_SAVE(culling, Shader)
{
	if (HasFlag(state->Settings, ShaderSettings.BackfaceCulling))
	{
		fprintf(stream, "%s", "true");
	}
}

TOKEN_LOAD(useCameraPerspective, struct _shaderState*)
{
	bool enabled;
	if (Parsing.TryGetBool(buffer, length, &enabled))
	{
		AssignFlag(state->Settings, ShaderSettings.UseCameraPerspective, enabled);
		return true;
	}
	return false;
}

TOKEN_SAVE(useCameraPerspective, Shader)
{
	if (HasFlag(state->Settings, ShaderSettings.UseCameraPerspective))
	{
		fprintf(stream, "%s", "true");
	}
}

TOKEN_LOAD(enableBlending, struct _shaderState*)
{
	bool enabled;
	if (Parsing.TryGetBool(buffer, length, &enabled))
	{
		AssignFlag(state->Settings, ShaderSettings.Transparency, enabled);
		return true;
	}
	return false;
}

TOKEN_SAVE(enableBlending, Shader)
{
	if (HasFlag(state->Settings, ShaderSettings.Transparency))
	{
		fprintf(stream, "%s", "true");
	}
}

TOKEN_LOAD(writeToStencilBuffer, struct _shaderState*)
{
	bool enabled;
	if (Parsing.TryGetBool(buffer, length, &enabled))
	{
		AssignFlag(state->Settings, ShaderSettings.WriteToStencilBuffer, enabled);
		return true;
	}
	return false;
}

TOKEN_SAVE(writeToStencilBuffer, Shader)
{
	if (HasFlag(state->Settings, ShaderSettings.WriteToStencilBuffer))
	{
		fprintf(stream, "%s", "true");
	}
}

TOKEN_LOAD(depthTest, struct _shaderState*)
{
	SetFlag(state->Settings, ShaderSettings.UseDepthTest);
	return TryGetComparison(buffer, length, &state->DepthFunction);
}

TOKEN_SAVE(depthTest, Shader)
{
	if (HasFlag(state->Settings, ShaderSettings.UseDepthTest))
	{
		fprintf(stream, "%s", "true");
	}
}

TOKEN_LOAD(useCustomStencilAttributes, struct _shaderState*)
{
	bool enabled;
	if (Parsing.TryGetBool(buffer, length, &enabled))
	{
		AssignFlag(state->Settings, ShaderSettings.CustomStencilAttributes, enabled);
		return true;
	}
	return false;
}

TOKEN_SAVE(useCustomStencilAttributes, Shader)
{
	if (HasFlag(state->Settings, ShaderSettings.CustomStencilAttributes))
	{
		fprintf(stream, "%s", "true");
	}
}

TOKEN_LOAD(customStencilFunction, struct _shaderState*)
{
	return TryGetComparison(buffer, length, &state->StencilComparison);
}

TOKEN_SAVE(customStencilFunction, Shader)
{
	if (HasFlag(state->Settings, ShaderSettings.CustomStencilAttributes))
	{
		fprintf(stream, "%s", state->StencilFunction.Name);
	}
}

TOKEN_LOAD(customStencilValue, struct _shaderState*)
{
	ignore_unused(length);
	int count = sscanf_s(buffer, "%xui", &(state->StencilValue));
	return count == 1;
}

TOKEN_SAVE(customStencilValue, Shader)
{
	if (HasFlag(state->Settings, ShaderSettings.CustomStencilAttributes))
	{
		fprintf(stream, "%xui", state->StencilValue);
	}
}

TOKEN_LOAD(customStencilMask, struct _shaderState*)
{
	ignore_unused(length);
	int count = sscanf_s(buffer, "%xui", &(state->StencilMask));
	return count == 1;
}

TOKEN_SAVE(customStencilMask, Shader)
{
	if (HasFlag(state->Settings, ShaderSettings.CustomStencilAttributes))
	{
		fprintf(stream, "%xui", state->StencilMask);
	}
}

TOKEN_LOAD(useStencilBuffer, struct _shaderState*)
{
	bool enabled;
	if (Parsing.TryGetBool(buffer, length, &enabled))
	{
		AssignFlag(state->Settings, ShaderSettings.UseStencilBuffer, enabled);
		return true;
	}
	return false;
}

TOKEN_SAVE(useStencilBuffer, Shader)
{
	if (HasFlag(state->Settings, ShaderSettings.UseStencilBuffer))
	{
		fprintf(stream, "%s", "true");
	}
}

TOKEN_LOAD(fillMode, struct _shaderState*)
{
	return TryGetFillMode(buffer, length, &state->FillMode);
}

TOKEN_SAVE(fillMode, Shader)
{
	fprintf(stream, "%s", state->FillMode.Name);
}

TOKENS(14)
{
	TOKEN(vertexShader, "# The paths to the vertex shader that should be used for this shader"),
		TOKEN(fragmentShader, "# The paths to the fragment shader that should be used for this shader"),
		TOKEN(geometryShader, "# the paths to any geometry shaders that should be used for this shader"),
		TOKEN(culling, "# whether or not backface culling should be enabled for this shader"),
		TOKEN(useCameraPerspective, "# whether or not this shader should use camera perspective, (GUI elements for example shouldnt)"),
		TOKEN(enableBlending, "# whether or not blending (transparency) should be enabled"),
		TOKEN(writeToStencilBuffer, "# whether or not fragments that are drawn are written to the stencil buffer"),
		TOKEN(depthTest, "# whether or not depth testing should be used when this shader is used to render an object"),
		TOKEN(useCustomStencilAttributes, "# whether or not this shader should set various stencil function attributes when it's enabled"),
		TOKEN(customStencilFunction, "# the custom function that should be used for this shader\n# does not do anythig when useComstomStencilAttributes is set to false\n# Valid Values: always, never, equal, notEqual, greaterThan, lessThan, greaterThanOrEqual, lessThanOrEqual"),
		TOKEN(customStencilValue, "# the value that a fragment's stencil buffer value should be compared to using customStencilFunction"),
		TOKEN(customStencilMask, "# the mask that should be bitwise AND'd with a fragemnt's stencil buffer value BEFORE it's compared to customStencilValue to determine if a fragment passes"),
		TOKEN(useStencilBuffer, "# whether or not the stencil buffer should be used to determine if fragments are rendered, if this is false fragments are always rendered and never write to the stencil buffer"),
		TOKEN(fillMode, "# how polygons should be drawn, fill would be normal, while line would be wireframe")
};

const struct _configDefinition ShaderConfigDefinition = {
	.Tokens = Tokens,
	.CommentCharacter = '#',
	.Count = sizeof(Tokens) / sizeof(struct _configToken)
};

static Shader Load(const string path)
{
	if (path is null)
	{
		fprintf(stderr, "Shader path was null");
		throw(InvalidArgumentException);
	}

	Shader shader = null;

	// create a place to store info needed to compile shader
	struct _shaderState state = {
		.FragmentPieces = {
			.Count = 0,
			.StringLengths = null,
			.Strings = null
		},
		.VertexPieces = {
			.Count = 0,
			.StringLengths = null,
			.Strings = null
		},
		.GeometryPieces = {
			.Count = 0,
			.StringLengths = null,
			.Strings = null
		},
		.Settings = 0,
		.DepthFunction = Comparisons.LessThan,
		.StencilComparison = Comparisons.Always,
		.StencilMask = 0xFF,
		.StencilValue = 1,
		.CullingType = CullingTypes.Back,
		.FillMode = FillModes.Fill
	};

	if (Configs.TryLoadConfig(path, (const ConfigDefinition)&ShaderConfigDefinition, &state))
	{
		// check to see if we already compiled a shader similar to this one
		shader = CompileShader(&state.VertexPieces, &state.FragmentPieces, &state.GeometryPieces);

		// if we didn't load the shader but compiled it instead, set the name to the path given
		if (shader->Name is null)
		{
			// copy the path as the shader name
			shader->Name = Strings.DuplicateTerminated(path->Values);
		}

		shader->Settings = state.Settings;

		shader->DepthFunction = state.DepthFunction;

		shader->StencilFunction = state.StencilComparison;
		shader->StencilMask = state.StencilMask;
		shader->StencilValue = state.StencilValue;

		shader->FillMode = state.FillMode;
	}

	// always free these strings, CompileShader will make copies if it needs to
	StringArrays.DisposeMembers(&state.FragmentPieces);
	StringArrays.DisposeMembers(&state.VertexPieces);
	StringArrays.DisposeMembers(&state.GeometryPieces);

	if (shader is null)
	{
		fprintf(stderr, "Failed to load the shader from path: %s"NEWLINE, path->Values);
	}

	return shader;
}

static bool Save(Shader shader, const string path)
{
	File file;
	if (Files.TryOpen(path, FileModes.Create, &file) is false)
	{
		return false;
	}

	Configs.SaveConfigStream(file, (const ConfigDefinition)&ShaderConfigDefinition, shader);

	return Files.TryClose(file);
}