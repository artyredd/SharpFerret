#include <engine/physics/Collider.h>

#include "core/types.h"
#include "core/memory.h"
#include <stdlib.h>
#include <string.h>
#include "core/config.h"
#include "core/parsing.h"
#include "engine/modeling/importer.h"
#include "core/quickmask.h"
#include "core/math/triangles.h"
#include "engine/graphics/drawing.h"

#include "cglm/mat4.h"

static Collider Create(Model);
static void Dispose(Collider);
static bool Intersects(Collider, Collider);
static Collider Load(const char* path);

struct _colliderMethods Colliders = {
	.DefaultMask = FLAG_ALL,
	.DefaultLayer = FLAG_0,
	.Create = &Create,
	.Dispose = &Dispose,
	.Intersects = &Intersects,
	.Load = &Load
};

DEFINE_TYPE_ID(Collider);

static Collider Create(Model model)
{
	REGISTER_TYPE(Collider);

	Collider collider = Memory.Alloc(sizeof(struct _collider), ColliderTypeId);

	collider->Layer = Colliders.DefaultLayer;
	collider->Mask = Colliders.DefaultMask;
	collider->Model = model;

	if (model is null)
	{
		throw(InvalidArgumentException);
	}

	// generate the tree
	collider->VoxelTree = Voxels.Create(model->Meshes[0]);

	return collider;
}

private bool GuardCollider(const Collider collider)
{
	if (collider->Model is null)
	{
		return false;
	}

	if (collider->Transform is null)
	{
		return false;
	}

	// assign the transform if we need to
	collider->VoxelTree.Transform = collider->Transform;

	return true;
}

static bool TryGetIntersects(const Collider left, const Collider right, collision* out_hit)
{
	out_hit->LeftHitIndex = 0;
	out_hit->RightHitIndex = 0;

	if (GuardCollider(left) is false || GuardCollider(right) is false)
	{
		return false;
	}

	// generate voxel trees if we need to
	return Voxels.IntersectsTree(left->VoxelTree, right->VoxelTree);
}

static bool Intersects(const Collider left, const Collider right)
{
	// if neither can interact the they cant intersect
	if ((right->Mask & left->Layer) is false && (left->Mask & right->Layer) is false)
	{
		return false;
	}

	collision collision;

	return TryGetIntersects(left, right, &collision);
}

static void Dispose(Collider collider)
{
	Memory.Free(collider->ModelPath, Memory.String);

	Models.Dispose(collider->Model);

	Memory.Free(collider, ColliderTypeId);
}

#define MAX_PATH_LENGTH 512

struct _colliderState {
	char* ModelPath;
	bool IsTrigger;
};


TOKEN_LOAD(model, struct _colliderState*)
{
	return Parsing.TryGetString(buffer, length, MAX_PATH_LENGTH, &state->ModelPath);
}

TOKEN_SAVE(model, Collider)
{
	fprintf(stream, "%s", state->ModelPath);
}

TOKEN_LOAD(trigger, struct _colliderState*)
{
	return Parsing.TryGetBool(buffer, length, &state->IsTrigger);
}

TOKEN_SAVE(trigger, Collider)
{
	fprintf(stream, "%s", state->IsTrigger ? "true" : "false");
}

TOKENS(2) {
	TOKEN(model, "# the model path that should be loaded for the collider"),
		TOKEN(trigger, "# whether or not the collider should have physics interactions at runtime")
};

CONFIG(Collider);

static Collider Load(const char* path)
{
	Collider result = null;

	struct _colliderState state =
	{
		.ModelPath = null,
		.IsTrigger = false
	};

	if (Configs.TryLoadConfig(path, &ColliderConfigDefinition, &state))
	{
		Model model;
		if (Importers.TryImport(state.ModelPath, FileFormats.Obj, &model) == false)
		{
			return result;
		}

		result = Colliders.Create(model);

		result->ModelPath = state.ModelPath;
		result->IsTrigger = state.IsTrigger;

		result->Model = model;
	}

	return result;
}