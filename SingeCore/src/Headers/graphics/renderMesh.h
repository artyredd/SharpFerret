#pragma once

#include "modeling/model.h"
#include "math/vectors.h"
#include "graphics/shaders.h"
#include "graphics/transform.h"
#include "graphics/sharedBuffer.h"

static unsigned int VertexShaderPosition = 0;
static unsigned int UVShaderPosition = 1;
static unsigned int NormalShaderPosition = 2;

typedef struct _renderMesh* RenderMesh;

struct _renderMesh {
	size_t Id;

	/// <summary>
	/// The shared name for all render meshes from a single model, all render meshes loaded from the same model share this reference
	/// </summary>
	char* Name;

	SharedHandle VertexBuffer;
	SharedHandle UVBuffer;
	SharedHandle NormalBuffer;

	size_t NumberOfTriangles;
	Transform Transform;
};

struct _renderMeshMethods {
	void(*Draw)(RenderMesh);
	void(*Dispose)(RenderMesh);
	// Attempts to register the mesh with the underlying graphics device
	bool (*TryBindMesh)(Mesh mesh, RenderMesh* out_mesh);
	/// <summary>
	/// Attempts to register all meshes within the model
	/// </summary>
	bool (*TryBindModel)(Model model, RenderMesh** out_meshArray);
	// Creates a new instance of the provided rendermesh with it's own transform
	RenderMesh(*Instance)(RenderMesh);
	// Creates a new instance of the provided rendermesh with it's own transform that shares the same attributes as the provided rendermesh
	RenderMesh(*Duplicate)(RenderMesh);
	// Creates a render mesh object with no buffers or populated fields
	RenderMesh(*Create)(void);
};

extern const struct _renderMeshMethods RenderMeshes;