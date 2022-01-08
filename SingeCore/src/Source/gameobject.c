#include "singine/gameobject.h"
#include "singine/memory.h"
#include "helpers/macros.h"
#include "string.h"
#include "singine/guards.h"

Material DefaultMaterial = null;

static GameObject Duplicate(GameObject);
static void SetName(GameObject, char* name);
static void Dispose(GameObject);
static void Draw(GameObject, Camera);
static GameObject CreateGameObject(void);
static void SetMaterial(GameObject, Material);
static void DrawMany(GameObject* array, size_t count, Camera camera);
static void DestroyMany(GameObject* array, size_t count);
static Material GetDefaultMaterial(void);
static void SetDefaultMaterial(Material);
static GameObject CreateWithMaterial(Material);

const struct _gameObjectMethods GameObjects = {
	.Create = &CreateGameObject,
	.CreateWithMaterial = &CreateWithMaterial,
	.Draw = &Draw,
	.Duplicate = &Duplicate,
	.SetName = &SetName,
	.Destroy = &Dispose,
	.SetMaterial = &SetMaterial,
	.DrawMany = &DrawMany,
	.DestroyMany = &DestroyMany,
	.SetDefaultMaterial = &SetDefaultMaterial,
	.GetDefaultMaterial = &GetDefaultMaterial
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
	return CreateWithMaterial(DefaultMaterial);
}


static GameObject CreateWithMaterial(Material material)
{
	GameObject gameObject = SafeAlloc(sizeof(struct _gameObject));

	gameObject->Id = 0;
	gameObject->Name = null;
	gameObject->Meshes = null;
	gameObject->Count = 0;
	gameObject->NameLength = 0;

	gameObject->Transform = Transforms.Create();
	gameObject->Material = Materials.Instance(material);

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

	SetMaterial(destination, source->Material);

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
	GuardNotNull(gameobject);

	if (gameobject->Name isnt null)
	{
		SafeFree(gameobject->Name);
	}

	if (name is null)
	{
		gameobject->Name = null;
		return;
	}

	size_t length = min(strlen(name), MAX_GAMEOBJECT_NAME_LENGTH);

	char* newName = SafeAlloc(length);

	strncpy_s(newName, length, name, length);

	gameobject->Name = newName;
}

static void SetMaterial(GameObject gameobject, Material material)
{
	GuardNotNull(gameobject);

	// if we dont dispose of the old one we end up with memory leaks becuase the underlying shader AND texture cannot be disposed
	// until all references to them are disposed themselfs
	Materials.Dispose(gameobject->Material);

	// never use the actual material reference given make copy so that if the original is disposed we can still use it
	gameobject->Material = Materials.Instance(material);
}

static void Draw(GameObject gameobject, Camera camera)
{
	// since it's more than  likely the gameobject itself is the parent to all of the transforms
	// it controls we should refresh it's transform first
	Transforms.Refresh(gameobject->Transform);

	for (size_t i = 0; i < gameobject->Count; i++)
	{
		RenderMesh mesh = gameobject->Meshes[i];

		if (mesh is null)
		{
			continue;
		}

		Materials.Draw(gameobject->Material, mesh, camera);
	}
}

static void DrawMany(GameObject* array, size_t count, Camera camera)
{
	for (size_t i = 0; i < count; i++)
	{
		Draw(array[i], camera);
	}
}

static void DestroyMany(GameObject* array, size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		Dispose(array[i]);
	}
}


static Material GetDefaultMaterial(void)
{
	return Materials.Instance(DefaultMaterial);
}

static void SetDefaultMaterial(Material material)
{
	DefaultMaterial = material;
}