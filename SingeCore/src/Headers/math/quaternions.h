#pragma once

#include "math/vectors.h"

/// A rotation vector stored as [x, y, z, w]
typedef vec4 Quaternion;

static struct _quaternionConsts {
	Quaternion Identity;
} sQuaternion = {
	.Identity = {0,0,0,1}
};