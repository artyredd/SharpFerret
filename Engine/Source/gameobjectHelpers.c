#include "engine/gameobjectHelpers.h"
#include "core/memory.h"

TYPE_ID(GameObjectMeshes);

GameObject CreateGameObjectFromMesh(Mesh mesh)
{
	RenderMesh newMesh;
	if (RenderMeshes.TryBindMesh(mesh, &newMesh) is false)
	{
		throw(NotImplementedException);
	}

	GameObject parent = GameObjects.Create();

	parent->Meshes = Memory.Alloc(sizeof(RenderMesh), GameObjectMeshesTypeId);
	parent->Count = 1;
	parent->Meshes[0] = newMesh;

	Transforms.SetParent(newMesh->Transform, parent->Transform);

	return parent;
}

GameObject CreateFromRenderMesh(RenderMesh mesh)
{
	GameObject parent = GameObjects.Create();

	Memory.RegisterTypeName("GameObjectMeshes", &GameObjectMeshesTypeId);

	parent->Meshes = Memory.Alloc(sizeof(RenderMesh), GameObjectMeshesTypeId);
	parent->Count = 1;
	parent->Meshes[0] = RenderMeshes.Instance(mesh);;

	Transforms.SetParent(parent->Meshes[0]->Transform, parent->Transform);

	return parent;
}

GameObject LoadGameObjectFromModel(char* path, FileFormat format)
{
	Model model;
	if (Importers.TryImport(path, format, &model) is false)
	{
		throw(FailedToReadFileException);
	}

	if (model->Count is 0)
	{
		Models.Dispose(model);
		throw(FailedToReadFileException);
	}

	RenderMesh* meshes;
	if (RenderMeshes.TryBindModel(model, &meshes) is false)
	{
		Models.Dispose(model);
		throw(FailedToBindMeshException);
	}

	GameObject parent = GameObjects.Create();

	parent->Meshes = meshes;
	parent->Count = model->Count;

	// set all the children's meshes parent to the gameobjects transform
	for (size_t i = 0; i < model->Count; i++)
	{
		Transforms.SetParent(meshes[i]->Transform, parent->Transform);
	}

	Models.Dispose(model);

	return parent;
}