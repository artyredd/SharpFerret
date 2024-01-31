#include "engine/graphics/transform.h"
#include "core/memory.h"

#include "core/macros.h"

#include "cglm/affine.h"
#include "cglm/quat.h"
#include "core/quickmask.h"
#include "core/guards.h"
#include "core/math/vectors.h"
#include <stdlib.h>
#include "core/config.h"
#include "core/parsing.h"

#define PositionModifiedFlag FLAG_0
#define RotationModifiedFlag FLAG_1
#define ScaleModifiedFlag FLAG_2
#define ParentModifiedFlag FLAG_3

#define AllModifiedFlag (PositionModifiedFlag | RotationModifiedFlag | ScaleModifiedFlag)

static void Dispose(Transform transform);
static Transform CreateTransform(void);
static void TransformCopyTo(Transform source, Transform destination);
static void SetParent(Transform transform, Transform parent);
static void SetPosition(Transform transform, vector3 position);
static void SetPositions(Transform transform, float x, float y, float z);
static void SetRotation(Transform transform, quaternion rotation);
static void SetScale(Transform transform, vector3 scale);
static void SetScales(Transform transform, float x, float y, float z);
static void AddPosition(Transform transform, vector3 amount);
static void Translate(Transform, float x, float y, float z);
static void TranslateX(Transform, float x);
static void TranslateY(Transform, float y);
static void TranslateZ(Transform, float z);
static void Rotate(Transform transform, quaternion amount);
static void RotateOnAxis(Transform, float amountInRads, vector3 axis);
static void SetRotationOnAxis(Transform, float amountInRads, vector3 axis);
static void AddScale(Transform transform, vector3 amount);
static vector3 GetDirection(Transform transform, Direction direction);
static matrix4 RefreshTransform(Transform transform);
static matrix4 ForceRefreshTransform(Transform transform);
static void ScaleAll(Transform, float scaler);
static void ClearChildren(Transform);
static Transform Load(File);
static void Save(Transform, File);
static void SetChildCapacity(Transform, size_t count);
static void LookAt(Transform, vector3 target);
static void LookAtPositions(Transform, float x, float y, float z);
static vector3 TransformPoint(Transform, vector3 point);

const struct _transformMethods Transforms = {
	.Dispose = &Dispose,
	.Create = &CreateTransform,
	.CopyTo = &TransformCopyTo,
	.SetParent = &SetParent,
	.SetPosition = &SetPosition,
	.SetPositions = &SetPositions,
	.SetRotation = &SetRotation,
	.ScaleAll = &ScaleAll,
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
	.TranslateZ = &TranslateZ,
	.ClearChildren = &ClearChildren,
	.Save = &Save,
	.Load = &Load,
	.SetChildCapacity = &SetChildCapacity,
	.LookAt = LookAt,
	.LookAtPositions = LookAtPositions,
	.TransformPoint = TransformPoint
};

DEFINE_TYPE_ID(Transform);

static void Dispose(Transform transform)
{
	if (transform is null)
	{
		return;
	}

	if (transform->Count isnt 0)
	{
		ClearChildren(transform);
	}

	if (transform->Parent isnt null)
	{
		SetParent(transform, null);
	}

	Memory.Free(transform, TransformTypeId);
}

static Transform CreateTransform()
{
	Memory.RegisterTypeName(nameof(Transform), &TransformTypeId);

	Transform transform = Memory.Alloc(sizeof(struct _transform), TransformTypeId);

	transform->Children = null;
	transform->Parent = null;
	transform->Length = 0;
	transform->Count = 0;
	transform->FreeIndex = 0;
	transform->RotateAroundCenter = false;
	transform->InvertTransform = false;

	transform->Position = Vector3.Zero;
	transform->Scale = (vector3){ 1, 1, 1 };
	transform->Rotation = (quaternion){ 0, 0, 0, 1 };

	transform->State.State = Matrix4.Zero;

	// set the all modified flag so we do a full refresh of the transform on first draw
	transform->State.Modified = AllModifiedFlag;
	ResetFlags(transform->State.Directions.Accessed);

	// no need to init the transform states since they will be populated before first draw by RecalculateTransform

	return transform;
}

static void DirectionStatesCopyTo(struct _directionStates* source, struct _directionStates* destination)
{
	CopyMember(source, destination, Accessed);

	for (size_t i = 0; i < 6; i++)
	{
		destination->Directions[i] = source->Directions[i];
	}
}

static void StateCopyTo(struct transformState* source, struct transformState* destination)
{
	CopyMember(source, destination, Modified);

	destination->ScaleMatrix = source->ScaleMatrix;
	destination->RotationMatrix = source->RotationMatrix;
	destination->LocalState = source->LocalState;
	destination->State = source->State;

	DirectionStatesCopyTo(&source->Directions, &destination->Directions);
}

static void TransformCopyTo(Transform source, Transform destination)
{
	CopyMember(source, destination, RotateAroundCenter);
	CopyMember(source, destination, InvertTransform);

	destination->Scale = source->Scale;
	destination->Position = source->Position;
	destination->Rotation = source->Rotation;

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
	for (size_t i = 0; i < transform->Count; i++)
	{
		Transform child = transform->Children[i];

		if (child isnt null)
		{
			SetFlag(child->State.Modified, ParentModifiedFlag);
		}
	}
}

static vector3 GetPosition(Transform transform)
{
	if (transform->InvertTransform)
	{
		return (vector3)
		{
			-transform->Position.x,
				-transform->Position.y,
				-transform->Position.z
		};
	}

	return transform->Position;
}

static quaternion GetRotation(Transform transform)
{
	if (transform->InvertTransform)
	{
		return Quaternions.Invert(transform->Rotation);
	}

	return transform->Rotation;
}

static matrix4 RefreshTransform(Transform transform)
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
		transform->State.State = Matrix4s.Multiply(transform->Parent->State.State, transform->State.LocalState);

		ResetFlags(transform->State.Modified);

		NotifyChildren(transform);

		return transform->State.State;
	}

	// check to see if we should perform a whole refresh
	// since the transform starts with position a change to it requires a whole refresh by default
	if (HasFlag(mask, AllModifiedFlag) || HasFlag(mask, PositionModifiedFlag))
	{
		return Transforms.ForceRefresh(transform);
	}

	// becuase the transform may be inverted grab either the regular or inverted transforms
	quaternion rotation = GetRotation(transform);

	// since we dont need to do a full refresh, do only the calcs needed to save cpu time
	// this engine was orignally designed to be single threaded
	if (HasFlag(mask, RotationModifiedFlag))
	{
		transform->State.RotationMatrix = Quaternions.RotateMatrix(rotation, transform->State.TranslationMatrix);
	}

	// scale the matrix
	transform->State.LocalState = Matrix4s.Scale(transform->State.RotationMatrix, transform->Scale);

	if (transform->Parent != null)
	{
		transform->State.State = Matrix4s.Multiply(transform->Parent->State.State, transform->State.LocalState);
	}
	else
	{
		transform->State.State = transform->State.LocalState;
	}

	ResetFlags(transform->State.Modified);

	NotifyChildren(transform);

	return transform->State.State;
}

static matrix4 ForceRefreshTransform(Transform transform)
{
	// becuase this engine is single threaded we perform some extra calculations here to save them in non-forced refresh

	// calc and store scale, rotation, and translation in their own matrices so we can selectively update them later

	// becuase the transform may be inverted grab either the regular or inverted transforms
	quaternion rotation = GetRotation(transform);

	vector3 position = GetPosition(transform);

	// create and store a translation matrix
	transform->State.TranslationMatrix = Matrix4s.Translate(Matrix4.Identity, position);

	// create and store a rotation matrix
	transform->State.RotationMatrix = Quaternions.RotateMatrix(rotation, transform->State.TranslationMatrix);

	// calc and store scale matrix
	transform->State.LocalState = Matrix4s.Scale(transform->State.RotationMatrix, transform->Scale);

	// multiply them in reverse order
	// order should be scale -> rotate -> translate
	/*glm_matrix4_mulN((matrix4 * []) {
		&transform->State.TranslationMatrix, & transform->State.RotationMatrix, & transform->State.ScaleMatrix
	}, 3, transform->State.LocalState);*/

	// if we have a parent we should grab their matrices and multiply it with our local one
	if (transform->Parent != null)
	{
		transform->State.State = Matrix4s.Multiply(transform->Parent->State.State, transform->State.LocalState);
	}
	else
	{
		// since we don't have a parent copy the local transform to the state
		transform->State.State = transform->State.LocalState;
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

	if (child is null)
	{
		return;
	}

	// while it doesnt make sense to call this with a null child
	// the expected result of removing nothing, is nothing.. so just return
	if (child is null)
	{
		return;
	}

	for (size_t i = 0; i < transform->Count; i++)
	{
		Transform current = transform->Children[i];

		if (current is child)
		{
			// detach the child
			transform->Children[i] = null;
			current->Parent = null;

			// mark the child so it's next refresh removes the parents transform
			SetFlag(current->State.Modified, ParentModifiedFlag);

			// decrement the count
			--(transform->Count);

			break;
		}
	}
}

static void AttachChildAtIndex(Transform transform, Transform child, size_t index)
{
	transform->Children[index] = child;

	// check to see if there is an open spot that we can place the next child
	++(index);

	if (index < transform->Length && transform->Children[index] is null)
	{
		transform->FreeIndex = index;
	}
	else
	{
		transform->FreeIndex = 0;
	}
}

static void ReallocChildren(Transform transform, size_t newCount)
{
	// try to realloc the space otherwise manually move it, alloc 25% more space, or + 1
	size_t previousLength = transform->Length * sizeof(Transform);
	size_t newLength = newCount * sizeof(Transform);

	// if we fail to realloc the array make a new one =(
	if (Memory.TryRealloc(transform->Children, previousLength, newLength, (void**)&transform->Children) is false)
	{
		Transform* newArray = Memory.Alloc(newLength, TransformTypeId);

		for (size_t i = 0; i < transform->Length; i++)
		{
			newArray[i] = transform->Children[i];
		}

		Memory.Free(transform->Children, TransformTypeId);

		transform->Children = newArray;
	}

	// since we know the first index of the extended portion is null we can set the lastchild index
	// to that value so we dont have to search for it later
	transform->FreeIndex = transform->Length;

	// update the length
	transform->Length = newCount;
}

static void SetChildCapacity(Transform transform, size_t count)
{
	GuardNotNull(transform);
	GuardNotZero(count);

	if (transform->Length is 0)
	{
		transform->Children = Memory.Alloc(sizeof(Transform) * count, TransformTypeId);
		transform->Length = count;
		transform->FreeIndex = 0;

		return;
	}

	if (transform->Length < count)
	{
		ReallocChildren(transform, count);
	}
}

static void AttachChild(Transform transform, Transform child)
{
	if (transform is null or child is null)
	{
		return;
	}

	// make sure there is enough room, if there isn't move the array
	if ((transform->Count) + 1 > transform->Length)
	{
		// try to realloc the array if not make a new one and copy the children over

		// make it 25% larger to allow for future attachments
		size_t newCount = max((size_t)((transform->Length * 125) / 100), transform->Length + 1);

		ReallocChildren(transform, newCount);
	}

	// if we extended the array lastchild will now be non-zero
	if (transform->Children[transform->FreeIndex] is null)
	{
		AttachChildAtIndex(transform, child, transform->FreeIndex);
	}
	else
	{
		// linearly search for open spot
		for (size_t i = 0; i < transform->Length; i++)
		{
			if (transform->Children[i] is null)
			{
				// attach the child and check to see if we can save some time
				// next call by checking for null spot next to the index
				AttachChildAtIndex(transform, child, i);
				break;
			}
		}
	}

	//attach the parent to the child
	child->Parent = transform;

	++(transform->Count);
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

static void ClearChildren(Transform transform)
{
	for (size_t i = 0; i < transform->Count; i++)
	{
		Transform child = transform->Children[i];

		if (child isnt null)
		{
			child->Parent = null;
			SetFlag(child->State.Modified, ParentModifiedFlag);
		}

		transform->Children[i] = null;

		//Transforms.Dispose( child );
	}

	transform->FreeIndex = 0;
	transform->Count = 0;
}

static void SetPosition(Transform transform, vector3 position)
{
	if (Vector3s.Equals(position, transform->Position))
	{
		return;
	}

	transform->Position = position;
	SetFlag(transform->State.Modified, PositionModifiedFlag);
}

static void SetPositions(Transform transform, float x, float y, float z)
{
	vector3 newPos = { x, y, z };

	if (Vector3s.Equals(transform->Position, newPos))
	{
		return;
	}

	transform->Position = newPos;

	SetFlag(transform->State.Modified, PositionModifiedFlag);
}

static void SetRotation(Transform transform, quaternion rotation)
{
	if (Quaternions.Equals(rotation, transform->Rotation))
	{
		return;
	}

	transform->Rotation = rotation;

	SetFlag(transform->State.Modified, RotationModifiedFlag);
}

static void LookAt(Transform transform, vector3 target)
{
	transform->Rotation = Quaternions.LookAt(transform->Position, target, Vector3.Up);

	SetFlag(transform->State.Modified, RotationModifiedFlag);
}

static void LookAtPositions(Transform transform, float x, float y, float z)
{
	vector3 target = { x, y, z };

	transform->Rotation = Quaternions.LookAt(transform->Position, target, Vector3.Up);

	SetFlag(transform->State.Modified, RotationModifiedFlag);
}

static void ScaleAll(Transform transform, float scalar)
{
	transform->Scale = Vector3s.Scale(transform->Scale, scalar);

	SetFlag(transform->State.Modified, ScaleModifiedFlag);
}

static void SetScale(Transform transform, vector3 scale)
{
	if (Vector3s.Equals(scale, transform->Scale))
	{
		return;
	}

	transform->Scale = scale;

	SetFlag(transform->State.Modified, ScaleModifiedFlag);
}

static void SetScales(Transform transform, float x, float y, float z)
{
	vector3 newPos = { x, y, z };

	if (Vector3s.Equals(transform->Scale, newPos))
	{
		return;
	}

	transform->Scale = newPos;

	SetFlag(transform->State.Modified, ScaleModifiedFlag);
}

static void AddPosition(Transform transform, vector3 amount)
{
	if (Vector3s.Equals(amount, Vector3.Zero))
	{
		return;
	}

	transform->Position = Vector3s.Add(transform->Position, amount);

	SetFlag(transform->State.Modified, PositionModifiedFlag);
}

static void Translate(Transform transform, float x, float y, float z)
{
	vector3 amount = { x, y, z };

	AddPosition(transform, amount);
}

static void TranslateX(Transform transform, float x)
{
	if (x is transform->Position.x)
	{
		return;
	}

	transform->Position.x += x;

	SetFlag(transform->State.Modified, PositionModifiedFlag);
}

static void TranslateY(Transform transform, float y)
{
	if (y is transform->Position.y)
	{
		return;
	}

	transform->Position.y += y;

	SetFlag(transform->State.Modified, PositionModifiedFlag);
}

static void TranslateZ(Transform transform, float z)
{
	if (z is transform->Position.z)
	{
		return;
	}

	transform->Position.z += z;

	SetFlag(transform->State.Modified, PositionModifiedFlag);
}

static void Rotate(Transform transform, quaternion amount)
{
	// to add a rotation to a quaterion we multiply, gotta love imaginary number magic
	transform->Rotation = Quaternions.Add(transform->Rotation, amount);

	SetFlag(transform->State.Modified, RotationModifiedFlag);
}

static void RotateOnAxis(Transform transform, float angleInRads, vector3 axis)
{
	quaternion rotation = Quaternions.Create(angleInRads, axis);

	Rotate(transform, rotation);
	// no need to set flag here since we call the other method that does
}

static void SetRotationOnAxis(Transform transform, float angleInRads, vector3 axis)
{
	quaternion rotation = Quaternions.Create(angleInRads, axis);

	SetRotation(transform, rotation);
	// no need to set flag here since we call the other method that does
}

static void AddScale(Transform transform, vector3 amount)
{
	transform->Scale = Vector3s.Add(transform->Scale, amount);

	SetFlag(transform->State.Modified, ScaleModifiedFlag);
}

static vector3 GetDirection(Transform transform, Direction direction)
{
	GuardNotNull(transform);

	if (direction is Directions.Zero)
	{
		return Vector3.Zero;
	}

	// since we handled Zero as an edge case shift index down by one
	--direction;

	// this is  a magic number but I doubt there will be more than 6 cardinal directions in the future..
	GuardLessThanEqual(direction, 5);

	vector3 directions[6] = {
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
		return transform->State.Directions.Directions[direction];
	}

	transform->State.Directions.Directions[direction] = Quaternions.RotateVector(transform->Rotation, directions[direction]);

	SetFlag(transform->State.Directions.Accessed, FlagN(direction));

	return transform->State.Directions.Directions[direction];
}

static vector3 TransformPoint(Transform transform, vector3 point)
{
	RefreshTransform(transform);

	return Matrix4s.MultiplyVector3(transform->State.State, point, 1.0);
}

struct _transformInfo {
	vector3 Position;
	quaternion Rotation;
	vector3 Scale;
	bool RotateAroundCenter;
	bool InvertTransform;
};

TOKEN_LOAD(position, struct _transformInfo*)
{
	return Vector3s.TryDeserialize(buffer, length, &state->Position);
}

TOKEN_SAVE(position, Transform)
{
	Vector3s.TrySerializeStream(stream, state->Position);
}

TOKEN_LOAD(rotation, struct _transformInfo*)
{
	return Quaternions.TryDeserialize(buffer, length, &state->Rotation);
}

TOKEN_SAVE(rotation, Transform)
{
	Quaternions.TrySerializeStream(stream, state->Rotation);
}

TOKEN_LOAD(scale, struct _transformInfo*)
{
	return Vector3s.TryDeserialize(buffer, length, &state->Scale);
}

TOKEN_SAVE(scale, Transform)
{
	Vector3s.TrySerializeStream(stream, state->Scale);
}

TOKEN_LOAD(invertTransform, struct _transformInfo*)
{
	return Parsing.TryGetBool(buffer, length, &state->InvertTransform);
}

TOKEN_SAVE(invertTransform, Transform)
{
	fprintf(stream, "%s", state->InvertTransform ? "true" : "false");
}

TOKEN_LOAD(rotateAroundCenter, struct _transformInfo*)
{
	return Parsing.TryGetBool(buffer, length, &state->RotateAroundCenter);
}

TOKEN_SAVE(rotateAroundCenter, Transform)
{
	fprintf(stream, "%s", state->RotateAroundCenter ? "true" : "false");
}

TOKENS(5)
{
	TOKEN(position, "# the position of the object, this may be in world space or screen space depending if the object is rendered with camera perspective"),
		TOKEN(rotation, "# the rotation of the object"),
		TOKEN(scale, "# the scale of the object"),
		TOKEN(invertTransform, "# whether or not the values listed are inverted before the object is rendered"),
		TOKEN(rotateAroundCenter, "# whether or not the object is rotated around world center or it's position in world/screen space")
};

struct _configDefinition TransformConfigDefinition = {
	.Tokens = Tokens,
	.CommentCharacter = '#',
	.Count = sizeof(Tokens) / sizeof(struct _configToken),
};

static Transform Load(File stream)
{
	Transform transform = null;

	struct _transformInfo state = {
		.InvertTransform = false,
		.RotateAroundCenter = false,
		.Position = { 0, 0, 0},
		.Rotation = { 0, 0, 0, 1},
		.Scale = { 1, 1, 1}
	};

	if (Configs.TryLoadConfigStream(stream, &TransformConfigDefinition, &state))
	{
		transform = CreateTransform();

		transform->InvertTransform = state.InvertTransform;
		transform->RotateAroundCenter = state.InvertTransform;

		transform->Scale = state.Scale;
		transform->Position = state.Position;
		transform->Rotation = state.Rotation;
	}

	return transform;
}

static void Save(Transform transform, File stream)
{
	GuardNotNull(transform);
	GuardNotNull(stream);

	Configs.SaveConfigStream(stream, &TransformConfigDefinition, transform);
}