#include "math/quaternions.h"
#include "cglm/quat.h"
#include "math/vectors.h"

static quaternion AddQuaternion(quaternion left, quaternion right);
static quaternion Invert(quaternion quaternion);
static bool TryDeserialize(const char* buffer, const size_t length, quaternion* out_quaternion);
static bool TrySerialize(char* buffer, const size_t length, const quaternion vector);
static bool TrySerializeStream(File stream, const quaternion vector);
static quaternion Create(const float angle, const vector3 axis);
static vector3 RotateVector(quaternion, vector3 vector);
static matrix4 RotateMat4(quaternion angle, matrix4);
static bool Equals(quaternion, quaternion);
static quaternion LookAt(vector3 origin, vector3 target, vector3 upAxis);

const struct _quaternionMethods Quaternions = {
	.Identity = { 0, 0, 0, 1},
	.Add = &AddQuaternion,
	.Invert = &Invert,
	.Create = Create,
	.RotateVector = RotateVector,
	.RotateMatrix = RotateMat4,
	.TryDeserialize = TryDeserialize,
	.TrySerialize = TrySerialize,
	.TrySerializeStream = TrySerializeStream,
	.Equals = Equals,
	.LookAt = LookAt
};

static bool TryDeserialize(const char* buffer, const size_t length, quaternion* out_rotation)
{
	return Vector4s.TryDeserialize(buffer, length, (vector4*)out_rotation);
}

static bool TrySerialize(char* buffer, const size_t length, const quaternion rotation)
{
	return Vector4s.TrySerialize(buffer, length, *(vector4*)&rotation);
}

static bool TrySerializeStream(File stream, const quaternion rotation)
{
	return Vector4s.TrySerializeStream(stream, *(vector4*)&rotation);
}

static quaternion Create(const float angle, const vector3 axis)
{
	// create a quaternion that represents spinning around the Y axis(horizontally spinning)
	quaternion result;
	glm_quat((float*)&result, angle, axis.x, axis.y, axis.z);

	return result;
}


// Mutates the left quaterion by adding(multiplying for quaternions) the right values to the left
static quaternion AddQuaternion(quaternion left, quaternion right)
{
	// the only reason this isn't a macro is so we can avoid having cglm as a dependency
	// 
	// to add a rotation to a quaterion we multiply, gotta love imaginary number magic
	quaternion result;
	glm_quat_mul((float*)&left, (float*)&right, (float*) & result);

	return result;
}

static quaternion Invert(quaternion quaternion)
{
	struct quaternion result;
	glm_quat_inv((float*)&quaternion, (float*) & result);

	return result;
}

static vector3 RotateVector(quaternion rotation, vector3 vector)
{
	vector3 result;
	glm_quat_rotatev((float*)&rotation, (float*) & vector, (float*) & result);

	return result;
}

static matrix4 RotateMat4(quaternion rotation, matrix4 matrix)
{
	matrix4 result;
	glm_quat_rotate( (vec4*)&matrix, (float*)&rotation, (vec4*)& result);

	return result;
}

static bool Equals(quaternion left, quaternion right)
{
	return  left.x == right.x && 
			left.y == right.y && 
			left.z == right.z && 
			left.w == right.w;
}

static quaternion LookAt(vector3 origin, vector3 target, vector3 upAxis)
{
	quaternion result;
	glm_quat_forp((float*)&origin, (float*)&target, (float*)&upAxis, (float*) & result);

	return result;
}