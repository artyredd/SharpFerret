#include "engine/gameobject.h"
#include "core/memory.h"
#include "core/macros.h"
#include "string.h"
#include "core/guards.h"
#include "core/strings.h"
#include "core/config.h"
#include "core/math/ints.h"
#include "core/parsing.h"
#include "engine/modeling/importer.h"
#include "cglm/quat.h"
#include "engine/defaults.h"

Material DefaultMaterial = null;

static GameObject Duplicate(GameObject);
static void SetName(GameObject, char* name);
static void Dispose(GameObject);
static void Draw(GameObject, Scene);
static GameObject CreateGameObject(void);
static void SetMaterial(GameObject, Material);
static void DrawMany(GameObject* array, ulong count, Scene camera, Material override);
static void DestroyMany(GameObject* array, ulong count);
static Material GetDefaultMaterial(void);
static void SetDefaultMaterial(Material);
static GameObject CreateWithMaterial(Material);
static GameObject CreateEmpty(ulong size);
static void Clear(GameObject);
static void Resize(GameObject, ulong count);
static GameObject Load(const string path);
static bool Save(GameObject, const string path);
static void GenerateShadowMaps(GameObject* array, ulong count, Scene scene, Material shadowMaterial, Camera shadowCamera);


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
	.GetDefaultMaterial = &GetDefaultMaterial,
	.Clear = &Clear,
	.Resize = &Resize,
	.CreateEmpty = &CreateEmpty,
	.Load = &Load,
	.Save = &Save,
	.GenerateShadowMaps = GenerateShadowMaps
};

static void DisposeRenderMeshArray(GameObject gameobject)
{
	if (gameobject->Meshes isnt null)
	{
		for (ulong i = 0; i < gameobject->Count; i++)
		{
			RenderMesh mesh = gameobject->Meshes[i];

			RenderMeshes.Dispose(mesh);
		}
	}
}

DEFINE_TYPE_ID(GameObject);
DEFINE_TYPE_ID(GameObjectMeshes);

static void Dispose(GameObject gameobject)
{
	Transforms.Dispose(gameobject->Transform);

	DisposeRenderMeshArray(gameobject);

	Memory.Free(gameobject->Meshes, GameObjectMeshesTypeId);

	Memory.Free(gameobject->Name, Memory.String);

	Materials.Dispose(gameobject->Material);

	Memory.Free(gameobject, GameObjectTypeId);
}

static GameObject CreateGameObject()
{
	return CreateWithMaterial(DefaultMaterial);
}

static GameObject CreateWithMaterial(Material material)
{
	Memory.RegisterTypeName(nameof(GameObject), &GameObjectTypeId);

	GameObject gameObject = Memory.Alloc(sizeof(struct _gameObject), GameObjectTypeId);

	gameObject->Transform = Transforms.Create();
	gameObject->Material = Materials.Instance(material);

	return gameObject;
}

static GameObject CreateEmpty(ulong count)
{
	Memory.RegisterTypeName(nameof(GameObject), &GameObjectTypeId);
	Memory.RegisterTypeName(nameof(GameObjectMeshes), &GameObjectMeshesTypeId);

	GameObject gameObject = Memory.Alloc(sizeof(struct _gameObject), GameObjectTypeId);

	gameObject->Transform = null;
	gameObject->Material = null;

	if (count isnt 0)
	{
		gameObject->Count = count;
		gameObject->Meshes = Memory.Alloc(sizeof(RenderMesh) * count, GameObjectMeshesTypeId);

		// set all values to null
		memset(gameObject->Meshes, 0, sizeof(RenderMesh) * count);
	}

	return gameObject;
}

static void GameObjectCopyTo(GameObject source, GameObject destination)
{
	if (source->Name isnt null)
	{
		destination->Name = Strings.DuplicateTerminated(source->Name);
	}

	CopyMember(source, destination, Count);

	if (source->Meshes isnt null)
	{
		destination->Meshes = Memory.Alloc(sizeof(RenderMesh) * source->Count, GameObjectMeshesTypeId);

		for (ulong i = 0; i < source->Count; i++)
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
		Memory.Free(gameobject->Name, Memory.String);
	}

	if (name is null)
	{
		gameobject->Name = null;
		return;
	}

	ulong length = min(strlen(name), MAX_GAMEOBJECT_NAME_LENGTH);

	char* newName = Memory.Alloc(length, Memory.String);

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

static void Draw(GameObject gameobject, Scene scene)
{
	// since it's more than  likely the gameobject itself is the parent to all of the transforms
	// it controls we should refresh it's transform first
	Transforms.Refresh(gameobject->Transform);

	for (ulong i = 0; i < gameobject->Count; i++)
	{
		RenderMesh mesh = gameobject->Meshes[i];

		if (mesh is null)
		{
			continue;
		}

		Materials.Draw(gameobject->Material, mesh, scene);
	}
}

static void DrawWithMaterial(GameObject gameobject, Scene scene, Material material)
{
	// since it's more than  likely the gameobject itself is the parent to all of the transforms
	// it controls we should refresh it's transform first
	Transforms.Refresh(gameobject->Transform);

	for (ulong i = 0; i < gameobject->Count; i++)
	{
		RenderMesh mesh = gameobject->Meshes[i];

		if (mesh is null)
		{
			continue;
		}

		Materials.Draw(material, mesh, scene);
	}
}

static void DrawMany(GameObject* array, ulong count, Scene scene, Material override)
{
	if (override isnt null)
	{
		for (ulong i = 0; i < count; i++)
		{
			DrawWithMaterial(array[i], scene, override);
		}
	}
	else
	{
		for (ulong i = 0; i < count; i++)
		{
			Draw(array[i], scene);
		}
	}
}

static void DestroyMany(GameObject* array, ulong count)
{
	for (ulong i = 0; i < count; i++)
	{
		Dispose(array[i]);
	}
}


static Material GetDefaultMaterial(void)
{
	if (DefaultMaterial is null)
	{
		fprintf(stderr, "Warning: Default material was retrieved, but no default material has been set with GameObjects.SetDefaultMaterial()"NEWLINE);
	}

	return Materials.Instance(DefaultMaterial);
}

static void SetDefaultMaterial(Material material)
{
	DefaultMaterial = material;
}

static void Clear(GameObject gameobject)
{
	for (ulong i = 0; i < gameobject->Count; i++)
	{
		RenderMesh mesh = gameobject->Meshes[i];

		if (mesh isnt null)
		{
			Transforms.SetParent(mesh->Transform, null);
			RenderMeshes.Dispose(mesh);
		}

		gameobject->Meshes[i] = null;
	}
}

static void Resize(GameObject gameobject, ulong count)
{
	DisposeRenderMeshArray(gameobject);

	if (gameobject->Count isnt count)
	{
		Memory.Free(gameobject->Meshes, GameObjectMeshesTypeId);

		gameobject->Count = count;

		gameobject->Meshes = Memory.Alloc(sizeof(RenderMesh) * count, GameObjectMeshesTypeId);
	}

	for (ulong i = 0; i < count; i++)
	{
		gameobject->Meshes[i] = null;
	}
}

static void GenerateShadowMaps(GameObject* array, ulong count, Scene scene, Material shadowMaterial, Camera shadowCamera)
{
	// if there is no lighting return
	if (scene->LightCount is 0)
	{
		return;
	}

	Camera previousCam = scene->MainCamera;
	scene->MainCamera = shadowCamera;

	// make sure the FoV for the camera is 90deg (should fill the face of a cube)
	Cameras.SetFoV(shadowCamera, 120.0f);

	// set the aspect ratio to fit the shadow maps
	Cameras.SetAspectRatio(shadowCamera, (float)ShadowMaps.ResolutionX / (float)ShadowMaps.ResolutionY);

	Cameras.SetFarClippingDistance(shadowCamera, 500.0f);

	for (ulong currentLight = 0; currentLight < scene->LightCount; currentLight++)
	{
		Light light = scene->Lights[currentLight];

		if (light->Enabled is false)
		{
			continue;
		}

		// first set up the "camera" that will be the light
		// set the tranform
		Transforms.Refresh(light->Transform);

		Transforms.SetPosition(shadowCamera->Transform, light->Transform->Position);
		Transforms.SetRotation(shadowCamera->Transform, light->Transform->Rotation);

		Cameras.Refresh(shadowCamera);

		shadowCamera->Orthographic = light->Orthographic;

		Cameras.SetLeftDistance(shadowCamera, -light->Radius);
		Cameras.SetRightDistance(shadowCamera, light->Radius);
		Cameras.SetBottomDistance(shadowCamera, -light->Radius);
		Cameras.SetTopDistance(shadowCamera, light->Radius);

		// enable the frame buffer for the light
		// this will enable the 2d or cubemap framebuffer that was created for the light
		FrameBuffers.ClearAndUse(light->FrameBuffer);

		// draw the objects into the framebuffer

		// since there is lighting iterate through the scenes lights
		// for each light generate it's shadow map by rendering the scene with the provided material
		for (ulong i = 0; i < count; i++)
		{
			DrawWithMaterial(array[i], scene, shadowMaterial);
		}

		// now that the camera's transform should have been updated set it's property
		light->ViewMatrix = shadowCamera->State.State;
	}

	scene->MainCamera = previousCam;
}

#define MaxPathLength 512

#define StreamAbortToken "transform"

struct _gameObjectState
{
	ulong Id;
	char* MaterialPath;
	char* ModelPath;
};

TOKEN_LOAD(id, struct _gameObjectState*)
{
	return Ints.TryDeserialize(buffer, length, &state->Id);
}

TOKEN_LOAD(material, struct _gameObjectState*)
{
	return Parsing.TryGetString(buffer, length, MaxPathLength, &state->MaterialPath);
}

TOKEN_LOAD(model, struct _gameObjectState*)
{
	return Parsing.TryGetString(buffer, length, MaxPathLength, &state->ModelPath);
}

TOKEN_SAVE(id, GameObject)
{
	Ints.Serialize(stream, state->Id);
}

TOKEN_SAVE(model, GameObject)
{
	if (state->Meshes isnt null)
	{
		RenderMeshes.Save(stream, state->Meshes[0]);
	}
}

TOKEN_SAVE(material, GameObject)
{
	if (state->Material)
	{
		fprintf(stream, "%s", state->Material->Name->Values);
	}
}

TOKENS(3) {
	TOKEN(id, "# the id of the gameobject at runtime"),
		TOKEN(model, "# the model path that should be loaded for this gameobject"),
		TOKEN(material, "# the material definition path of this gameobject")
};

struct _configDefinition GameObjectConfigDefinition = {
	.Tokens = Tokens,
	.CommentCharacter = '#',
	.Count = sizeof(Tokens) / sizeof(struct _configToken),
	.AbortToken = ABORT_TOKEN(transform)
};

static GameObject Load(const string path)
{
	GameObject gameObject = null;

	struct _gameObjectState state = {
		.Id = 0,
		.MaterialPath = null,
		.ModelPath = null,
	};

	// manually open the file and use the stream overload since we have to deserialize the transform mid-stream
	File stream;
	if (Files.TryOpen(path, FileModes.ReadBinary, &stream) is false)
	{
		fprintf(stderr, "Failed to load gameobject from file: %s"NEWLINE, path->Values);
		return null;
	}

	if (Configs.TryLoadConfigStream(stream, &GameObjectConfigDefinition, &state))
	{
		RenderMesh* meshArray = null;
		ulong count = 0;

		if (state.ModelPath isnt null)
		{
			ulong modelPathLength = strlen(state.ModelPath ? state.ModelPath : "");
			string modelPath = empty_stack_array(byte, _MAX_PATH);
			strings.AppendCArray(modelPath, state.ModelPath, modelPathLength);

			// load the model first so we can determine how many render meshes we will need
			Model model;
			if (Importers.TryImport(modelPath, FileFormats.Obj, &model) is false)
			{
				fprintf(stderr, "Failed to import the model at path: %s"NEWLINE, state.ModelPath);
				throw(FailedToImportModelException);
			}

			count = model->Count;

			// convert the model into a mesh array so we can use it on the gameobject

			if (RenderMeshes.TryBindModel(model, &meshArray) is false)
			{
				Models.Dispose(model);
				throw(FailedToBindMeshException);
			}

			// get rid of the model now that we have converted it
			Models.Dispose(model);
		}

		// load the material
		ulong materialPathLength = strlen(state.MaterialPath ? state.MaterialPath : "");
		string materialPath = empty_stack_array(byte, _MAX_PATH);
		strings.AppendCArray(materialPath, state.MaterialPath, materialPathLength);
		Material material = Materials.Load(materialPath);

		// if no material was listed use the default one
		if (material is null)
		{
			material = GameObjects.GetDefaultMaterial();
		}

		// deserialize the transform
		Transform transform = Transforms.Load(stream);

		// if we wer're able to get a transform from the file create a new one
		if (transform is null)
		{
			transform = Transforms.Create();
		}

		// set all render meshes as the children to the new transform
		// becuase the transform has no child array when it's created and attaching a child resizes the child array
		// we should manually set the child array
		Transforms.SetChildCapacity(transform, count);

		// iterate the new meshs and set them as children
		for (ulong i = 0; i < count; i++)
		{
			Transforms.SetParent(meshArray[i]->Transform, transform);
		}

		// compose the gameobject
		gameObject = GameObjects.CreateEmpty(0);

		gameObject->Transform = transform;
		gameObject->Material = material;
		gameObject->Meshes = meshArray;
		gameObject->Count = count;
		gameObject->Id = state.Id;
	}

	// close the stream we opened to read the object definition
	if (Files.TryClose(stream) is false)
	{
		throw(FailedToCloseFileException);
	}

	if (gameObject is null)
	{
		fprintf(stderr, "Failed to deserialize a transform from a provided stream");
	}

	// clear the strings we alloced to load the gameobject
	Memory.Free(state.MaterialPath, Memory.String);
	Memory.Free(state.ModelPath, Memory.String);

	return gameObject;
}

static void SaveStream(File stream, GameObject gameobject, const string path)
{
	ignore_unused(path);

	// make sure to put the abort token before the transform so we dont have to rewind the stream to deserialize the transform
	fprintf(stream, "%s:\n", StreamAbortToken);
	Transforms.Save(gameobject->Transform, stream);
}

static bool Save(GameObject gameobject, const string path)
{
	// manually open the file and use the stream overload since we have to serialize the transform mid-stream
	File stream;
	if (Files.TryOpen(path, FileModes.Create, &stream) is false)
	{
		return false;
	}

	Configs.SaveConfigStream(stream, &GameObjectConfigDefinition, gameobject);

	SaveStream(stream, gameobject, path);

	return Files.TryClose(stream);
}