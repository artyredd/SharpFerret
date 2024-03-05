#pragma once

#include "engine/modeling/model.h"
#include "core/math/vectors.h"
#include "engine/graphics/shaders.h"
#include "engine/graphics/transform.h"
#include "engine/graphics/sharedBuffer.h"
#include "core/array.h"

static unsigned int VertexShaderPosition = 0;
static unsigned int UVShaderPosition = 1;
static unsigned int NormalShaderPosition = 2;

typedef struct _renderMesh* RenderMesh;

struct _renderMesh {
	ulong Id;

	Pointer(char) Name;

	SharedHandle VertexBuffer;
	SharedHandle UVBuffer;
	SharedHandle NormalBuffer;

	// Whether or not the mesh should be rendered smooth
	bool ShadeSmooth;

	ulong NumberOfTriangles;
	Transform Transform;

	bool CopyBuffersOnDraw;

	// The CPU Side mesh to copy over data to the GPU when
	// CopyBuffersOnDraw is enabled 
	Pointer(mesh) Mesh;
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
	void (*Save)(File, RenderMesh mesh);
};

extern const struct _renderMeshMethods RenderMeshes;