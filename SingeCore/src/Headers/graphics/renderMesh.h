#pragma once

#include "modeling/model.h"
#include "math/vectors.h"
#include "graphics/shaders.h"
#include "graphics/transform.h"

/// <summary>
/// The default Shader that should be used 
/// </summary>
Shader DefaultShader;

static unsigned int VertexShaderPosition = 0;
static unsigned int UVShaderPosition = 1;
static unsigned int NormalShaderPosition = 2;

typedef struct _sharedBuffer* SharedBuffer;

struct _sharedBuffer {
	// The opengl handle for this buffer
	unsigned int Handle;
	// The number of objects that reference this struct with pointers, when this reaches 0 this buffer can be reclaimed by OpenGL(freed)
	size_t ActiveInstances;
};

typedef struct _renderMesh* RenderMesh;

struct _renderMesh {
	Shader Shader;
	SharedBuffer VertexBuffer;
	SharedBuffer UVBuffer;
	SharedBuffer NormalBuffer;
	size_t NumberOfTriangles;
	Transform Transform;
	void(*Draw)(RenderMesh, mat4 position);
	void(*Dispose)(RenderMesh);
};

// Attempts to register the model with the underlying graphics device
bool TryBindMesh(Mesh mesh, RenderMesh* out_model);

// Creates a new instance of the provided rendermesh with it's own transform
RenderMesh InstanceMesh(RenderMesh);