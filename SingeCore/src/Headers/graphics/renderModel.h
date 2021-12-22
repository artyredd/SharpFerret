#pragma once

#include "modeling/model.h"
#include "math/vectors.h"
#include "graphics/shaders.h"

/// <summary>
/// The default Shader that should be used 
/// </summary>
Shader DefaultShader;

static unsigned int VertexShaderPosition = 0;
static unsigned int UVShaderPosition = 1;
static unsigned int NormalShaderPosition = 2;

typedef struct _renderMesh* RenderMesh;

struct _renderMesh {
	Shader Shader;
	unsigned int VertexBuffer;
	unsigned int UVBuffer;
	unsigned int NormalBuffer;
	size_t NumberOfTriangles;
	mat4 Transform;
	void(*Draw)(RenderMesh, mat4 position);
	void(*Dispose)(RenderMesh);
};

// Attempts to register the model with the underlying graphics device
bool TryBindMesh(Mesh mesh, RenderMesh* out_model);