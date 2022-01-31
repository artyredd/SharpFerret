#include "math/quaternions.h"
#include "cglm/quat.h"

static void AddQuaternion(Quaternion left, Quaternion right);
static void CopyTo(Quaternion source, Quaternion destination);
static void Invert(Quaternion quaternion);

const struct _quaternionMethods Quaternions = {
	.Identity = {0,0,0,1},
	.Add = &AddQuaternion,
	.CopyTo = &CopyTo,
	.Invert = &Invert
};

// Mutates the left quaterion by adding(multiplying for quaternions) the right values to the left
static void AddQuaternion(Quaternion left, Quaternion right)
{
	// the only reason this isn't a macro is so we can avoid having cglm as a dependency
	// 
	// to add a rotation to a quaterion we multiply, gotta love imaginary number magic
	glm_quat_mul(left, right, left);
}

static void CopyTo(Quaternion source, Quaternion destination)
{
	Vectors4CopyTo(source, destination);
}

static void Invert(Quaternion quaternion)
{
	glm_quat_inv(quaternion, quaternion);
}