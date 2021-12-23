#pragma once

#include "math/vectors.h"
#include "math/quaternions.h"

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
	mat4 PreviousScaleMatrix;
	/// <summary>
	/// The previously calculated rotation matrix for this transform, this is NOT initialized with a value
	/// until the first time this transform is refreshed
	/// </summary>
	mat4 PreviousRotationMatrix;
	/// <summary>
	/// The previously calculated trasform matrix for this transform, this is NOT initialized with a value
	/// until the first time this transform is refreshed
	/// </summary>
	mat4 State;
};

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
	struct transformState PreviousState;
};

Transform CreateTransform();

/// <summary>
/// Recalulates the underlying state of the transform
/// </summary>
/// <param name=""></param>
vec4* RefreshTransform(Transform);

/// <summary>
/// Recalulates the underlying state of the transform ignoring any previously saved state
/// </summary>
/// <param name=""></param>
vec4* ForceRefreshTransform(Transform);

void SetPosition(Transform, vec3 position);
void SetRotation(Transform, Quaternion rotation);
void SetScale(Transform, vec3 scale);

void AddPostion(Transform, vec3 amount);
void AddRotation(Transform, Quaternion amount);
void AddScale(Transform, vec3 amount);