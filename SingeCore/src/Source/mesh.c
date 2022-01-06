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
	SafeFree(mesh->Vertices);
	SafeFree(mesh->Normals);
	SafeFree(mesh->TextureVertices);
	SafeFree(mesh);
}

static Mesh CreateMesh(void)
{
	Mesh mesh = SafeAlloc(sizeof(struct _mesh));

	mesh->Name = null;

	mesh->Normals = null;
	mesh->Vertices = null;
	mesh->TextureVertices = null;

	mesh->NormalCount = 0;
	mesh->VertexCount = 0;
	mesh->TextureCount = 0;

	mesh->Next = null;

	return mesh;
}