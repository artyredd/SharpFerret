#pragma once

#include "math/vectors.h"

/// A rotation vector stored as [x, y, z, w]
typedef vec4 Quaternion;

static struct _quaternionConsts {
	Quaternion Identity;
} Quaternions = {
	.Identity = {0,0,0,1}
};

// Mutates the left quaterion by adding(multiplying for quaternions) the right values to the left
void AddQuaternion(Quaternion left, Quaternion right);

/// Sets the left quaterion's value to the right quaternions values
#define SetQuaternion(left,right) SetVectors4(left,right)