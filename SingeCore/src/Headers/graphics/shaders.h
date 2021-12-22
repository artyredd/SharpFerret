#pragma once
#include "csharp.h"
#include "math/vectors.h"

typedef struct _shader* Shader;

struct _shader {
	unsigned int Handle;
	int MVPHandle;
	/// <summary>
	/// The method that is ran before the mesh using this shader is ran, 
	/// mat4 is the MVP for the mesh being rendered, can be NULL
	/// </summary>
	void(*BeforeDraw)(Shader,mat4 mvpMatrix);
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

Shader CreateShader();