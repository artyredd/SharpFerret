#include "graphics/transform.h"
#include "singine/memory.h"

#include "cglm/affine.h"
#include "cglm/quat.h"
#include "helpers/quickmask.h"


#define PositionModifiedFlag FLAG_0
#define RotationModifiedFlag FLAG_1
#define ScaleModifiedFlag FLAG_2
#define ParentModifiedFlag FLAG_3

#define AllModifiedFlag (PositionModifiedFlag | RotationModifiedFlag | ScaleModifiedFlag)

Transform CreateTransform()
{
	Transform transform = SafeAlloc(sizeof(struct _transform));

	transform->Child = null;
	transform->LastChild = null;
	transform->Parent = null;
	transform->Next = null;
	transform->Previous = null;
	transform->Count = 0;

	InitializeVector3(transform->Position);
	SetVector3(transform->Scale, 1, 1, 1);
	SetVector4(transform->Rotation, 0, 0, 0, 1);

	InitializeMat4(transform->PreviousState.State);

	// set the all modified flag so we do a full refresh of the transform on first draw
	transform->PreviousState.Modified = AllModifiedFlag;

	// no need to init the transform states since they will be populated before first draw by RecalculateTransform

	return transform;
}

/// <summary>
/// notifies all children of this transform that the transform was modified
/// </summary>
/// <param name="transform"></param>
static void NotifyChildren(Transform transform)
{
	Transform child = transform->Child;
	while (child != null)
	{
		SetFlag(child->PreviousState.Modified, ParentModifiedFlag, true);
		child = child->Next;
	}
}

vec4* RefreshTransform(Transform transform)
{
	unsigned int mask = transform->PreviousState.Modified;

	// if nothing changed return
	if (mask is 0)
	{
		return transform->PreviousState.State;
	}
	
	// check to see if only my parent has changed
	if (mask is ParentModifiedFlag && transform->Parent != null)
	{
		glm_mat4_mul(transform->Parent->PreviousState.State, transform->PreviousState.LocalState, transform->PreviousState.State);

		ResetFlags(transform->PreviousState.Modified);

		NotifyChildren(transform);

		return transform->PreviousState.State;
	}

	// check to see if we should perform a whole refresh
	// since the transform starts with scale a change to it requires a whole refresh by default
	if (HasFlag(mask, AllModifiedFlag) || HasFlag(mask, ScaleModifiedFlag))
	{
		return ForceRefreshTransform(transform);
	}

	// since we dont need to do a full refresh, do only the calcs needed to save cpu time
	// this engine was orignally designed to be single threaded
	if (HasFlag(mask, RotationModifiedFlag))
	{
		glm_quat_rotate(transform->PreviousState.ScaleMatrix, transform->Rotation, transform->PreviousState.RotationMatrix);
	}

	// since the rotation and scale were not affected only translate and store into the previous state
	glm_translate_to(transform->PreviousState.RotationMatrix, transform->Position, transform->PreviousState.LocalState);

	if(transform->Parent != null)
	{ 
		glm_mat4_mul(transform->Parent->PreviousState.State, transform->PreviousState.LocalState, transform->PreviousState.State);
	}
	else
	{
		SetMatrices4(transform->PreviousState.State, transform->PreviousState.LocalState);
	}

	ResetFlags(transform->PreviousState.Modified);

	NotifyChildren(transform);

	return transform->PreviousState.State;
}

vec4* ForceRefreshTransform(Transform transform)
{
	glm_scale_to(Matrix4.Identity, transform->Scale, transform->PreviousState.ScaleMatrix);

	glm_quat_rotate(transform->PreviousState.ScaleMatrix, transform->Rotation, transform->PreviousState.RotationMatrix);

	glm_translate_to(transform->PreviousState.RotationMatrix, transform->Position, transform->PreviousState.LocalState);

	if (transform->Parent != null)
	{
		glm_mat4_mul(transform->Parent->PreviousState.State, transform->PreviousState.LocalState, transform->PreviousState.State);
	}
	else
	{
		SetMatrices4(transform->PreviousState.State, transform->PreviousState.LocalState);
	}

	// make sure to reset the dirty flag
	ResetFlags(transform->PreviousState.Modified);

	// notify the children that they should recalculate their transforms to use our
	// new state
	NotifyChildren(transform);

	return transform->PreviousState.State;
}

void SetPosition(Transform transform, vec3 position)
{
	if (Vector3Equals(position, transform->Position))
	{
		return;
	}

	SetVectors3(transform->Position, position);
	SetFlag(transform->PreviousState.Modified, PositionModifiedFlag, true);
}

void SetRotation(Transform transform, Quaternion rotation)
{
	if (Vector4Equals(rotation, transform->Rotation))
	{
		return;
	}

	SetVectors4(transform->Rotation, rotation);
	SetFlag(transform->PreviousState.Modified, RotationModifiedFlag, true);
}

void SetScale(Transform transform, vec3 scale)
{
	if (Vector3Equals(scale, transform->Scale))
	{
		return;
	}

	SetVectors3(transform->Scale, scale);
	SetFlag(transform->PreviousState.Modified, ScaleModifiedFlag, true);
}

void AddPostion(Transform transform, vec3 amount)
{
	AddVectors3(transform->Position, amount);
	SetFlag(transform->PreviousState.Modified, PositionModifiedFlag, true);
}

void AddRotation(Transform transform, Quaternion amount)
{
	// to add a rotation to a quaterion we multiply, gotta love imaginary number magic
	glm_quat_mul(transform->Rotation, amount, transform->Rotation);

	SetFlag(transform->PreviousState.Modified, RotationModifiedFlag, true);
}

void AddScale(Transform transform, vec3 amount)
{
	AddVectors3(transform->Scale, amount);
	SetFlag(transform->PreviousState.Modified, ScaleModifiedFlag, true);
}