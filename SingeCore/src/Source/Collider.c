#include "physics/Collider.h"

#include "types.h"
#include "singine/memory.h"
#include <stdlib.h>
#include <string.h>
#include "singine/config.h"
#include "singine/parsing.h"
#include "modeling/importer.h"
#include "helpers/quickmask.h"
#include "math/triangles.h"
#include "graphics/drawing.h"

#include "cglm/mat4.h"

static Collider Create();
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

TYPE_ID(Collider);

static Collider Create()
{
	REGISTER_TYPE(Collider);

	Collider collider = Memory.Alloc(sizeof(struct _collider), ColliderTypeId);

	collider->Layer = Colliders.DefaultLayer;
	collider->Mask = Colliders.DefaultMask;

	return collider;
}

bool GuardCollider(const Collider collider)
{
	if (collider->Model is null)
	{
		return false;
	}

	if (collider->Transform is null)
	{
		return false;
	}

	return true;
}

Material leftTriangleMaterial;
Material rightTriangleMaterial;

static bool TryGetIntersects(const Collider leftCollider, const Collider rightCollider, collision* out_hit)
{
	if (leftTriangleMaterial is null)
	{
		leftTriangleMaterial = Materials.Load("assets/materials/unlit.material");
		rightTriangleMaterial = Materials.Load("assets/materials/unlit.material");

		Materials.SetColors(leftTriangleMaterial, 1.0, 0.0, 0.0, 1.0);
		Materials.SetColors(rightTriangleMaterial, 0.0, 0.0, 1.0, 1.0);
	}

	out_hit->LeftHitIndex = 0;
	out_hit->RightHitIndex = 0;

	if (GuardCollider(leftCollider) is false || GuardCollider(rightCollider) is false)
	{
		return false;
	}

	// traverse the triangles and see if any intersect
	// for performance reasons we're assuming the first collider in each model is 
	// the mesh for the collider
	const Mesh left = leftCollider->Model->Meshes[0];
	const Mesh right = rightCollider->Model->Meshes[0];

	for (size_t leftIndex = 0; leftIndex < left->VertexCount; leftIndex += 9)
	{
		triangle leftTriangle;

		Transforms.TransformPoint(leftCollider->Transform, &left->VertexData[leftIndex], leftTriangle[0]);
		Transforms.TransformPoint(leftCollider->Transform, &left->VertexData[leftIndex + 3], leftTriangle[1]);
		Transforms.TransformPoint(leftCollider->Transform, &left->VertexData[leftIndex + 6], leftTriangle[2]);

		for (size_t rightIndex = 0; rightIndex < right->VertexCount; rightIndex += 9)
		{
			triangle rightTriangle;

			Transforms.TransformPoint(rightCollider->Transform, &right->VertexData[rightIndex], rightTriangle[0]);
			Transforms.TransformPoint(rightCollider->Transform, &right->VertexData[rightIndex + 3], rightTriangle[1]);
			Transforms.TransformPoint(rightCollider->Transform, &right->VertexData[rightIndex + 6], rightTriangle[2]);

			Triangles.WindTriangle(leftTriangle);
			Triangles.WindTriangle(rightTriangle);

			if (Triangles.Intersects(leftTriangle, rightTriangle))
			{
				out_hit->RightHitIndex = rightIndex;
				out_hit->LeftHitIndex = leftIndex;

				Drawing.DrawTriangle((float*)leftTriangle, leftTriangleMaterial);
				Drawing.DrawTriangle((float*)rightTriangle, rightTriangleMaterial);

				return true;
			}
		}
	}

	return false;
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
		result = Colliders.Create();

		result->ModelPath = state.ModelPath;
		result->IsTrigger = state.IsTrigger;

		Model model;
		if (Importers.TryImport(state.ModelPath, FileFormats.Obj, &model) == false)
		{
			Colliders.Dispose(result);
			result = null;

			return result;
		}

		result->Model = model;
	}

	return result;
}