#pragma once

#include "core/csharp.h"
#include "core/math/vectors.h"

// options
typedef struct _mesh* Mesh;

struct _mesh {
	char* Name;
	bool SmoothingEnabled;
	// The number of vector3s in the VertexData array
	size_t VertexCount;
	vector3* Vertices;
	// The number of vector2 in the TextureVertexData array
	size_t TextureCount;
	vector2* TextureVertices;
	// The number of vector3 in the NormalVertexData array
	size_t NormalCount;
	vector3* NormalVertices;
	char* MaterialName;
};

struct _meshMethods {
	Mesh(*Create)(void);
	void (*Dispose)(Mesh mesh);
};

extern const struct _meshMethods Meshes;