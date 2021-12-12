#pragma once
#include "singine/enumerable.h"

typedef struct _shader* Shader;

struct _shader {
	unsigned int Handle;
};
//
//const struct _shaderMethods {
//	Shader(*Compile)(const char* vertexPath, const char* fragmentPath);
//} sShader;

Shader Compile(const char* vertexPath, const char* fragmentPath);