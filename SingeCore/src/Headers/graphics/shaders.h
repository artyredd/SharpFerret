#pragma once
#include "csharp.h"
#include "math/vectors.h"

#define UNIFORM_NAME_MVP "MVP"
#define UNIFORM_NAME_Texture0 "myTextureSampler"

typedef const struct _uniform Uniform;

struct _uniform {
	const char* Name;
	unsigned int Index;
};

struct _uniforms {
	Uniform MVP;
	Uniform Texture0;
};

extern const struct _uniforms Uniforms;

struct _shaderUniforms {
	/// <summary>
	/// Mask that define which handles are available for this shader, MVp, texture, position etc.., the absence of a flag here denotes EITHER it hasn't
	/// been loaded YET or that it's not available
	/// </summary>
	unsigned int AvailableUniforms;
	/// <summary>
	/// Mask that contains flags that denote that a flag is not present in a shader AFTER atempting to load it
	/// </summary>
	unsigned int UnavailableUniforms;
	/// <summary>
	/// The Uniform handles, this array is never initialized, do not rely on it having 0's in any given index
	/// </summary>
	int Handles[16];
};

typedef struct _shader* Shader;

struct _shader {
	unsigned int Handle;
	struct _shaderUniforms Uniforms;
	/// <summary>
	/// The method that is ran before the mesh using this shader is ran, 
	/// mat4 is the MVP for the mesh being rendered, can be NULL
	/// </summary>
	void(*BeforeDraw)(Shader, mat4 mvpMatrix);
	/// <summary>
	/// Should draw the given mesh using this shader
	/// </summary>
	void(*DrawMesh)(Shader, void* RenderMesh);
	/// <summary>
	/// Invoked after any given mesh is drawn
	/// </summary>
	void(*AfterDraw)(Shader);
	/// <summary>
	/// Disposes and frees this shader and any managed resources it controls
	/// </summary>
	void(*Dispose)(Shader);
};

struct _shaderMethods {
	bool (*TryGetUniform)(Shader, Uniform, int* out_handle);
};

const extern struct _shaderMethods Shaders;

Shader CreateShader();