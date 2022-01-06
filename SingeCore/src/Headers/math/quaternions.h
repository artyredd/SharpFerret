#pragma once

#include "math/vectors.h"

/// A rotation vector stored as [x, y, z, w]
typedef vec4 Quaternion;

struct _quaternionMethods {
	Quaternion Identity;
	// Mutates the left quaterion by adding(multiplying for quaternions) the right values to the left
	void (*Add)(Quaternion left, Quaternion right);
};

extern const struct _quaternionMethods Quaternions;