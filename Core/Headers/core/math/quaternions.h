#pragma once

#include "core/math/vectors.h"

/// A rotation vector stored as [x, y, z, w]
typedef struct quaternion quaternion;

struct quaternion
{
	vector4;
};

struct _quaternionMethods {
	quaternion Identity;
	quaternion (*Create)(float angle, vector3 axis);
	quaternion (*Add)(quaternion left, quaternion right);
	quaternion (*Invert)(quaternion quaternion);
	vector3 (*RotateVector)(quaternion, vector3 vector);
	matrix4 (*RotateMatrix)(quaternion, matrix4);
	quaternion (*LookAt)(vector3 origin, vector3 target, vector3 upAxis);
	bool (*Equals)(quaternion, quaternion);
	bool (*TryDeserialize)(const char* buffer, const ulong length, quaternion* out_vector4);
	bool (*TrySerialize)(char* buffer, const ulong length, const quaternion vector);
	bool (*TrySerializeStream)(File stream, const quaternion vector);
};

extern const struct _quaternionMethods Quaternions;