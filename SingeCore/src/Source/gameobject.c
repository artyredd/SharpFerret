#include "singine/gameobject.h"
#include "singine/memory.h"
#include "helpers/macros.h"
#include "string.h"
#include "singine/guards.h"
#include "singine/strings.h"
#include "singine/config.h"
#include "math/ints.h"
#include "singine/parsing.h"
#include "modeling/importer.h"
#include "cglm/quat.h"
#include "singine/defaults.h"

Material DefaultMaterial = null;

static GameObject Duplicate(GameObject);
static void SetName(GameObject, char* name);
static void Dispose(GameObject);
static void Draw(GameObject, Scene);
static GameObject CreateGameObject(void);
static void SetMaterial(GameObject, Material);
static void DrawMany(GameObject* array, size_t count, Scene camera, Material override);
static void DestroyMany(GameObject* array, size_t count);
static Material GetDefaultMaterial(void);
static void SetDefaultMaterial(Material);
static GameObject CreateWithMaterial(Material);
static GameObject CreateEmpty(size_t size);
static void Clear(GameObject);
static void Resize(GameObject, size_t count);
static GameObject Load(const char* path);
static bool Save(GameObject, const char* path);
static void GenerateShadowMaps(GameObject* array, size_t count, Scene scene, Material shadowMaterial, Material cubemapShadowMaterial, Camera shadowCamera);


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
		for (size_t i = 0; i < gameobject->Count; i++)
		{
			RenderMesh mesh = gameobject->Meshes[i];

			RenderMeshes.Dispose(mesh);
		}
	}
}

static void Dispose(GameObject gameobject)
{
	Transforms.Dispose(gameobject->Transform);

	DisposeRenderMeshArray(gameobject);

	SafeFree(gameobject->Meshes);

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

	gameObject->Transform = Transforms.Create();
	gameObject->Material = Materials.Instance(material);

	return gameObject;
}

static GameObject CreateEmpty(size_t count)
{
	GameObject gameObject = SafeAlloc(sizeof(struct _gameObject));

	gameObject->Transform = null;
	gameObject->Material = null;

	if (count isnt 0)
	{
		gameObject->Count = count;
		gameObject->Meshes = SafeAlloc(sizeof(RenderMesh) * count);

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

static void Draw(GameObject gameobject, Scene scene)
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

		Materials.Draw(gameobject->Material, mesh, scene);
	}
}

static void DrawWithMaterial(GameObject gameobject, Scene scene, Material material)
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

		Materials.Draw(material, mesh, scene);
	}
}

static void DrawMany(GameObject* array, size_t count, Scene scene, Material override)
{
	if (override isnt null)
	{
		for (size_t i = 0; i < count; i++)
		{
			DrawWithMaterial(array[i], scene, override);
		}
	}
	else
	{
		for (size_t i = 0; i < count; i++)
		{
			Draw(array[i], scene);
		}
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
	if (DefaultMaterial is null)
	{
		fprintf(stderr, "Warning: Default material was retrieved, but no default material has been set with GameObjects.SetDefaultMaterial()");
	}

	return Materials.Instance(DefaultMaterial);
}

static void SetDefaultMaterial(Material material)
{
	DefaultMaterial = material;
}

static void Clear(GameObject gameobject)
{
	for (size_t i = 0; i < gameobject->Count; i++)
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

static void Resize(GameObject gameobject, size_t count)
{
	DisposeRenderMeshArray(gameobject);

	if (gameobject->Count isnt count)
	{
		SafeFree(gameobject->Meshes);

		gameobject->Count = count;

		gameobject->Meshes = SafeAlloc(sizeof(RenderMesh) * count);
	}

	for (size_t i = 0; i < count; i++)
	{
		gameobject->Meshes[i] = null;
	}
}

static void GenerateShadowMaps(GameObject* array, size_t count, Scene scene, Material shadowMaterial, Material cubemapShadowMaterial, Camera shadowCamera)
{
	// if there is no lighting return
	if (scene->LightCount is 0)
	{
		return;
	}

	Camera previousCam = scene->MainCamera;
	scene->MainCamera = shadowCamera;

	// make sure the FoV for the camera is 90deg (should fill the face of a cube)
	Cameras.SetFoV(shadowCamera, 90.0f);

	// set the aspect ratio to fit the shadow maps
	Cameras.SetAspectRatio(shadowCamera, (float)ShadowMaps.ResolutionX / (float)ShadowMaps.ResolutionY);

	for (size_t currentLight = 0; currentLight < scene->LightCount; currentLight++)
	{
		Light light = scene->Lights[currentLight];

		// refresh the transform and light matrices
		Lights.RefreshMatrices(light);

		// first set up the "camera" that will be the light
		// set the tranform
		Transforms.SetPosition(shadowCamera->Transform, light->Transform->Position);
		Transforms.SetRotation(shadowCamera->Transform, light->Transform->Rotation);

		// default to 2d shadow
		Material material = shadowMaterial;

		if (light->Type is LightTypes.Directional)
		{
			shadowCamera->Orthographic = light->Orthographic;
			
			// use the method to flag the camera as needing to be refreshed
			Cameras.SetLeftDistance(shadowCamera, -light->Radius);
			shadowCamera->RightDistance = light->Radius;
			shadowCamera->BottomDistance = -light->Radius;
			shadowCamera->TopDistance = light->Radius;
		}
		else
		{
			shadowCamera->Orthographic = false;

			// if the camera is point or spotlight we need to render to cubemap instead, use the cubemap material
			// this material should ideall use a geometry shader to write to all faces if needed
			material = cubemapShadowMaterial;
		}

		// set the range of the shadow camera
		Cameras.SetFarClippingDistance(shadowCamera, light->Range);

		// enable the frame buffer for the light
		// this will enable the 2d or cubemap framebuffer that was created for the light
		FrameBuffers.ClearAndUse(light->FrameBuffer);

		// draw the objects into the framebuffer

		// since there is lighting iterate through the scenes lights
		// for each light generate it's shadow map by rendering the scene with the provided material
		for (size_t i = 0; i < count; i++)
		{
			DrawWithMaterial(array[i], scene, material);
		}

		// now that the camera's transform should have been updated set it's property
		Matrix4CopyTo(shadowCamera->State.State, light->LightMatrices[0]);
	}

	scene->MainCamera = previousCam;
}

#define MaxPathLength 512

#define CommentFormat "%s\n"
#define TokenFormat "%s: "

#define IdTokenComment "# the id of the gameobject at runtime"
#define IdToken "id"
#define MaterialTokenComment "# the material definition path of this gameobject"
#define MaterialToken "material"
#define ModelTokenComment "# the model path that should be loaded for this gameobject"
#define ModelToken "model"

#define StreamAbortToken "transform"

static const char* Tokens[] = {
	IdToken,
	ModelToken,
	MaterialToken
};

static const size_t TokenLengths[] = {
	sizeof(IdToken),
	sizeof(ModelToken),
	sizeof(MaterialToken),
};

struct _textureInfo {
	size_t Id;
	char* MaterialPath;
	char* ModelPath;
};

static bool OnTokenFound(size_t index, const char* buffer, const size_t length, struct _textureInfo* state);

struct _configDefinition GameObjectConfigDefinition = {
	.Tokens = (const char**)&Tokens,
	.TokenLengths = (const size_t*)&TokenLengths,
	.CommentCharacter = '#',
	.Count = sizeof(Tokens) / sizeof(char*),
	.OnTokenFound = &OnTokenFound,
	.AbortToken = StreamAbortToken,
	.AbortTokenLength = sizeof(StreamAbortToken)
};

static bool OnTokenFound(size_t index, const char* buffer, const size_t length, struct _textureInfo* state)
{
	switch (index)
	{
	case 0: // id
		return Ints.TryDeserialize(buffer, length, &state->Id);
	case 1: // model path
		return TryParseString(buffer, length, MaxPathLength, &state->ModelPath);
	case 2: // material path
		return TryParseString(buffer, length, MaxPathLength, &state->MaterialPath);
	default:
		return false;
	}
}

static GameObject Load(const char* path)
{
	GameObject gameObject = null;

	struct _textureInfo state = {
		.Id = 0,
		.MaterialPath = null,
		.ModelPath = null,
	};

	// manually open the file and use the stream overload since we have to deserialize the transform mid-stream
	File stream;
	if (Files.TryOpen(path, FileModes.ReadBinary, &stream) is false)
	{
		fprintf(stderr, "Failed to load gameobject from file: %s", path);
		return null;
	}

	if (Configs.TryLoadConfigStream(stream, &GameObjectConfigDefinition, &state))
	{
		RenderMesh* meshArray = null;
		size_t count = 0;

		if (state.ModelPath isnt null)
		{
			// load the model first so we can determine how many render meshes we will need
			Model model;
			if (Importers.TryImport(state.ModelPath, FileFormats.Obj, &model) is false)
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
		Material material = Materials.Load(state.MaterialPath);

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
		for (size_t i = 0; i < count; i++)
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
	SafeFree(state.MaterialPath);
	SafeFree(state.ModelPath);

	return gameObject;
}

static bool Save(GameObject gameobject, const char* path)
{
	// manually open the file and use the stream overload since we have to serialize the transform mid-stream
	File stream;
	if (Files.TryOpen(path, FileModes.Create, &stream) is false)
	{
		return false;
	}

	fprintf(stream, CommentFormat, IdTokenComment);
	fprintf(stream, TokenFormat, IdToken);
	fprintf(stream, "%lli\n", gameobject->Id);

	if (gameobject->Meshes isnt null)
	{
		fprintf(stream, CommentFormat, ModelTokenComment);
		fprintf(stream, TokenFormat, ModelToken);
		fprintf(stream, "%s\n", (char*)gameobject->Meshes[0]->Name->Resource);
	}

	fprintf(stream, CommentFormat, MaterialTokenComment);
	fprintf(stream, TokenFormat, MaterialToken);
	fprintf(stream, "%s\n", gameobject->Material->Name);


	// make sure to put the abort token before the transform so we dont have to rewind the stream to deserialize the transform
	fprintf(stream, "%s:\n", StreamAbortToken);
	Transforms.Save(gameobject->Transform, stream);

	return Files.TryClose(stream);
}