#include "singine/gameobject.h"
#include "singine/memory.h"
#include "helpers/macros.h"
#include "string.h"

static GameObject Duplicate(GameObject);
static void SetName(GameObject, char* name);
static void Dispose(GameObject);
static void Draw(GameObject, Camera);
static GameObject CreateGameObject(void);

const struct _gameObjectMethods GameObjects = {
	.Create = &CreateGameObject,
	.Draw = &Draw,
	.Duplicate = &Duplicate,
	.SetName = &SetName,
	.Destroy = &Dispose
};

static void Dispose(GameObject gameobject)
{
	if (gameobject->Meshes isnt null)
	{
		for (size_t i = 0; i < gameobject->Count; i++)
		{
			RenderMeshes.Dispose(gameobject->Meshes[i]);
		}

		SafeFree(gameobject->Meshes);
	}

	Transforms.Dispose(gameobject->Transform);

	if (gameobject->Name isnt null)
	{
		SafeFree(gameobject->Name);
	}

	Materials.Dispose(gameobject->Material);

	SafeFree(gameobject);
}

static GameObject CreateGameObject()
{
	GameObject gameObject = SafeAlloc(sizeof(struct _gameObject));

	gameObject->Id = 0;
	gameObject->Name = null;
	gameObject->Meshes = null;
	gameObject->Count = 0;
	gameObject->NameLength = 0;

	gameObject->Transform = Transforms.Create();
	gameObject->Material = null;

	return gameObject;
}

static void GameObjectCopyTo(GameObject source, GameObject destination)
{
	if (source->Name isnt null)
	{
		destination->Name = SafeAlloc(source->NameLength);
		strcpy_s(source->Name, source->NameLength, destination->Name);
	}

	CopyMember(source, destination, Count);

	if (source->Meshes isnt null)
	{
		destination->Meshes = SafeAlloc(sizeof(RenderMesh) * source->Count);

		for (size_t i = 0; i < source->Count; i++)
		{
			destination->Meshes[i] = RenderMeshes.Instance(source->Meshes[i]);

			Transforms.SetParent(destination->Meshes[i]->Transform, destination->Transform);

			Transforms.CopyTo(source->Meshes[i]->Transform, destination->Meshes[i]->Transform);
		}
	}

	destination->Material = Materials.Instance(source->Material);

	Transforms.CopyTo(source->Transform, destination->Transform);
}

static GameObject Duplicate(GameObject gameobject)
{
	GameObject newGameObject = CreateGameObject();

	GameObjectCopyTo(gameobject, newGameObject);

	return newGameObject;
}

static void SetName(GameObject gameobject, char* name)
{
	throw(NotImplementedException);
}

static void Draw(GameObject gameobject, Camera camera)
{
	// since it's more than  likely the gameobject itself is the parent to all of the transforms
	// it controls we should refresh it's transform first
	Transforms.Refresh(gameobject->Transform);

	for (size_t i = 0; i < gameobject->Count; i++)
	{
		Cameras.DrawMesh(camera, gameobject->Meshes[i], gameobject->Material);
	}
}