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

static Shader CompileShader(const StringArray vertexPaths, const StringArray fragmentPaths, const StringArray geometryPaths);
static Shader Load(const char* path);
static bool Save(Shader shader, const char* path);

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
		const char* path = paths->Strings[i];

		if (Files.TryReadAll(path, &dataArray[i]) is false)
		{
			fprintf(stderr, "Failed to file the shader source file at path %s"NEWLINE, path);

			return false;
		}
	}


	// compile the shader
	bool compiled = TryCompileShader(dataArray, paths->Count, shaderType, out_handle);

	for (size_t i = 0; i < paths->Count; i++)
	{
		Memory.Free(dataArray[i], Memory.GenericMemoryBlock);
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

struct _shaderInfo {
	struct _stringArray FragmentPieces;
	struct _stringArray VertexPieces;
	struct _stringArray GeometryPieces;
	unsigned int Settings;
	Comparison DepthFunction;
	Comparison StencilComparison;
	CullingType CullingType;
	unsigned int StencilValue;
	unsigned int StencilMask;
};

#define ArrayDelimiter ','  // comma

#define ExportTokenFormat "\n%s: %s\n"

#define VertexShaderComment "# The paths to the vertex shader that should be used for this shader"
#define VertexShaderToken "vertexShader"
#define FragmentShaderComment "# The paths to the fragment shader that should be used for this shader"
#define FragmentShaderToken "fragmentShader"
#define GeometryShaderComment "# the paths to any geometry shaders that should be used for this shader"
#define GeometryShaderToken "geometryShader"
#define UseBackfaceCullingComment "# whether or not backface culling should be enabled for this shader"
#define UseBackfaceCullingToken "culling"
#define UseCameraPerspectiveComment "# whether or not this shader should use camera perspective, (GUI elements for example shouldnt)"
#define UseCameraPerspectiveToken "useCameraPerspective"
#define UseTransparencyComment "# whether or not blending (transparency) should be enabled"
#define UseTransparencyToken "enableBlending"
#define WriteToStencilBufferComment "# whether or not fragments that are drawn are written to the stencil buffer"
#define WriteToStencilBufferToken "writeToStencilBuffer"
#define UseDepthTestComment "# whether or not depth testing should be used when this shader is used to render an object"
#define UseDepthTestToken "depthTest"
#define UseCustomStencilAttributesComment "# whether or not this shader should set various stencil function attributes when it's enabled"
#define UseCustomStencilAttributesToken "useCustomStencilAttributes"
#define CustomStencilFuncionComment "# the custom function that should be used for this shader\n# does not do anythig when useComstomStencilAttributes is set to false\n# Valid Values: always, never, equal, notEqual, greaterThan, lessThan, greaterThanOrEqual, lessThanOrEqual"
#define CustomStencilFuncionToken "customStencilFunction"
#define CustomStencilValueComment "# the value that a fragment's stencil buffer value should be compared to using customStencilFunction"
#define CustomStencilValueToken "customStencilValue"
#define CustomStencilMaskComment "# the mask that should be bitwise AND'd with a fragemnt's stencil buffer value BEFORE it's compared to customStencilValue to determine if a fragment passes"
#define CustomStencilMaskToken "customStencilMask"
#define UseStencilBufferComment "# whether or not the stencil buffer should be used to determine if fragments are rendered, if this is false fragments are always rendered and never write to the stencil buffer"
#define UseStencilBufferToken "useStencilBuffer"

static const char* Tokens[] = {
	VertexShaderToken,
	FragmentShaderToken,
	UseBackfaceCullingToken,
	UseCameraPerspectiveToken,
	UseTransparencyToken,
	UseDepthTestToken,
	UseStencilBufferToken,
	WriteToStencilBufferToken,
	UseCustomStencilAttributesToken,
	CustomStencilFuncionToken,
	CustomStencilValueToken,
	CustomStencilMaskToken,
	GeometryShaderToken,
};

static const size_t TokenLengths[] = {
	sizeof(VertexShaderToken),
	sizeof(FragmentShaderToken),
	sizeof(UseBackfaceCullingToken),
	sizeof(UseCameraPerspectiveToken),
	sizeof(UseTransparencyToken),
	sizeof(UseDepthTestToken),
	sizeof(UseStencilBufferToken),
	sizeof(WriteToStencilBufferToken),
	sizeof(UseCustomStencilAttributesToken),
	sizeof(CustomStencilFuncionToken),
	sizeof(CustomStencilValueToken),
	sizeof(CustomStencilMaskToken),
	sizeof(GeometryShaderToken)
};

static bool OnTokenFound(size_t index, const char* buffer, const size_t length, struct _shaderInfo* state)
{
	bool enabled;
	int count;
	switch (index)
	{
	case 0: // vertex path
		return Strings.TrySplit(buffer, length, ArrayDelimiter, &state->VertexPieces);
	case 1: // fragment path
		return Strings.TrySplit(buffer, length, ArrayDelimiter, &state->FragmentPieces);
	case 2: // backface culling
		if (TryGetCullingType(buffer, length, &state->CullingType))
		{
			if(state->CullingType.Value.AsUInt isnt CullingTypes.None.Value.AsUInt)
			{
				SetFlag(state->Settings, ShaderSettings.BackfaceCulling);
			}

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
	case 5: // enable depth testing
		SetFlag(state->Settings, ShaderSettings.UseDepthTest);
		return TryGetComparison(buffer, length, &state->DepthFunction);
	case 6: // use stencil buffer
		if (TryParseBoolean(buffer, length, &enabled))
		{
			AssignFlag(state->Settings, ShaderSettings.UseStencilBuffer, enabled);
			return true;
		}
		return false;
	case 7: // write to stencil buffer
		if (TryParseBoolean(buffer, length, &enabled))
		{
			AssignFlag(state->Settings, ShaderSettings.WriteToStencilBuffer, enabled);
			return true;
		}
		return false;
	case 8: // use custom stencil values
		if (TryParseBoolean(buffer, length, &enabled))
		{
			AssignFlag(state->Settings, ShaderSettings.CustomStencilAttributes, enabled);
			return true;
		}
		return false;
	case 9: // custom stencil function
		return TryGetComparison(buffer, length, &state->StencilComparison);
	case 10: // stencil value
		count = sscanf_s(buffer, "%xui", &(state->StencilValue));
		return count == 1;
	case 11: // stencil mask
		count = sscanf_s(buffer, "%xui", &(state->StencilMask));
		return count == 1;
	case 12: // fragment path
		return Strings.TrySplit(buffer, length, ArrayDelimiter, &state->GeometryPieces);
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
		.CullingType = CullingTypes.Back
	};

	if (Configs.TryLoadConfig(path, (const ConfigDefinition)&ShaderConfigDefinition, &info))
	{
		// check to see if we already compiled a shader similar to this one
		shader = CompileShader(&info.VertexPieces, &info.FragmentPieces, &info.GeometryPieces);

		// if we didn't load the shader but compiled it instead, set the name to the path given
		if (shader->Name is null)
		{
			// copy the path as the shader name
			shader->Name = Strings.DuplicateTerminated(path);
		}

		shader->Settings = info.Settings;

		shader->DepthFunction = info.DepthFunction;

		shader->StencilFunction = info.StencilComparison;
		shader->StencilMask = info.StencilMask;
		shader->StencilValue = info.StencilValue;
	}

	// always free these strings, CompileShader will make copies if it needs to
	StringArrays.DisposeMembers(&info.FragmentPieces);
	StringArrays.DisposeMembers(&info.VertexPieces);
	StringArrays.DisposeMembers(&info.GeometryPieces);

	if (shader is null)
	{
		fprintf(stderr, "Failed to load the shader from path: %s"NEWLINE, path);
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

	if (HasFlag(shader->Settings, ShaderSettings.UseDepthTest))
	{
		fprintf(file, UseDepthTestComment);
		fprintf(file, ExportTokenFormat, UseDepthTestToken, "true");
	}

	if (HasFlag(shader->Settings, ShaderSettings.UseStencilBuffer))
	{
		fprintf(file, UseStencilBufferComment);
		fprintf(file, ExportTokenFormat, UseStencilBufferToken, "true");
	}

	if (HasFlag(shader->Settings, ShaderSettings.WriteToStencilBuffer))
	{
		fprintf(file, WriteToStencilBufferComment);
		fprintf(file, ExportTokenFormat, WriteToStencilBufferToken, "true");
	}

	if (HasFlag(shader->Settings, ShaderSettings.CustomStencilAttributes))
	{
		fprintf(file, UseCustomStencilAttributesComment);
		fprintf(file, ExportTokenFormat, UseCustomStencilAttributesToken, "true");

		fprintf(file, CustomStencilFuncionComment);
		fprintf(file, ExportTokenFormat, CustomStencilFuncionToken, shader->StencilFunction.Name);

		fprintf(file, CustomStencilValueComment);
		fprintf(file, "\n%s: %xui\n", CustomStencilValueToken, shader->StencilValue);

		fprintf(file, CustomStencilMaskComment);
		fprintf(file, "\n%s: %xui\n", CustomStencilMaskToken, shader->StencilMask);
	}

	return Files.TryClose(file);
}