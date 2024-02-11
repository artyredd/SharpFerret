#pragma once

#include "shaders.h"
#include "core/strings.h"

// The maximum number of vertex and fragment shaders may be contained within a single shader
#define MAX_SHADER_PIECES 128

struct _shaderCompilerMethods {
	// Compiles a new shader using the default shader settings using the provided vertex and fragment paths
	Shader(*CompileShader)(const StringArray vertexPaths, const StringArray fragmentPaths, const StringArray geometryPaths);
	// loads a shader from the provided path
	Shader(*Load)(const string path);
	// Saves the provided shader to the provided path
	bool (*Save)(Shader shader, const string path);
};

extern const struct _shaderCompilerMethods ShaderCompilers;