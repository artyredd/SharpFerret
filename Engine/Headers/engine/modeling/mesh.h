#pragma once

#include "core/csharp.h"
#include "core/math/vectors.h"
#include "core/array.h"

// options
typedef struct _mesh* Mesh;
typedef struct _mesh mesh;

struct _mesh {
	char* Name;
	bool SmoothingEnabled;
	// The number of vector3s in the VertexData array
	ulong VertexCount;
	vector3* Vertices;
	// The number of vector2 in the TextureVertexData array
	ulong TextureCount;
	vector2* TextureVertices;
	// The number of vector3 in the NormalVertexData array
	ulong NormalCount;
	vector3* NormalVertices;
	char* MaterialName;
};

struct _meshMethods {
	Mesh(*Create)(void);
	void (*Dispose)(Mesh mesh);
};

extern const struct _meshMethods Meshes;

#pragma warning(disable:4113)
DEFINE_CONTAINERS(mesh);
DEFINE_CONTAINERS(Mesh);