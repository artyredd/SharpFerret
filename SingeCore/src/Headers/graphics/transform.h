#pragma once

#include "math/vectors.h"
#include "math/quaternions.h"
#include "csharp.h"

struct _directionStates {
	// Represents whether the directions were accessed and lazily assigned this frame, this resets to 0 everytime the transform is modified
	// the first time one of the directions is accessed after a transform has been updated it is flags in this int so we don't re-calculate it
	// for this frame
	unsigned int Accessed;
	// array of vector3's that represent already calculated directions for a transform, order: left, right, up, down, forward, back
	vec3 Directions[6];
};

struct transformState {
	/// <summary>
	/// Modified state is an unsigned int used as a bit mas to identify if various elements of this
	/// state have been modified
	/// format: 0 : unmodified; != 0 : modified, all other values are implementation defined but are as follows:
	/// 0x01 : position modified; 0x02 rotation modified; 0x04 scale modified; Any or all flags may be present
	/// </summary>
	unsigned int Modified;
	/// <summary>
	/// The previously calculated scale matrix for this transform, this is NOT initialized with a value
	/// until the first time this transform is refreshed
	/// </summary>
	mat4 ScaleMatrix;
	/// <summary>
	/// The previously calculated rotation matrix for this transform, this is NOT initialized with a value
	/// until the first time this transform is refreshed
	/// </summary>
	mat4 RotationMatrix;
	/// <summary>
	/// The previously calculated trasform matrix for this transform NOT including the parent matrix, this is NOT initialized with a value
	/// until the first time this transform is refreshed
	/// </summary>
	mat4 LocalState;
	/// <summary>
	/// The previously calculated trasform matrix for this transform, INCLUDING the parent's matrix, this is NOT initialized with a value
	/// until the first time this transform is refreshed
	/// </summary>
	mat4 State;
	/// <summary>
	/// Stores the pre-calculated directions for this object during runtime
	/// </summary>
	struct _directionStates Directions;
};

typedef struct _transform* Transform;

struct _transform {
	// Linked list of the transform tree
	/// <summary>
	/// The parent of this transform
	/// </summary>
	Transform Parent;
	Transform* Children;
	/// <summary>
	/// The length of the children array, should always be >= count
	/// </summary>
	size_t Length;
	/// <summary>
	/// The number of children in this transform
	/// </summary>
	size_t Count;
	/// <summary>
	/// The index of an open child spot within the children array, this is 0 when there is no known spot
	/// </summary>
	size_t FreeIndex;
	// transform members
	/// <summary>
	/// The position of this transform in world space
	/// </summary>
	vec3 Position;
	/// <summary>
	/// The rotation of this transform
	/// </summary>
	Quaternion Rotation;
	/// <summary>
	/// The scale of this transform
	/// </summary>
	vec3 Scale;
	/// <summary>
	/// Whether rotations should be calculated from the world origin or the transforms position, think planet rotation around sun(origin) vs planet rotating by itself (around position)
	/// </summary>
	bool RotateAroundCenter;
	/// <summary>
	/// Whether or not this transform should be inverted when drawn, this is mostly used for the camera but could be used for any transform
	/// </summary>
	bool InvertTransform;
	/// <summary>
	/// The stored state of this transform
	/// </summary>
	struct transformState State;
};

struct _transformMethods {
	Transform(*Create)(void);

	void (*CopyTo)(Transform source, Transform destination);

	Transform(*DuplicateTransform)(Transform);

	/// <summary>
	/// Recalulates the underlying state of the transform
	/// </summary>
	/// <param name=""></param>
	vec4* (*Refresh)(Transform);

	/// <summary>
	/// Recalulates the underlying state of the transform ignoring any previously saved state
	/// </summary>
	/// <param name=""></param>
	vec4* (*ForceRefresh)(Transform);

	void (*SetPosition)(Transform, vec3 position);
	void (*SetPositions)(Transform transform, float x, float y, float z);
	void (*SetRotation)(Transform, Quaternion rotation);
	void (*ScaleAll)(Transform, float scaler);
	void (*SetScale)(Transform, vec3 scale);
	void (*SetScales)(Transform transform, float x, float y, float z);

	void (*AddPosition)(Transform, vec3 amount);
	void (*Translate)(Transform, float x, float y, float z);
	void (*TranslateX)(Transform, float x);
	void (*TranslateY)(Transform, float y);
	void (*TranslateZ)(Transform, float z);
	void (*Rotate)(Transform, Quaternion amount);
	void (*RotateOnAxis)(Transform, float amountInRads, vec3 axis);
	void (*SetRotationOnAxis)(Transform, float amountInRads, vec3 axis);
	void (*AddScale)(Transform, vec3 amount);

	// Sets the provided transform's parent as the provided parent
	void (*SetParent)(Transform, Transform parent);

	// Resizes the child array to fit count elements
	void (*SetChildCapacity)(Transform, size_t count);

	void (*GetDirection)(Transform transform, Direction directions, vec3 out_direction);
	void (*ClearChildren)(Transform transform);
	void (*Dispose)(Transform);

	/// <summary>
	/// Creates a transform by deserializing it from the provided stream
	/// </summary>
	Transform(*Load)(File stream);
	/// <summary>
	/// Saves a transform by serializing it to the provided stream
	/// </summary>
	void (*Save)(Transform, File stream);
};

extern const struct _transformMethods Transforms;