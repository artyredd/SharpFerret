#pragma once

#include "shaders.h"

struct _shaderCompilerMethods {
	bool (*TrySetUniform_mat4)(Shader shader, int handle, void* value);

	bool (*TryGetUniform)(Shader shader, const char* name, int* out_handle);

	Shader(*CompileShader)(const char* vertexPath, const char* fragmentPath);
};

extern const struct _shaderCompilerMethods ShaderCompilers;