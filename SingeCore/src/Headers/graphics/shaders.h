#pragma once
#include "singine/enumerable.h"

typedef struct _shader* Shader;

struct _shader {
	unsigned int Handle;
	void(*Dispose)(Shader);
};
//
//const struct _shaderMethods {
//	Shader(*Compile)(const char* vertexPath, const char* fragmentPath);
//} sShader;

Shader CompileShader(const char* vertexPath, const char* fragmentPath);