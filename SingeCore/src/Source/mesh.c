#include "modeling/mesh.h"
#include "singine/memory.h"
#include "cglm/mat4.h"

static Mesh CreateMesh(void);
static void Dispose(Mesh);

const struct _meshMethods Meshes = {
	.Create = &CreateMesh,
	.Dispose = &Dispose
};

static void Dispose(Mesh mesh)
{
	Memory.Free(mesh->Vertices);
	Memory.Free(mesh->Normals);
	Memory.Free(mesh->TextureVertices);
	Memory.Free(mesh->Name);
	Memory.Free(mesh->MaterialName);
	Memory.Free(mesh);
}

static Mesh CreateMesh(void)
{
	Mesh mesh = Memory.Alloc(sizeof(struct _mesh));

	mesh->VertexCount = 0;
	mesh->MaterialName = null;
	mesh->NormalCount = 0;

	return mesh;
}