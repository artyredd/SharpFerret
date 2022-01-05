#include "singine/gameobject.h"
#include "singine/memory.h"
#include "helpers/macros.h"
#include "string.h"

static GameObject Duplicate(GameObject);
static void SetName(GameObject, char* name);
static void Dispose(GameObject);
static void Draw(GameObject, Camera);

const struct _gameObjectMethods GameObjects = {
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
			gameobject->Meshes[i]->Dispose(gameobject->Meshes[i]);
		}

		SafeFree(gameobject->Meshes);
	}

	gameobject->Transform->Dispose(gameobject->Transform);

	if (gameobject->Name isnt null)
	{
		SafeFree(gameobject->Name);
	}

	Materials.Dispose(gameobject->Material);

	SafeFree(gameobject);
}

GameObject CreateGameObject()
{
	GameObject gameObject = SafeAlloc(sizeof(struct _gameObject));

	gameObject->Id = 0;
	gameObject->Name = null;
	gameObject->Meshes = null;
	gameObject->Count = 0;
	gameObject->NameLength = 0;

	gameObject->Transform = CreateTransform();
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
			destination->Meshes[i] = InstanceMesh(source->Meshes[i]);

			SetParent(destination->Meshes[i]->Transform, destination->Transform);

			TransformCopyTo(source->Meshes[i]->Transform, destination->Meshes[i]->Transform);
		}
	}

	destination->Material = Materials.Instance(source->Material);

	TransformCopyTo(source->Transform, destination->Transform);
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
	RefreshTransform(gameobject->Transform);

	for (size_t i = 0; i < gameobject->Count; i++)
	{
		camera->DrawMesh(camera, gameobject->Meshes[i], gameobject->Material);
	}
}