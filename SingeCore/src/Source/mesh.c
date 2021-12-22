#include "modeling/mesh.h"
#include "singine/memory.h"
#include "cglm/mat4.h"

Mesh CreateMesh()
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