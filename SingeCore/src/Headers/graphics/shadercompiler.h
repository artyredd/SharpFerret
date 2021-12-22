#pragma once

#include "shaders.h"

/// <summary>
/// The standardised name that should be used to map the MVP union in shaders to memory, 
/// name must be null terminated
/// </summary>
static const char* ShaderMVPUniformName = "MVP";

bool TrySetUniform_mat4(Shader shader, int handle, void* value);

bool TryGetUniform(Shader shader, const char* name, int* out_handle);

Shader CompileShader(const char* vertexPath, const char* fragmentPath);