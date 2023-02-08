#include "math/vectors.h"
#include <stdio.h>
#include "singine/file.h"
#include "cunit.h"
#include "string.h"
#include "cglm/cam.h"
#include "cglm/mat3.h"
#include "cglm/vec3.h"
#include "cglm/cam.h"
#include "cglm/affine.h"
#include <math.h>

static bool TryParseVector3(const char* buffer, const size_t length, vector3* out_vector3);
static bool TrySerializeVec3(char* buffer, const size_t length, const vector3 vector);
static bool TrySerializeVec3Stream(File stream, const vector3 vector);
static vector3 Cross(const vector3 left, const vector3 right);
static vector3 Multiply(const vector3 left, const vector3 right);
static vector3 Scale(const vector3 vector, const float value);
static vector3 Add(const vector3 left, const vector3 right);
static vector3 Subtract(const vector3 left, const vector3 right);
static bool Equals(const vector3 left, const vector3 right);
static bool Close(const vector3 left, const vector3 right, float epsilon);
static float Distance(const vector3 left, const vector3 right);

const struct _vector3Methods Vector3s = {
	.TryDeserialize = &TryParseVector3,
	.TrySerialize = &TrySerializeVec3,
	.TrySerializeStream = &TrySerializeVec3Stream,
	.Cross = &Cross,
	.Multiply = Multiply,
	.Scale  = Scale,
	.Add = Add,
	.Subtract = Subtract,
	.Equals = Equals,
	.Close = Close,
	.Distance = Distance
};

static bool TryParseVector2(const char* buffer, const size_t length, vector2* out_vector2);
static bool TrySerializeVec2(char* buffer, const size_t length, const vector2 vector);
static bool EqualsVec2(const vector2 left, const vector2 right);
static bool CloseVec2(const vector2 left, const vector2 right, float epsilon);

const struct _vector2Methods Vector2s = {
	.TryDeserialize = &TryParseVector2,
	.TrySerialize = &TrySerializeVec2,
	.Equals = EqualsVec2,
	.Close = CloseVec2
};

static bool TryDeserializeVec4(const char* buffer, const size_t length, vector4* out_vector2);
static bool TrySerializeVec4(char* buffer, const size_t length, const vector4 vector);
static bool TrySerializeVec4Stream(File stream, const vector4 vector);

const struct _vector4Methods Vector4s = {
	.TryDeserialize = &TryDeserializeVec4,
	.TrySerialize = &TrySerializeVec4,
	.TrySerializeStream = &TrySerializeVec4Stream
};

static matrix4 LookAt(vector3 position, vector3 target, vector3 upDirection);
static matrix4 MultiplyMat4(matrix4, matrix4);
static matrix4 ScaleMat4(matrix4, vector3);
static vector3 MultiplyVector3(matrix4, vector3, float w);
static matrix4 Translate(matrix4, vector3 position);

const struct _mat4Methods Matrix4s = {
	.LookAt = LookAt,
	.Multiply = MultiplyMat4,
	.MultiplyVector3 = MultiplyVector3,
	.Scale = ScaleMat4,
	.Translate = Translate
};

static float Determinant(matrix3 matrix);

const struct _mat3Methods Matrix3s = {
	.Determinant = Determinant
};

#define Vector2SerializationFormat "%f %f"
#define Vector3SerializationFormat "%f %f %f"
#define Vector4SerializationFormat "%f %f %f %f"

static bool EqualsVec2(const vector2 left, const vector2 right)
{
	return left.x == right.x && left.y == right.y;
}

static bool CloseVec2(const vector2 left, const vector2 right, float epsilon)
{
	return (fabs(left.x - right.x) < epsilon) &&
		(fabs(left.y - right.y) < epsilon);
}

static bool TryParseVector3(const char* buffer, size_t length, vector3* out_vector3)
{
	if (length is 0)
	{
		return false;
	}

	int count = sscanf_s(buffer, "%f %f %f", 
		&out_vector3->x,
		&out_vector3->y,
		&out_vector3->z);

	return count == 3;
}

static bool TryParseVector2(const char* buffer, size_t length, vector2* out_vector2)
{
	if (length is 0)
	{
		return false;
	}

	int count = sscanf_s(buffer, "%f %f",
		&out_vector2->x,
		&out_vector2->y);

	return count == 2;
}

static bool Equals(const vector3 left, const vector3 right) 
{
	return left.x == right.x && left.y == right.y && left.z == right.z;
}

static bool Close(const vector3 left, const vector3 right, float epsilon)
{
	return (fabs(left.x - right.x) < epsilon) &&
		(fabs(left.y - right.y) < epsilon) &&
		(fabs(left.z - right.z) < epsilon);
}

static vector3 Cross(const vector3 left, const vector3 right)
{
	return (vector3) 
	{
		left.y * right.z - left.z * right.y,
		left.z * right.x - left.x * right.z,
		left.x * right.y - left.y * right.x,
	};
}

static float Distance(const vector3 left, const vector3 right)
{
	return glm_vec3_distance((float*)&left, (float*)&right);
}

static vector3 Multiply(const vector3 left, const vector3 right)
{
	return (vector3) { left.x * right.x, left.y * right.y, left.z * right.z };
}

static vector3 Add(const vector3 left, const vector3 right)
{
	return (vector3) { left.x + right.x, left.y + right.y, left.z + right.z };
}

static vector3 Subtract(const vector3 left, const vector3 right)
{
	return (vector3) { left.x - right.x, left.y - right.y, left.z - right.z };
}

static vector3 Scale(const vector3 vector, float value)
{
	return (vector3) { vector.x * value, vector.y* value, vector.z * value };
}

static bool TrySerializeVec3Stream(File stream, const vector3 vector)
{
	if (stream is null)
	{
		return false;
	}

	int result = fprintf_s(stream, Vector3SerializationFormat,
		vector.x,
		vector.y,
		vector.z);

	// 0 is runtime error, negative is encoding error
	return result > 0;
}

static bool TrySerializeVec3(char* buffer, const size_t length, const vector3 vector)
{
	if (length is 0)
	{
		return false;
	}

	int result = sprintf_s(buffer, length, Vector3SerializationFormat, vector.x, vector.y, vector.z);

	// 0 is runtime error, negative is encoding error
	return result > 0;
}

static bool TrySerializeVec2(char* buffer, const size_t length, const vector2 vector)
{
	if (length is 0)
	{
		return false;
	}

	int result = sprintf_s(buffer, length, Vector2SerializationFormat, vector.x, vector.y);

	// 0 is runtime error, negative is encoding error
	return result > 0;
}

static bool TryDeserializeVec4(const char* buffer, const size_t length, vector4* out_vector4)
{
	if (length is 0)
	{
		return false;
	}

	int count = sscanf_s(buffer, Vector4SerializationFormat, &out_vector4->x,
		&out_vector4->y,
		&out_vector4->z,
		&out_vector4->w);

	return count == 4;
}

static bool TrySerializeVec4Stream(File stream, const vector4 vector)
{
	if (stream is null)
	{
		return false;
	}

	int result = fprintf_s(stream, Vector4SerializationFormat,
		vector.x,
		vector.y,
		vector.z,
		vector.w);

	// 0 is runtime error, negative is encoding error
	return result > 0;
}

static bool TrySerializeVec4(char* buffer, const size_t length, const vector4 vector)
{
	if (length is 0)
	{
		return false;
	}

	int result = sprintf_s(buffer, length, Vector4SerializationFormat,
		vector.x,
		vector.y,
		vector.z,
		vector.w);

	// 0 is runtime error, negative is encoding error
	return result > 0;
}

static bool Test_TryGetVector3(File stream)
{
	char* buffer = "-1.0 2.4 4.90";

	vector3 expected = { -1.0f, 2.4f, 4.90f };

	vector3 actual;

	Assert(Vector3s.TryDeserialize(buffer, strlen(buffer), &actual));

	Assert(Vector3s.Equals(expected, actual));

	return true;
}

static matrix4 MultiplyMat4(matrix4 left, matrix4 right)
{
	matrix4 result;
	glm_mat4_mul((float*)&left, (float*)&right, (float*)&result);

	return result;
}

static matrix4 ScaleMat4(matrix4 matrix, vector3 scale)
{
	matrix4 result;
	glm_scale_to((float*)&matrix, (float*)&scale, (float*) & result);

	return result;
}

static vector3 MultiplyVector3(matrix4 matrix, vector3 vector, float w)
{
	vector3 result;
	glm_mat4_mulv3( (float*)&matrix, (float*)&vector, w, (float*)&result);

	return result;
}

static bool Test_TryGetVector2(File stream)
{
	char* buffer = "-1.0 2.4 4.90";

	vector2 expected = { -1.0f, 2.4f };

	vector2 actual;

	Assert(Vector2s.TryDeserialize(buffer, strlen(buffer), &actual));

	Assert(Vector2s.Equals(expected, actual));

	return true;
}

bool RunVectorUnitTests()
{
	TestSuite suite = CreateSuite(__FILE__);

	suite->Append(suite, "TryGetVector3", Test_TryGetVector3);
	suite->Append(suite, "TryGetVector2", Test_TryGetVector2);

	bool pass = suite->Run(suite);

	suite->Dispose(suite);

	return pass;
}

static float Determinant(matrix3 matrix)
{
	return glm_mat3_det((vec3*) & matrix);
}

static matrix4 LookAt(vector3 position, vector3 target, vector3 upDirection)
{
	matrix4 result;
	glm_look((float*) & position, (float*)&target, (float*)&upDirection, (vec4*)&result);

	return result;
}

static matrix4 Translate(matrix4 matrix, vector3 position)
{
	matrix4 result;
	glm_translate_to((float*) & matrix, (float*)&position, (float*) & result);

	return result;
}