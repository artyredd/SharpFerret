#include "graphics/transform.h"
#include "singine/memory.h"

#include "cglm/affine.h"
#include "cglm/quat.h"
#include "helpers/quickmask.h"
#include "singine/guards.h"

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

	InitializeMat4(transform->State.State);

	// set the all modified flag so we do a full refresh of the transform on first draw
	transform->State.Modified = AllModifiedFlag;

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
		SetFlag(child->State.Modified, ParentModifiedFlag);
		child = child->Next;
	}
}

vec4* RefreshTransform(Transform transform)
{
	unsigned int mask = transform->State.Modified;

	// if nothing changed return
	if (mask is 0)
	{
		return transform->State.State;
	}
	
	// check to see if only my parent has changed
	if (mask is ParentModifiedFlag && transform->Parent != null)
	{
		glm_mat4_mul(transform->Parent->State.State, transform->State.LocalState, transform->State.State);

		ResetFlags(transform->State.Modified);

		NotifyChildren(transform);

		return transform->State.State;
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
		glm_quat_rotate(transform->State.ScaleMatrix, transform->Rotation, transform->State.RotationMatrix);
	}

	// since the rotation and scale were not affected only translate and store into the previous state
	glm_translate_to(transform->State.RotationMatrix, transform->Position, transform->State.LocalState);

	if(transform->Parent != null)
	{ 
		glm_mat4_mul(transform->Parent->State.State, transform->State.LocalState, transform->State.State);
	}
	else
	{
		SetMatrices4(transform->State.State, transform->State.LocalState);
	}

	ResetFlags(transform->State.Modified);

	NotifyChildren(transform);

	return transform->State.State;
}

vec4* ForceRefreshTransform(Transform transform)
{
	glm_scale_to(Matrix4.Identity, transform->Scale, transform->State.ScaleMatrix);

	glm_quat_rotate(transform->State.ScaleMatrix, transform->Rotation, transform->State.RotationMatrix);

	glm_translate_to(transform->State.RotationMatrix, transform->Position, transform->State.LocalState);

	if (transform->Parent != null)
	{
		glm_mat4_mul(transform->Parent->State.State, transform->State.LocalState, transform->State.State);
	}
	else
	{
		SetMatrices4(transform->State.State, transform->State.LocalState);
	}

	// make sure to reset the dirty flag
	ResetFlags(transform->State.Modified);

	// notify the children that they should recalculate their transforms to use our
	// new state
	NotifyChildren(transform);

	return transform->State.State;
}

// Detaches the provided child from the given transform
// peforms an O(n) search by reference
static void DetachChild(Transform transform, Transform child)
{
	GuardNotNull(transform);

	// while it doesnt make sense to call this with a null child
	// the expected result of removing nothing, is nothing.. so just return
	if (child is null)
	{
		return;
	}

	// traverse all children and remove in-place

	Transform current = transform->Child;

	while (current != null)
	{
		// perform reference check
		if (current is child)
		{
			// since this is the child remove it,
			// transforms are a doubly linked list

			// store the next and previous so we can link them
			Transform next = current->Next;
			Transform previous = current->Previous;

			// reset the child's links
			current->Parent = null;
			current->Next = null;
			current->Previous = null;

			// make sure if the child is the head or tail we drop the references
			if(current is transform->Child)
			{ 
				// set the new head to either next or previous with preference to next
				transform->Child = NullCoalesce(next, previous);
			}
			if (current is transform->LastChild)
			{
				// set the new tail to either next or previous with preference to previous
				transform->LastChild = NullCoalesce(previous, next);;
			}

			// link next and previous if they exist
			if (next isnt null)
			{
				next->Previous = previous;
			}
			if (previous isnt null)
			{
				previous->Next = next;
			}

			// reduce the number of children
			--(transform->Count);

			break;
		}
	}
}

static void AttachChild(Transform transform, Transform child)
{
	if (transform isnt null)
	{
		++(transform->Count);

		if (transform->LastChild is null)
		{
			transform->Child = transform->LastChild = child;
		}
		else
		{
			transform->LastChild->Next = child;
			child->Previous = transform->LastChild;
		}
	}

	child->Parent = transform;
}

void SetParent(Transform transform, Transform parent)
{
	// make sure to detach the previous parent if there is one
	if (transform->Parent isnt null)
	{
		DetachChild(transform->Parent, transform);
	}

	// attach the child to the new parent
	AttachChild(parent, transform);

	// mark the child transform as modified since there is a new parent that
	// will affect it's transform
   	SetFlag(transform->State.Modified, ParentModifiedFlag);
}

void SetPosition(Transform transform, vec3 position)
{
	if (Vector3Equals(position, transform->Position))
	{
		return;
	}

	SetVectors3(transform->Position, position);
	SetFlag(transform->State.Modified, PositionModifiedFlag);
}

void SetRotation(Transform transform, Quaternion rotation)
{
	if (Vector4Equals(rotation, transform->Rotation))
	{
		return;
	}

	SetVectors4(transform->Rotation, rotation);
	SetFlag(transform->State.Modified, RotationModifiedFlag);
}

void SetScale(Transform transform, vec3 scale)
{
	if (Vector3Equals(scale, transform->Scale))
	{
		return;
	}

	SetVectors3(transform->Scale, scale);
	SetFlag(transform->State.Modified, ScaleModifiedFlag);
}

void AddPosition(Transform transform, vec3 amount)
{
	if (Vector3Equals(amount, Vector3.Zero))
	{
		return;
	}

	AddVectors3(transform->Position, amount);
	SetFlag(transform->State.Modified, PositionModifiedFlag);
}

void AddRotation(Transform transform, Quaternion amount)
{
	// to add a rotation to a quaterion we multiply, gotta love imaginary number magic
	glm_quat_mul(transform->Rotation, amount, transform->Rotation);

	SetFlag(transform->State.Modified, RotationModifiedFlag);
}

void AddScale(Transform transform, vec3 amount)
{
	AddVectors3(transform->Scale, amount);
	SetFlag(transform->State.Modified, ScaleModifiedFlag);
}