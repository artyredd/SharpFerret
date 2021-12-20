#pragma once

#include "math/vectors.h"
#include "csharp.h"

// options
typedef struct _mesh* Mesh;

struct _mesh {
	char* Name;
	bool SmoothingEnabled;
	size_t VertexCount;
	float* Vertices;
	size_t TextureVertexCount;
	float* TextureVertices;
	size_t NormalCount;
	float* Normals;
	Mesh Next;
};