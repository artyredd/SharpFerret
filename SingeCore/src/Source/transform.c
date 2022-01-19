#include "graphics/transform.h"
#include "singine/memory.h"

#include "helpers/macros.h"

#include "cglm/affine.h"
#include "cglm/quat.h"
#include "helpers/quickmask.h"
#include "singine/guards.h"
#include "math/vectors.h"
#include <stdlib.h>
#include "singine/config.h"
#include "singine/parsing.h"

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
static void ScaleAll(Transform, float scaler);
static void ClearChildren(Transform);
static Transform Load(File);
static void Save(Transform, File);
static void SetChildCapacity(Transform, size_t count);

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
	.SetChildCapacity = &SetChildCapacity
};

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

	SafeFree(transform);
}

static Transform CreateTransform()
{
	Transform transform = SafeAlloc(sizeof(struct _transform));

	transform->Children = null;
	transform->Parent = null;
	transform->Length = 0;
	transform->Count = 0;
	transform->FreeIndex = 0;
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

static void TransformCopyTo(Transform source, Transform destination)
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
	for (size_t i = 0; i < transform->Count; i++)
	{
		Transform child = transform->Children[i];

		if (child isnt null)
		{
			SetFlag(child->State.Modified, ParentModifiedFlag);
		}
	}
}

static float* GetPosition(Transform transform)
{
	if (transform->InvertTransform)
	{
		vec3 result;
		glm_vec3_negate_to(transform->Position, result);;
		return result;
	}

	return transform->Position;
}

static float* GetRotation(Transform transform)
{
	if (transform->InvertTransform)
	{
		Quaternion result;
		glm_quat_inv(transform->Rotation, result);

		return result;
	}

	return transform->Rotation;
}

static vec4* RefreshTransform(Transform transform)
{
	return ForceRefreshTransform(transform);

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
	// becuase this engine is single threaded we perform some extra calculations here to save them in non-forced refresh

	// calc and store scale, rotation, and translation in their own matrices so we can selectively update them later

	// calc and store scale matrix
	glm_scale_to(Matrix4.Identity, transform->Scale, transform->State.ScaleMatrix);

	// becuase the transform may be inverted grab either the regular or inverted transforms
	float* rotation = GetRotation(transform);
	float* position = GetPosition(transform);

	// create and store a rotation matrix
	glm_quat_rotate(Matrix4.Identity, rotation, transform->State.RotationMatrix);

	// create and store a translation matrix
	glm_translate_to(Matrix4.Identity, position, transform->State.TranslationMatrix);

	// multiply them in reverse order
	// order should be scale -> rotate -> translate
	glm_mat4_mulN((mat4 * []) {
		&transform->State.TranslationMatrix, & transform->State.RotationMatrix, & transform->State.ScaleMatrix
	}, 3, transform->State.LocalState);

	// if we have a parent we should grab their matrices and multiply it with our local one
	if (transform->Parent != null)
	{
		glm_mat4_mul(transform->Parent->State.State, transform->State.LocalState, transform->State.State);
	}
	else
	{
		// since we don't have a parent copy the local transform to the state
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
	if (TryRealloc(transform->Children, previousLength, newLength, (void**)&transform->Children) is false)
	{
		Transform* newArray = SafeAlloc(newLength);

		for (size_t i = 0; i < transform->Length; i++)
		{
			newArray[i] = transform->Children[i];
		}

		SafeFree(transform->Children);

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
		transform->Children = SafeAlloc(sizeof(Transform) * count);
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
	}

	transform->FreeIndex = 0;
	transform->Count = 0;
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

static void ScaleAll(Transform transform, float scaler)
{
	float* scale = transform->Scale;

	scale[0] *= scaler;
	scale[1] *= scaler;
	scale[2] *= scaler;

	SetFlag(transform->State.Modified, ScaleModifiedFlag);
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

#define CommentFormat "%s\n"
#define TokenFormat "%s: "

#define PositionTokenComment "# the position of the object, this may be in world space or screen space depending if the object is rendered with camera perspective"
#define PositionToken "position"
#define RotationTokenComment "# the rotation of the object"
#define RotationToken "rotation"
#define ScaleTokenComment "# the scale of the object"
#define ScaleToken "scale"
#define InvertTransformTokenComment "# whether or not the values listed are inverted before the object is rendered"
#define InvertTransformToken "invertTransform"
#define RotateAroundCenterTokenComment "# whether or not the object is rotated around world center or it's position in world/screen space"
#define RotateAroundCenterToken "rotateAroundCenter"

static const char* Tokens[] = {
	PositionToken,
	RotationToken,
	ScaleToken,
	InvertTransformToken,
	RotateAroundCenterToken
};

static const size_t TokenLengths[] = {
	sizeof(PositionToken),
	sizeof(RotationToken),
	sizeof(ScaleToken),
	sizeof(InvertTransformToken),
	sizeof(RotateAroundCenterToken)
};

struct _gameObjectInfo {
	vec3 Position;
	vec4 Rotation;
	vec3 Scale;
	bool RotateAroundCenter;
	bool InvertTransform;
};

static bool OnTokenFound(size_t index, const char* buffer, const size_t length, struct _gameObjectInfo* state);

struct _configDefinition TransformConfigDefinition = {
	.Tokens = (const char**)&Tokens,
	.TokenLengths = (const size_t*)&TokenLengths,
	.CommentCharacter = '#',
	.Count = sizeof(Tokens) / sizeof(char*),
	.OnTokenFound = &OnTokenFound
};

static bool OnTokenFound(size_t index, const char* buffer, const size_t length, struct _gameObjectInfo* state)
{
	switch (index)
	{
	case 0: // position
		return Vector3s.TryDeserialize(buffer, length, state->Position);
	case 1: // rotation
		return Vector4s.TryDeserialize(buffer, length, state->Rotation);
	case 2: // scale
		return Vector3s.TryDeserialize(buffer, length, state->Scale);
	case 3: // InvertTransform
		bool shouldInvert = false;
		if (TryParseBoolean(buffer, length, &shouldInvert))
		{
			state->InvertTransform = shouldInvert;
		}
		return true;
	case 4: // RotateAroundCenter
		bool shouldRotateAroundCenter = false;
		if (TryParseBoolean(buffer, length, &shouldRotateAroundCenter))
		{
			state->RotateAroundCenter = shouldRotateAroundCenter;
		}
		return true;
	default:
		return false;
	}
}

static Transform Load(File stream)
{
	Transform transform = null;

	struct _gameObjectInfo state = {
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

		Vectors3CopyTo(state.Scale, transform->Scale);
		Vectors3CopyTo(state.Position, transform->Position);
		Vectors4CopyTo(state.Rotation, transform->Rotation);
	}

	return transform;
}

static void Save(Transform transform, File stream)
{
	GuardNotNull(transform);
	GuardNotNull(stream);

	fprintf(stream, CommentFormat, PositionTokenComment);
	fprintf(stream, TokenFormat, PositionToken);
	Vector3s.TrySerializeStream(stream, transform->Position);
	fprintf(stream, "%c", '\n');

	fprintf(stream, CommentFormat, RotationTokenComment);
	fprintf(stream, TokenFormat, RotationToken);
	Vector4s.TrySerializeStream(stream, transform->Rotation);
	fprintf(stream, "%c", '\n');

	fprintf(stream, CommentFormat, ScaleTokenComment);
	fprintf(stream, TokenFormat, ScaleToken);
	Vector3s.TrySerializeStream(stream, transform->Scale);
	fprintf(stream, "%c", '\n');

	fprintf(stream, CommentFormat, InvertTransformTokenComment);
	fprintf(stream, TokenFormat, InvertTransformToken);
	fprintf(stream, "%s", transform->InvertTransform ? "true" : "false");
	fprintf(stream, "%c", '\n');

	fprintf(stream, CommentFormat, RotateAroundCenterTokenComment);
	fprintf(stream, TokenFormat, RotateAroundCenterToken);
	fprintf(stream, "%s", transform->RotateAroundCenter ? "true" : "false");
	fprintf(stream, "%c", '\n');
}