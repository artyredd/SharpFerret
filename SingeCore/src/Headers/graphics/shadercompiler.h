#pragma once

#include "shaders.h"

struct _shaderCompilerMethods {
	// Compiles a new shader using the default shader settings using the provided vertex and fragment paths
	Shader(*CompileShader)(const char* vertexPath, const char* fragmentPath);
	// loads a shader from the provided path
	Shader(*Load)(const char* path);
	// Saves the provided shader to the provided path
	bool (*Save)(Shader shader, const char* path);
};

extern const struct _shaderCompilerMethods ShaderCompilers;