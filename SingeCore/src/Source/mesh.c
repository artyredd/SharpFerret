#include "modeling/mesh.h"
#include "singine/memory.h"

static Mesh CreateMesh(void);
static void Dispose(Mesh);

const struct _meshMethods Meshes = {
	.Create = &CreateMesh,
	.Dispose = &Dispose
};

TYPE_ID(Mesh);

static void Dispose(Mesh mesh)
{
	Memory.Free(mesh->VertexData, Memory.GenericMemoryBlock);
	Memory.Free(mesh->NormalVertexData, Memory.GenericMemoryBlock);
	Memory.Free(mesh->TextureVertexData, Memory.GenericMemoryBlock);
	Memory.Free(mesh->Name, Memory.String);
	Memory.Free(mesh->MaterialName, Memory.String);
	Memory.Free(mesh, MeshTypeId);
}

static Mesh CreateMesh(void)
{
	Memory.RegisterTypeName(nameof(Mesh), &MeshTypeId);

	Mesh mesh = Memory.Alloc(sizeof(struct _mesh), MeshTypeId);

	mesh->VertexCount = 0;
	mesh->MaterialName = null;
	mesh->NormalCount = 0;

	return mesh;
}