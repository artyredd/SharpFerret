#pragma once

#include "csharp.h"

// options
typedef struct _mesh* Mesh;

struct _mesh {
	char* Name;
	bool SmoothingEnabled;
	// THE NUMBER OF FLOATS IN VERTICES JESUS CHRIST THIS TOOK ME AND AJAY HOURS TO FIGURE OUT
	size_t VertexCount;
	float* VertexData;
	// THE NUMBER OF FLOATS IN TextureVertices JESUS CHRIST THIS TOOK ME AND AJAY HOURS TO FIGURE OUT
	size_t TextureCount;
	float* TextureVertexData;
	// THE NUMBER OF FLOATS IN Normals JESUS CHRIST THIS TOOK ME AND AJAY HOURS TO FIGURE OUT
	size_t NormalCount;
	float* NormalVertexData;
	char* MaterialName;
};

struct _meshMethods {
	Mesh(*Create)(void);
	void (*Dispose)(Mesh mesh);
};

extern const struct _meshMethods Meshes;