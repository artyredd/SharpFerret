#include "singine/gameobjectHelpers.h"
#include "singine/memory.h"

static RenderMesh* BindMeshes(Model model)
{
	RenderMesh* meshesArray = SafeAlloc(sizeof(RenderMesh) * model->Count);

	Mesh next = model->Head;
	size_t index = 0;
	while (next != null)
	{
		if (index >= model->Count)
		{
			throw(IndexOutOfRangeException);
		}

		RenderMesh newMesh;
		if (RenderMeshes.TryBindMesh(next, &newMesh) is false)
		{
			throw(NotImplementedException);
		}

		meshesArray[index++] = newMesh;

		next = next->Next;
	}

	return meshesArray;
}

GameObject CreateGameObjectFromMesh(Mesh mesh)
{
	RenderMesh newMesh;
	if (RenderMeshes.TryBindMesh(mesh, &newMesh) is false)
	{
		throw(NotImplementedException);
	}

	GameObject parent = GameObjects.Create();

	parent->Meshes = SafeAlloc(sizeof(RenderMesh));
	parent->Count = 1;
	parent->Meshes[0] = newMesh;

	Transforms.SetParent(newMesh->Transform, parent->Transform);

	return parent;
}

GameObject LoadGameObjectFromModel(char* path, FileFormat format)
{
	Model model;
	if (Importers.TryImport(path, FileFormats.Obj, &model) is false)
	{
		throw(FailedToReadFileException);
	}

	if (model->Count is 0)
	{
		model->Dispose(model);
		throw(FailedToReadFileException);
	}

	RenderMesh* meshes = BindMeshes(model);

	GameObject parent = GameObjects.Create();

	parent->Meshes = meshes;
	parent->Count = model->Count;

	// set all the children's meshes parent to the gameobjects transform
	for (size_t i = 0; i < model->Count; i++)
	{
		Transforms.SetParent(meshes[i]->Transform, parent->Transform);
	}

	model->Dispose(model);

	return parent;
}