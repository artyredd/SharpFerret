#pragma once

#include "shaders.h"

struct _shaderCompilerMethods {
	Shader(*CompileShader)(const char* vertexPath, const char* fragmentPath);
};

extern const struct _shaderCompilerMethods ShaderCompilers;