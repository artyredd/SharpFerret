#pragma once

#include "math/vectors.h"
#include "math/quaternions.h"

typedef struct _transform* Transform;

struct _transform {
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
	/// The stored state of this transform
	/// </summary>
	mat4 PreviousState;
	/// <summary>
	/// This flag denotes whethere or not this transform has been modified
	/// When a transform is modified in any way a new Matrix must be computed
	/// </summary>
	bool Modified;
};

Transform CreateTransform();

/// <summary>
/// Recalulates the underlying state of the transform
/// </summary>
/// <param name=""></param>
void RefreshTransform(Transform);

/// <summary>
/// Recalulates the underlying state of the transform ignoring any previously saved state
/// </summary>
/// <param name=""></param>
void ForceRefreshTransform(Transform);