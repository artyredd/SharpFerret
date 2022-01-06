#pragma once

#include "csharp.h"

// options
typedef struct _mesh* Mesh;

struct _mesh {
	char* Name;
	bool SmoothingEnabled;
	size_t VertexCount;
	float* Vertices;
	size_t TextureCount;
	float* TextureVertices;
	size_t NormalCount;
	float* Normals;
	Mesh Next;
};

struct _meshMethods {
	Mesh(*Create)(void);
	void (*Dispose)(Mesh mesh);
};

extern const struct _meshMethods Meshes;