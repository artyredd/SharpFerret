#include "graphics/transform.h"
#include "singine/memory.h"

#include "helpers/macros.h"

#include "cglm/affine.h"
#include "cglm/quat.h"
#include "helpers/quickmask.h"
#include "singine/guards.h"
#include "math/vectors.h"

#define PositionModifiedFlag FLAG_0
#define RotationModifiedFlag FLAG_1
#define ScaleModifiedFlag FLAG_2
#define ParentModifiedFlag FLAG_3

#define AllModifiedFlag (PositionModifiedFlag | RotationModifiedFlag | ScaleModifiedFlag)

static void Dispose(Transform transform);
static Transform CreateTransform(void);
static void TransformCopyTo(Transform source, Transform destination);
static void SetParent(Transform transform, Transform parent);
static void SetPosition(Transform transform, vec3 position);
static void SetPositions(Transform transform, float x, float y, float z);
static void SetRotation(Transform transform, Quaternion rotation);
static void SetScale(Transform transform, vec3 scale);
static void SetScales(Transform transform, float x, float y, float z);
static void AddPosition(Transform transform, vec3 amount);
static void Translate(Transform, float x, float y, float z);
static void TranslateX(Transform, float x);
static void TranslateY(Transform, float y);
static void TranslateZ(Transform, float z);
static void Rotate(Transform transform, Quaternion amount);
static void RotateOnAxis(Transform, float amountInRads, vec3 axis);
static void SetRotationOnAxis(Transform, float amountInRads, vec3 axis);
static void AddScale(Transform transform, vec3 amount);
static void GetDirection(Transform transform, Direction direction, vec3 out_direction);
static vec4* RefreshTransform(Transform transform);
static vec4* ForceRefreshTransform(Transform transform);

const struct _transformMethods Transforms = {
	.Dispose = &Dispose,
	.Create = &CreateTransform,
	.CopyTo = &TransformCopyTo,
	.SetParent = &SetParent,
	.SetPosition = &SetPosition,
	.SetPositions = &SetPositions,
	.SetRotation = &SetRotation,
	.SetScale = &SetScale,
	.SetScales = &SetScales,
	.AddPosition = &AddPosition,
	.Rotate = &Rotate,
	.RotateOnAxis = &RotateOnAxis,
	.SetRotationOnAxis = &SetRotationOnAxis,
	.AddScale = &AddScale,
	.GetDirection = &GetDirection,
	.Refresh = &RefreshTransform,
	.ForceRefresh = &ForceRefreshTransform,
	.Translate = &Translate,
	.TranslateX = &TranslateX,
	.TranslateY = &TranslateY,
	.TranslateZ = &TranslateZ
};

static void Dispose(Transform transform)
{
	SafeFree(transform);
}

static Transform CreateTransform()
{
	Transform transform = SafeAlloc(sizeof(struct _transform));

	transform->Child = null;
	transform->LastChild = null;
	transform->Parent = null;
	transform->Next = null;
	transform->Previous = null;
	transform->Count = 0;
	transform->RotateAroundCenter = false;
	transform->InvertTransform = false;

	InitializeVector3(transform->Position);
	SetVector3(transform->Scale, 1, 1, 1);
	SetVector4(transform->Rotation, 0, 0, 0, 1);

	InitializeMat4(transform->State.State);

	// set the all modified flag so we do a full refresh of the transform on first draw
	transform->State.Modified = AllModifiedFlag;
	ResetFlags(transform->State.Directions.Accessed);

	// no need to init the transform states since they will be populated before first draw by RecalculateTransform

	return transform;
}

static void DirectionStatesCopyTo(struct _directionStates* source, struct _directionStates* destination)
{
	CopyMember(source, destination, Accessed);

	Vectors3CopyTo(source->Directions[0], destination->Directions[0]);
	Vectors3CopyTo(source->Directions[1], destination->Directions[1]);
	Vectors3CopyTo(source->Directions[2], destination->Directions[2]);
	Vectors3CopyTo(source->Directions[3], destination->Directions[3]);
	Vectors3CopyTo(source->Directions[4], destination->Directions[4]);
	Vectors3CopyTo(source->Directions[5], destination->Directions[5]);
}

static void StateCopyTo(struct transformState* source, struct transformState* destination)
{
	CopyMember(source, destination, Modified);

	Matrix4CopyTo(source->ScaleMatrix, destination->ScaleMatrix);
	Matrix4CopyTo(source->RotationMatrix, destination->RotationMatrix);
	Matrix4CopyTo(source->LocalState, destination->LocalState);
	Matrix4CopyTo(source->State, destination->State);

	DirectionStatesCopyTo(&source->Directions, &destination->Directions);
}

void TransformCopyTo(Transform source, Transform destination)
{
	CopyMember(source, destination, RotateAroundCenter);
	CopyMember(source, destination, InvertTransform);

	Vectors3CopyTo(source->Scale, destination->Scale);
	Vectors3CopyTo(source->Position, destination->Position);
	Vectors4CopyTo(source->Rotation, destination->Rotation);

	StateCopyTo(&source->State, &destination->State);
}

static Transform DuplicateTransform(Transform transform)
{
	Transform newTransform = CreateTransform();

	TransformCopyTo(transform, newTransform);

	return newTransform;
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

static vec4* RefreshTransform(Transform transform)
{
	unsigned int mask = transform->State.Modified;

	// if nothing changed return
	if (mask is 0)
	{
		return transform->State.State;
	}

	// since ANYthing changed force recalculation of direction vectors
	ResetFlags(transform->State.Directions.Accessed);

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
		return Transforms.ForceRefresh(transform);
	}

	// create a place to store rot and pos for the followup matrix multiplications
	Quaternion rotation;
	vec3 position;

	// check whether or not we should invert the pos and rotation
	// this is typically only used for the camera since the camera never "moves"
	if (transform->InvertTransform)
	{
		glm_quat_inv(transform->Rotation, rotation);
		glm_vec3_negate_to(transform->Position, position);
	}
	else
	{
		Vectors4CopyTo(transform->Rotation, rotation);
		Vectors3CopyTo(transform->Position, position);
	}

	// since we dont need to do a full refresh, do only the calcs needed to save cpu time
	// this engine was orignally designed to be single threaded
	if (HasFlag(mask, RotationModifiedFlag))
	{
		if (transform->RotateAroundCenter)
		{
			glm_quat_rotate(transform->State.ScaleMatrix, rotation, transform->State.RotationMatrix);
		}
		else
		{
			SetMatrices4(transform->State.RotationMatrix, transform->State.ScaleMatrix);
			glm_quat_rotate_at(transform->State.RotationMatrix, rotation, position);
		}
	}

	// since the rotation and scale were not affected only translate and store into the previous state
	glm_translate_to(transform->State.RotationMatrix, position, transform->State.LocalState);

	if (transform->Parent != null)
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

static vec4* ForceRefreshTransform(Transform transform)
{
	// order must be scale first -> then rotate -> then translate
	glm_scale_to(Matrix4.Identity, transform->Scale, transform->State.ScaleMatrix);

	// create a place to store rot and pos for the followup matrix multiplications
	Quaternion rotation;
	vec3 position;

	// check whether or not we should invert the pos and rotation
	// this is typically only used for the camera since the camera never "moves"
	if (transform->InvertTransform)
	{
		glm_quat_inv(transform->Rotation, rotation);
		glm_vec3_negate_to(transform->Position, position);
	}
	else
	{
		Vectors4CopyTo(transform->Rotation, rotation);
		Vectors3CopyTo(transform->Position, position);
	}

	if (transform->RotateAroundCenter)
	{
		glm_quat_rotate(transform->State.ScaleMatrix, rotation, transform->State.RotationMatrix);
	}
	else
	{
		SetMatrices4(transform->State.RotationMatrix, transform->State.ScaleMatrix);
		glm_quat_rotate_at(transform->State.RotationMatrix, rotation, position);
	}

	glm_translate_to(transform->State.RotationMatrix, position, transform->State.LocalState);

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

	// make sure our cardinal directions are forced to recalculate if they are used
	ResetFlags(transform->State.Directions.Accessed);

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
			if (current is transform->Child)
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
			transform->LastChild = child;
		}
	}

	child->Parent = transform;
}

static void SetParent(Transform transform, Transform parent)
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

static void SetPosition(Transform transform, vec3 position)
{
	if (Vector3Equals(position, transform->Position))
	{
		return;
	}

	SetVectors3(transform->Position, position);
	SetFlag(transform->State.Modified, PositionModifiedFlag);
}

static void SetPositions(Transform transform, float x, float y, float z)
{
	float newPos[3] = { x, y, z };

	if (Vector3Equals(transform->Position, newPos))
	{
		return;
	}

	SetVectors3(transform->Position, newPos);

	SetFlag(transform->State.Modified, PositionModifiedFlag);
}

static void SetRotation(Transform transform, Quaternion rotation)
{
	if (Vector4Equals(rotation, transform->Rotation))
	{
		return;
	}

	SetVectors4(transform->Rotation, rotation);
	SetFlag(transform->State.Modified, RotationModifiedFlag);
}

static void SetScale(Transform transform, vec3 scale)
{
	if (Vector3Equals(scale, transform->Scale))
	{
		return;
	}

	SetVectors3(transform->Scale, scale);
	SetFlag(transform->State.Modified, ScaleModifiedFlag);
}

static void SetScales(Transform transform, float x, float y, float z)
{
	float newPos[3] = { x, y, z };

	if (Vector3Equals(transform->Scale, newPos))
	{
		return;
	}

	SetVectors3(transform->Scale, newPos);

	SetFlag(transform->State.Modified, ScaleModifiedFlag);
}

static void AddPosition(Transform transform, vec3 amount)
{
	if (Vector3Equals(amount, Vector3.Zero))
	{
		return;
	}

	AddVectors3(transform->Position, amount);
	SetFlag(transform->State.Modified, PositionModifiedFlag);
}

static void Translate(Transform transform, float x, float y, float z)
{
	vec3 amount = { x, y, z };

	AddPosition(transform, amount);
}

static void TranslateX(Transform transform, float x)
{
	if (x is transform->Position[0])
	{
		return;
	}

	transform->Position[0] += x;

	SetFlag(transform->State.Modified, PositionModifiedFlag);
}

static void TranslateY(Transform transform, float y)
{
	if (y is transform->Position[1])
	{
		return;
	}

	transform->Position[1] += y;

	SetFlag(transform->State.Modified, PositionModifiedFlag);
}

static void TranslateZ(Transform transform, float z)
{
	if (z is transform->Position[2])
	{
		return;
	}

	transform->Position[2] += z;

	SetFlag(transform->State.Modified, PositionModifiedFlag);
}

static void Rotate(Transform transform, Quaternion amount)
{
	// to add a rotation to a quaterion we multiply, gotta love imaginary number magic
	glm_quat_mul(transform->Rotation, amount, transform->Rotation);

	SetFlag(transform->State.Modified, RotationModifiedFlag);
}

static void RotateOnAxis(Transform transform, float angleInRads, vec3 axis)
{
	Quaternion rotation;
	glm_quat(rotation, angleInRads, axis[0], axis[1], axis[2]);

	Rotate(transform, rotation);
	// no need to set flag here since we call the other method that does
}

static void SetRotationOnAxis(Transform transform, float angleInRads, vec3 axis)
{
	Quaternion rotation;
	glm_quat(rotation, angleInRads, axis[0], axis[1], axis[2]);

	SetRotation(transform, rotation);
	// no need to set flag here since we call the other method that does
}

static void AddScale(Transform transform, vec3 amount)
{
	AddVectors3(transform->Scale, amount);
	SetFlag(transform->State.Modified, ScaleModifiedFlag);
}

static void GetDirection(Transform transform, Direction direction, vec3 out_direction)
{
	GuardNotNull(transform);

	if (direction is Directions.Zero)
	{
		Vectors3CopyTo(Vector3.Zero, out_direction);
		return;
	}

	// since we handled Zero as an edge case shift index down by one
	--direction;

	// this is  a magic number but I doubt there will be more than 6 cardinal directions in the future..
	GuardLessThanEqual(direction, 5);

	static float* directions[6] = {
		Vector3.Left,
		Vector3.Right,
		Vector3.Up,
		Vector3.Down,
		Vector3.Forward,
		Vector3.Back
	};

	// first check to see if we have already calc'ed the vector this frame
	// if we have we should just return the pre-calculated one and not re-multiply the rotation
	if (HasFlag(transform->State.Directions.Accessed, FlagN(direction)))
	{
		Vectors3CopyTo(transform->State.Directions.Directions[direction], out_direction);

		return;
	}

	glm_quat_rotatev(transform->Rotation, directions[direction], transform->State.Directions.Directions[direction]);

	Vectors3CopyTo(transform->State.Directions.Directions[direction], out_direction);

	SetFlag(transform->State.Directions.Accessed, FlagN(direction));
}