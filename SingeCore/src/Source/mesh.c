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
	SafeFree(mesh->Name);
	SafeFree(mesh->MaterialName);
	SafeFree(mesh);
}

static Mesh CreateMesh(void)
{
	Mesh mesh = SafeAlloc(sizeof(struct _mesh));

	mesh->VertexCount = 0;
	mesh->MaterialName = null;
	mesh->NormalCount = 0;

	return mesh;
}